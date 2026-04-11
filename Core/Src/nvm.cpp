#include "nvm.h"
#include "nvm_telemetry_format.h"
#include "fatfs.h"
#include "hvc.h"
#include "faults.h"
#include <cstdio>
#include <cstring>

static FIL telemfile;
static FATFS fs;
static FRESULT res;
static FIL fsrc;
static FIL fdst;
static char telemfilename[64];
static GpsData telemetryStartGpsData = {};
static uint8_t telemetryWriteBuffer[NVM_TELEMETRY_WRITE_BUFFER_SIZE];
static UINT telemetryWriteBufferLength = 0U;
static uint32_t telemetryLastSyncTickMs = 0U;
static uint32_t telemetryLastSampleTickMs = 0U;
static bool telemetrySampleClockInitialized = false;
static bool telemetryDirty = false;

static bool telemetryBegan = false;
static bool telemetryFileOpen = false;
static NvmTelemetryStatus telemetryStatus = NVM_TELEMETRY_WAITING_FOR_GPS;

static_assert(sizeof(TelemetryRecordV1) < NVM_TELEMETRY_WRITE_BUFFER_SIZE, "Telemetry write buffer is too small");
static_assert(NVM_TELEMETRY_WRITE_THRESHOLD_BYTES <= NVM_TELEMETRY_WRITE_BUFFER_SIZE, "Telemetry threshold exceeds buffer size");

struct TelemetryStats {
  float max;
  float min;
  float mean;
};

static unsigned int nvm_getAppsFault(const VcuOutput *vcuCoreOutput) {
  return vcuCoreOutput->flags & 0x7U;
}

static unsigned int nvm_getBseFault(const VcuOutput *vcuCoreOutput) {
  return (vcuCoreOutput->flags >> 3) & 0x7U;
}

static unsigned int nvm_getStomppFault(const VcuOutput *vcuCoreOutput) {
  return (vcuCoreOutput->flags >> 6) & 0x1U;
}

static bool nvm_flushTelemetryBuffer(bool syncFile) {
  if (!telemetryFileOpen) {
    return false;
  }

  if (telemetryWriteBufferLength > 0U) {
    UINT bytesWritten = 0U;
    res = f_write(&telemfile, telemetryWriteBuffer, telemetryWriteBufferLength, &bytesWritten);
    if (res != FR_OK || bytesWritten != telemetryWriteBufferLength) {
      return false;
    }

    telemetryWriteBufferLength = 0U;
    telemetryDirty = true;
  }

  if (syncFile) {
    if (telemetryDirty) {
      res = f_sync(&telemfile);
      if (res != FR_OK) {
        return false;
      }
      telemetryDirty = false;
    }

    telemetryLastSyncTickMs = HAL_GetTick();
  }

  return true;
}

static void nvm_failTelemetry(uint32_t fault) {
  telemetryBegan = false;
  telemetryStatus = NVM_TELEMETRY_FAILED;
  telemetryWriteBufferLength = 0U;
  telemetryDirty = false;
  telemetrySampleClockInitialized = false;

  if (telemetryFileOpen) {
    f_close(&telemfile);
    telemetryFileOpen = false;
  }

  FAULT_SET(&faultVector, fault);
}

static TelemetryStats nvm_computeStats(const float *data, size_t numData) {
  float sum = 0.0f;
  size_t validCount = 0U;
  float maxValue = 0.0f;
  float minValue = 999.0f;
  bool foundMinValue = false;

  for (size_t i = 0; i < numData; ++i) {
    const float value = data[i];

    if (value != 0.0f) {
      sum += value;
      ++validCount;
      if (value > maxValue) {
        maxValue = value;
      }
    }

    if (value >= 0.1f) {
      if (!foundMinValue || value < minValue) {
        minValue = value;
        foundMinValue = true;
      }
    }
  }

  TelemetryStats stats = {};
  stats.max = maxValue;
  stats.min = foundMinValue ? minValue : 0.0f;
  stats.mean = validCount > 0U ? (sum / static_cast<float>(validCount)) : 0.0f;
  return stats;
}

static bool nvm_shouldCaptureTelemetry(uint32_t nowMs) {
  if (NVM_TELEMETRY_SAMPLE_PERIOD_MS == 0U) {
    return true;
  }

  if (!telemetrySampleClockInitialized) {
    telemetrySampleClockInitialized = true;
    telemetryLastSampleTickMs = nowMs;
    return true;
  }

  if ((nowMs - telemetryLastSampleTickMs) < NVM_TELEMETRY_SAMPLE_PERIOD_MS) {
    return false;
  }

  telemetryLastSampleTickMs = nowMs;
  return true;
}

static bool nvm_appendTelemetryRecord(const TelemetryRecordV1 &record) {
  const UINT recordSize = static_cast<UINT>(sizeof(record));

  if ((telemetryWriteBufferLength + recordSize) > NVM_TELEMETRY_WRITE_BUFFER_SIZE) {
    if (!nvm_flushTelemetryBuffer(false)) {
      return false;
    }
  }

  std::memcpy(telemetryWriteBuffer + telemetryWriteBufferLength, &record, sizeof(record));
  telemetryWriteBufferLength += recordSize;
  return true;
}

static bool nvm_beginTelemetry() {
  TelemetryFileHeaderV1 header = {};
  std::memcpy(header.magic, NVM_TELEMETRY_MAGIC, sizeof(header.magic));
  header.version = NVM_TELEMETRY_VERSION;
  header.headerSize = static_cast<uint16_t>(sizeof(header));
  header.recordSize = static_cast<uint16_t>(sizeof(TelemetryRecordV1));
  header.flags = NVM_TELEMETRY_FLAGS_LITTLE_ENDIAN | NVM_TELEMETRY_FLAGS_IEEE754;
  header.samplePeriodMs = NVM_TELEMETRY_SAMPLE_PERIOD_MS;
  header.syncIntervalMs = NVM_TELEMETRY_SYNC_INTERVAL_MS;
  header.startYear = telemetryStartGpsData.year;
  header.startMonth = telemetryStartGpsData.month;
  header.startDay = telemetryStartGpsData.day;
  header.startHour = telemetryStartGpsData.hour;
  header.startMinute = telemetryStartGpsData.minute;
  header.startSecond = telemetryStartGpsData.seconds;
  header.startMillis = telemetryStartGpsData.millis;

  res = f_open(&telemfile, telemfilename, FA_CREATE_ALWAYS | FA_WRITE);
  if (res != FR_OK) {
    return false;
  }

  telemetryFileOpen = true;

  UINT bytesWritten = 0U;
  res = f_write(&telemfile, &header, sizeof(header), &bytesWritten);
  if (res != FR_OK || bytesWritten != sizeof(header)) {
    f_close(&telemfile);
    telemetryFileOpen = false;
    return false;
  }

  res = f_sync(&telemfile);
  if (res != FR_OK) {
    f_close(&telemfile);
    telemetryFileOpen = false;
    return false;
  }

  telemetryWriteBufferLength = 0U;
  telemetryDirty = false;
  telemetrySampleClockInitialized = false;
  telemetryLastSyncTickMs = HAL_GetTick();
  return true;
}

static void nvm_writeTelemetry(VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus, PduStatus *pduStatus, InverterStatus *inverterStatus,
                               AnalogVoltages *analogVoltages, WheelMagnetValues *wheelMagnetValues, ImuData *imuData,
                               GpsData *gpsData) {
  const uint32_t nowMs = HAL_GetTick();

  if (!nvm_shouldCaptureTelemetry(nowMs)) {
    if (NVM_TELEMETRY_SYNC_INTERVAL_MS > 0U &&
        (nowMs - telemetryLastSyncTickMs) >= NVM_TELEMETRY_SYNC_INTERVAL_MS &&
        !nvm_flushTelemetryBuffer(true)) {
      nvm_failTelemetry(FAULT_VCU_NVM_NO_WRITE);
    }
    return;
  }

  const unsigned int appsFault = nvm_getAppsFault(vcuCoreOutput);
  const unsigned int bseFault = nvm_getBseFault(vcuCoreOutput);
  const unsigned int stomppFault = nvm_getStomppFault(vcuCoreOutput);
  const unsigned int steeringFault = 0U;

  const TelemetryStats segment1Stats = nvm_computeStats(hvcStatus->cellTemps, 10U);
  const TelemetryStats segment2Stats = nvm_computeStats(hvcStatus->cellTemps + 10, 10U);
  const TelemetryStats segment3Stats = nvm_computeStats(hvcStatus->cellTemps + 20, 10U);
  const TelemetryStats segment4Stats = nvm_computeStats(hvcStatus->cellTemps + 30, 10U);
  const TelemetryStats segmentUniqueStats = nvm_computeStats(hvcStatus->cellTemps + 40, 10U);
  const TelemetryStats cellVoltageStats = nvm_computeStats(hvcStatus->cellVoltages, 140U);
  const TelemetryStats cellTempStats = nvm_computeStats(hvcStatus->cellTemps, 50U);

  TelemetryRecordV1 record = {};
  record.uptimeMs = nowMs;

  record.segment1Max = segment1Stats.max;
  record.segment1Min = segment1Stats.min;
  record.segment1Mean = segment1Stats.mean;
  record.segment2Max = segment2Stats.max;
  record.segment2Min = segment2Stats.min;
  record.segment2Mean = segment2Stats.mean;
  record.segment3Max = segment3Stats.max;
  record.segment3Min = segment3Stats.min;
  record.segment3Mean = segment3Stats.mean;
  record.segment4Max = segment4Stats.max;
  record.segment4Min = segment4Stats.min;
  record.segment4Mean = segment4Stats.mean;
  record.segmentUniqueMax = segmentUniqueStats.max;
  record.segmentUniqueMin = segmentUniqueStats.min;
  record.segmentUniqueMean = segmentUniqueStats.mean;

  record.enableInverter = vcuCoreOutput->enableInverter ? 1U : 0U;
  record.inverterTorqueRequest = vcuCoreOutput->inverterTorqueRequest;
  record.telemetryOcvEstimate = vcuCoreOutput->telemetryOcvEstimate;
  record.telemetryPowerLimit = vcuCoreOutput->telemetryPowerLimit;
  record.telemetryPowerLimitFeedbackP = vcuCoreOutput->telemetryPowerLimitFeedbackP;
  record.telemetryPowerLimitFeedbackI = vcuCoreOutput->telemetryPowerLimitFeedbackI;
  record.telemetryPowerLimitFeedbackD = vcuCoreOutput->telemetryPowerLimitFeedbackD;
  record.telemetryPowerLimitFeedbackTorque = vcuCoreOutput->telemetryPowerLimitFeedbackTorque;
  record.prndlState = vcuCoreOutput->prndlState ? 1U : 0U;
  record.readyToDriveBuzzer = vcuCoreOutput->r2dBuzzer ? 1U : 0U;
  record.brakeLight = vcuCoreOutput->brakeLight;
  record.enableDragReduction = vcuCoreOutput->enableDragReduction ? 1U : 0U;
  record.pumpOutput = vcuCoreOutput->pumpOutput;
  record.radiatorOutput = vcuCoreOutput->radiatorOutput;
  record.batteryFansOutput = vcuCoreOutput->batteryFansOutput;
  record.vehicleDisplacementX = vcuCoreOutput->vehicleDisplacement.x;
  record.vehicleDisplacementY = vcuCoreOutput->vehicleDisplacement.y;
  record.vehicleDisplacementZ = vcuCoreOutput->vehicleDisplacement.z;
  record.vehicleVelocityX = vcuCoreOutput->vehicleVelocity.x;
  record.vehicleVelocityY = vcuCoreOutput->vehicleVelocity.y;
  record.vehicleVelocityZ = vcuCoreOutput->vehicleVelocity.z;
  record.vehicleAccelerationX = vcuCoreOutput->vehicleAcceleration.x;
  record.vehicleAccelerationY = vcuCoreOutput->vehicleAcceleration.y;
  record.vehicleAccelerationZ = vcuCoreOutput->vehicleAcceleration.z;
  record.hvBatterySoc = vcuCoreOutput->hvBatterySoc;
  record.lvBatterySoc = vcuCoreOutput->lvBatterySoc;
  record.dashSpeed = vcuCoreOutput->dashSpeed;
  record.telemetryApps = vcuCoreOutput->telemetryApps;
  record.telemetryBse = vcuCoreOutput->telemetryBse;
  record.telemetrySteeringWheel = vcuCoreOutput->telemetrySteeringWheel;
  record.appsFault = static_cast<uint8_t>(appsFault);
  record.bseFault = static_cast<uint8_t>(bseFault);
  record.stomppFault = static_cast<uint8_t>(stomppFault);
  record.steeringFault = static_cast<uint8_t>(steeringFault);

  record.packVoltage = hvcStatus->packVoltage;
  record.packCurrent = hvcStatus->packCurrent;
  record.stateOfCharge = hvcStatus->stateOfCharge;
  record.packVoltageMean = hvcStatus->packVoltageMean;
  record.packVoltageMin = hvcStatus->packVoltageMin;
  record.packVoltageMax = hvcStatus->packVoltageMax;
  record.packVoltageRange = hvcStatus->packVoltageRange;
  record.packTempMean = hvcStatus->packTempMean;
  record.packTempMin = hvcStatus->packTempMin;
  record.packTempMax = hvcStatus->packTempMax;
  record.packTempRange = hvcStatus->packTempRange;
  record.imd = hvcStatus->imd ? 1U : 0U;
  record.ams = hvcStatus->ams ? 1U : 0U;
  record.contactorStatus = hvcStatus->contactorStatus;
  record.cellVoltageMean = cellVoltageStats.mean;
  record.cellVoltageMax = cellVoltageStats.max;
  record.cellVoltageMin = cellVoltageStats.min;
  record.cellTempsMean = cellTempStats.mean;
  record.cellTempsMax = cellTempStats.max;
  record.cellTempsMin = cellTempStats.min;

  record.volumetricFlowRate = pduStatus->volumetricFlowRate;
  record.waterTempInverter = pduStatus->waterTempInverter;
  record.waterTempMotor = pduStatus->waterTempMotor;
  record.waterTempRadiator = pduStatus->waterTempRadiator;
  record.radiatorFanRpmPercentage = pduStatus->radiatorFanRpm;
  record.lvVoltage = pduStatus->lvVoltage;
  record.lvStateOfCharge = pduStatus->lvSoC;
  record.lvCurrent = pduStatus->lvCurrent;

  record.voltageInputIntoDc = inverterStatus->voltage;
  record.currentInputIntoDc = inverterStatus->current;
  record.rpm = inverterStatus->rpm;
  record.feedbackSpeed = inverterStatus->rpm;
  record.averageModuleTemp = inverterStatus->inverterTemp;
  record.inverterCoolantTemp = inverterStatus->inverterCoolantTemp;
  record.inverterHotSpotTemp = inverterStatus->inverterHotSpotTemp;
  record.motorTemp = inverterStatus->motorTemp;
  record.motorAngle = inverterStatus->motorAngle;
  record.deltaResolver = inverterStatus->deltaResolverFiltered;
  record.phaseACurrent = inverterStatus->phaseACurrent;
  record.phaseBCurrent = inverterStatus->phaseBCurrent;
  record.phaseCCurrent = inverterStatus->phaseCCurrent;
  record.idFeedback = inverterStatus->idFeedback;
  record.iqFeedback = inverterStatus->iqFeedback;
  record.bcVoltage = inverterStatus->BCVoltage;
  record.abVoltage = inverterStatus->ABVoltage;
  record.outputVoltage = inverterStatus->outputVoltage;
  record.idCommand = inverterStatus->idCommand;
  record.iqCommand = inverterStatus->iqCommand;
  record.inverterFrequency = inverterStatus->inverterFrequency;
  record.actualTorque = inverterStatus->torqueActual;
  record.torqueCommand = inverterStatus->torqueCommand;
  record.faultVector = inverterStatus->faultVector;
  record.stateVector = inverterStatus->stateVector;
  record.bmsLimitingRegenTorque = inverterStatus->bmsLimitingRegenTorque ? 1U : 0U;
  record.motorTempDerateLimiting = inverterStatus->limitMotorTempDerate ? 1U : 0U;
  record.motorHotSpotLimiting = inverterStatus->limitHotSpotMotor ? 1U : 0U;
  record.bmsActive = inverterStatus->bmsActive ? 1U : 0U;
  record.bmsLimitingMotorTorque = inverterStatus->bmsLimitingMotorTorque ? 1U : 0U;
  record.maxSpeedLimiting = inverterStatus->limitMaxSpeed ? 1U : 0U;
  record.inverterHotSpotLimiting = inverterStatus->limitHotSpotInverter ? 1U : 0U;
  record.lowSpeedLimiting = inverterStatus->lowSpeedLimiting ? 1U : 0U;
  record.coolantDeratingLimiting = inverterStatus->limitCoolantDerating ? 1U : 0U;
  record.stallBurstLimiting = inverterStatus->limitStallBurstModel ? 1U : 0U;

  record.apps1Voltage = analogVoltages->apps1;
  record.apps2Voltage = analogVoltages->apps2;
  record.bse1Voltage = analogVoltages->bse1;
  record.bse2Voltage = analogVoltages->bse2;
  record.steerVoltage = analogVoltages->steer;
  record.suspension1Voltage = analogVoltages->sus1;
  record.suspension2Voltage = analogVoltages->sus2;

  record.frontLeftWheelSpeed = vcuCoreOutput->telemetryWheelSpeedFl;
  record.frontRightWheelSpeed = vcuCoreOutput->telemetryWheelSpeedFr;
  record.backLeftWheelSpeed = vcuCoreOutput->telemetryWheelSpeedBl;
  record.backRightWheelSpeed = vcuCoreOutput->telemetryWheelSpeedBr;
  record.frontLeftWheelMagneticField = wheelMagnetValues->fl;

  record.vcuAccelerationX = imuData->accelVcu.x;
  record.vcuAccelerationY = imuData->accelVcu.y;
  record.vcuAccelerationZ = imuData->accelVcu.z;
  record.hvcAccelerationX = imuData->accelHvc.x;
  record.hvcAccelerationY = imuData->accelHvc.y;
  record.hvcAccelerationZ = imuData->accelHvc.z;
  record.pduAccelerationX = imuData->accelPdu.x;
  record.pduAccelerationY = imuData->accelPdu.y;
  record.pduAccelerationZ = imuData->accelPdu.z;
  record.frontLeftAccelerationX = imuData->accelFl.x;
  record.frontLeftAccelerationY = imuData->accelFl.y;
  record.frontLeftAccelerationZ = imuData->accelFl.z;
  record.frontRightAccelerationX = imuData->accelFr.x;
  record.frontRightAccelerationY = imuData->accelFr.y;
  record.frontRightAccelerationZ = imuData->accelFr.z;
  record.backLeftAccelerationX = imuData->accelBl.x;
  record.backLeftAccelerationY = imuData->accelBl.y;
  record.backLeftAccelerationZ = imuData->accelBl.z;
  record.backRightAccelerationX = imuData->accelBr.x;
  record.backRightAccelerationY = imuData->accelBr.y;
  record.backRightAccelerationZ = imuData->accelBr.z;
  record.vcuGyroX = imuData->gyroVcu.x;
  record.vcuGyroY = imuData->gyroVcu.y;
  record.vcuGyroZ = imuData->gyroVcu.z;
  record.hvcGyroX = imuData->gyroHvc.x;
  record.hvcGyroY = imuData->gyroHvc.y;
  record.hvcGyroZ = imuData->gyroHvc.z;
  record.pduGyroX = imuData->gyroPdu.x;
  record.pduGyroY = imuData->gyroPdu.y;
  record.pduGyroZ = imuData->gyroPdu.z;

  record.gpsLatitude = gpsData->latitude;
  record.gpsLongitude = gpsData->longitude;
  record.gpsSpeed = gpsData->speed;
  record.gpsHeading = gpsData->heading;
  record.gpsHour = gpsData->hour;
  record.gpsMinute = gpsData->minute;
  record.gpsSeconds = gpsData->seconds;
  record.gpsYear = gpsData->year;
  record.gpsMonth = gpsData->month;
  record.gpsDay = gpsData->day;
  record.gpsMilliseconds = gpsData->millis;

  if (!nvm_appendTelemetryRecord(record)) {
    nvm_failTelemetry(FAULT_VCU_NVM_NO_WRITE);
    return;
  }

  const bool writeThresholdReached = telemetryWriteBufferLength >= NVM_TELEMETRY_WRITE_THRESHOLD_BYTES;
  const bool syncDue = NVM_TELEMETRY_SYNC_INTERVAL_MS > 0U &&
                       (nowMs - telemetryLastSyncTickMs) >= NVM_TELEMETRY_SYNC_INTERVAL_MS;
  if ((writeThresholdReached || syncDue) && !nvm_flushTelemetryBuffer(syncDue)) {
    nvm_failTelemetry(FAULT_VCU_NVM_NO_WRITE);
  }
}

static void nvm_loadParameters(VcuParameters *vcuParameters) {
  UINT br = 0U;
  BYTE buffer[sizeof(VcuParameters)] = {};

  res = f_open(&fsrc, "VcuParams.dat", FA_OPEN_EXISTING | FA_READ);
  if (res != FR_OK) {
    return;
  }

  res = f_read(&fsrc, buffer, sizeof(buffer), &br);
  if (res == FR_OK) {
    *vcuParameters = *((VcuParameters *) buffer);
  }

  f_close(&fsrc);
}

static void nvm_saveParameters(VcuParameters *vcuParameters) {
  UINT bw = 0U;

  res = f_open(&fdst, "VcuParams.dat", FA_CREATE_ALWAYS | FA_WRITE);
  if (res != FR_OK) {
    FAULT_SET(&faultVector, FAULT_VCU_NVM_NO_CREATE);
    f_close(&fdst);
    return;
  }

  res = f_write(&fdst, vcuParameters, sizeof(VcuParameters), &bw);
  if (res != FR_OK || bw < sizeof(VcuParameters)) {
    FAULT_SET(&faultVector, FAULT_VCU_NVM_NO_WRITE);
  }

  f_close(&fdst);
}

void nvm_init(VcuParameters *vcuParameters) {
  res = f_mount(&fs, "", 0);
  if (res != FR_OK) {
    nvm_failTelemetry(FAULT_VCU_NVM_NO_MOUNT);
    return;
  }

  (void) vcuParameters;
//  nvm_loadParameters(vcuParameters);
}

void nvm_periodic(VcuParameters *vcuParameters, VcuOutput *vcuCoreOutput,
                  HvcStatus *hvcStatus, PduStatus *pduStatus, InverterStatus *inverterStatus,
                  AnalogVoltages *analogVoltages, WheelMagnetValues *wheelMagnetValues,
                  ImuData *imuData, GpsData *gpsData) {
  (void) vcuParameters;

  if (telemetryStatus == NVM_TELEMETRY_FAILED) {
    return;
  }

//  static float time = 0;
//  if (clock_getTime() >= time + 1) {
//    nvm_saveParameters(vcuParameters);
//    time = clock_getTime();
//  }

  if (!telemetryBegan && gpsData->year != 0U) {
    telemetryStartGpsData = *gpsData;
    std::snprintf(
        telemfilename,
        sizeof(telemfilename),
        "Log__20%02u_%02u_%02u__%02u_%02u_%02u.vbl",
        static_cast<unsigned int>(gpsData->year),
        static_cast<unsigned int>(gpsData->month),
        static_cast<unsigned int>(gpsData->day),
        static_cast<unsigned int>(gpsData->hour),
        static_cast<unsigned int>(gpsData->minute),
        static_cast<unsigned int>(gpsData->seconds)
    );

    if (nvm_beginTelemetry()) {
      telemetryBegan = true;
      telemetryStatus = NVM_TELEMETRY_LOGGING;
      FAULT_CLEAR(&faultVector, FAULT_VCU_NVM_NO_CREATE);
      FAULT_CLEAR(&faultVector, FAULT_VCU_NVM_NO_WRITE);
    } else {
      nvm_failTelemetry(FAULT_VCU_NVM_NO_CREATE);
      return;
    }
  }

  if (telemetryBegan) {
    nvm_writeTelemetry(vcuCoreOutput, hvcStatus, pduStatus, inverterStatus, analogVoltages, wheelMagnetValues, imuData,
                       gpsData);
  }
}

NvmTelemetryStatus nvm_getTelemetryStatus(void) {
  return telemetryStatus;
}
