#ifndef VCU_FIRMWARE_2024_NVM_TELEMETRY_FORMAT_H
#define VCU_FIRMWARE_2024_NVM_TELEMETRY_FORMAT_H

#include <cstddef>
#include <cstdint>

static constexpr char NVM_TELEMETRY_MAGIC[4] = {'V', 'B', 'L', 'G'};
static constexpr uint16_t NVM_TELEMETRY_VERSION = 1U;
static constexpr uint16_t NVM_TELEMETRY_FLAGS_LITTLE_ENDIAN = 1U << 0;
static constexpr uint16_t NVM_TELEMETRY_FLAGS_IEEE754 = 1U << 1;

#pragma pack(push, 1)

struct TelemetryFileHeaderV1 {
  char magic[4];
  uint16_t version;
  uint16_t headerSize;
  uint16_t recordSize;
  uint16_t flags;
  uint16_t samplePeriodMs;
  uint16_t syncIntervalMs;
  uint8_t startYear;
  uint8_t startMonth;
  uint8_t startDay;
  uint8_t startHour;
  uint8_t startMinute;
  uint8_t startSecond;
  uint16_t startMillis;
  uint8_t reserved[8];
};

struct TelemetryRecordV1 {
  uint32_t uptimeMs;

  float segment1Max;
  float segment1Min;
  float segment1Mean;
  float segment2Max;
  float segment2Min;
  float segment2Mean;
  float segment3Max;
  float segment3Min;
  float segment3Mean;
  float segment4Max;
  float segment4Min;
  float segment4Mean;
  float segmentUniqueMax;
  float segmentUniqueMin;
  float segmentUniqueMean;

  uint8_t enableInverter;
  float inverterTorqueRequest;
  float telemetryOcvEstimate;
  float telemetryPowerLimit;
  float telemetryPowerLimitFeedbackP;
  float telemetryPowerLimitFeedbackI;
  float telemetryPowerLimitFeedbackD;
  float telemetryPowerLimitFeedbackTorque;
  uint8_t prndlState;
  uint8_t readyToDriveBuzzer;
  float brakeLight;
  uint8_t enableDragReduction;
  float pumpOutput;
  float radiatorOutput;
  float batteryFansOutput;
  float vehicleDisplacementX;
  float vehicleDisplacementY;
  float vehicleDisplacementZ;
  float vehicleVelocityX;
  float vehicleVelocityY;
  float vehicleVelocityZ;
  float vehicleAccelerationX;
  float vehicleAccelerationY;
  float vehicleAccelerationZ;
  float hvBatterySoc;
  float lvBatterySoc;
  float dashSpeed;
  float telemetryApps;
  float telemetryBse;
  float telemetrySteeringWheel;
  uint8_t appsFault;
  uint8_t bseFault;
  uint8_t stomppFault;
  uint8_t steeringFault;

  float packVoltage;
  float packCurrent;
  float stateOfCharge;
  float packVoltageMean;
  float packVoltageMin;
  float packVoltageMax;
  float packVoltageRange;
  float packTempMean;
  float packTempMin;
  float packTempMax;
  float packTempRange;
  uint8_t imd;
  uint8_t ams;
  uint8_t contactorStatus;
  float cellVoltageMean;
  float cellVoltageMax;
  float cellVoltageMin;
  float cellTempsMean;
  float cellTempsMax;
  float cellTempsMin;

  float volumetricFlowRate;
  float waterTempInverter;
  float waterTempMotor;
  float waterTempRadiator;
  float radiatorFanRpmPercentage;
  float lvVoltage;
  float lvStateOfCharge;
  float lvCurrent;

  float voltageInputIntoDc;
  float currentInputIntoDc;
  float rpm;
  float feedbackSpeed;
  float averageModuleTemp;
  float inverterCoolantTemp;
  float inverterHotSpotTemp;
  float motorTemp;
  float motorAngle;
  float deltaResolver;
  float phaseACurrent;
  float phaseBCurrent;
  float phaseCCurrent;
  float idFeedback;
  float iqFeedback;
  float bcVoltage;
  float abVoltage;
  float outputVoltage;
  float idCommand;
  float iqCommand;
  float inverterFrequency;
  float actualTorque;
  float torqueCommand;
  uint64_t faultVector;
  uint64_t stateVector;
  uint8_t bmsLimitingRegenTorque;
  uint8_t motorTempDerateLimiting;
  uint8_t motorHotSpotLimiting;
  uint8_t bmsActive;
  uint8_t bmsLimitingMotorTorque;
  uint8_t maxSpeedLimiting;
  uint8_t inverterHotSpotLimiting;
  uint8_t lowSpeedLimiting;
  uint8_t coolantDeratingLimiting;
  uint8_t stallBurstLimiting;

  float apps1Voltage;
  float apps2Voltage;
  float bse1Voltage;
  float bse2Voltage;
  float steerVoltage;
  float suspension1Voltage;
  float suspension2Voltage;

  float frontLeftWheelSpeed;
  float frontRightWheelSpeed;
  float backLeftWheelSpeed;
  float backRightWheelSpeed;
  float frontLeftWheelMagneticField;

  float vcuAccelerationX;
  float vcuAccelerationY;
  float vcuAccelerationZ;
  float hvcAccelerationX;
  float hvcAccelerationY;
  float hvcAccelerationZ;
  float pduAccelerationX;
  float pduAccelerationY;
  float pduAccelerationZ;
  float frontLeftAccelerationX;
  float frontLeftAccelerationY;
  float frontLeftAccelerationZ;
  float frontRightAccelerationX;
  float frontRightAccelerationY;
  float frontRightAccelerationZ;
  float backLeftAccelerationX;
  float backLeftAccelerationY;
  float backLeftAccelerationZ;
  float backRightAccelerationX;
  float backRightAccelerationY;
  float backRightAccelerationZ;
  float vcuGyroX;
  float vcuGyroY;
  float vcuGyroZ;
  float hvcGyroX;
  float hvcGyroY;
  float hvcGyroZ;
  float pduGyroX;
  float pduGyroY;
  float pduGyroZ;

  double gpsLatitude;
  double gpsLongitude;
  float gpsSpeed;
  float gpsHeading;
  uint8_t gpsHour;
  uint8_t gpsMinute;
  uint8_t gpsSeconds;
  uint8_t gpsYear;
  uint8_t gpsMonth;
  uint8_t gpsDay;
  uint16_t gpsMilliseconds;
};

#pragma pack(pop)

static_assert(sizeof(TelemetryFileHeaderV1) == 32U, "Telemetry header layout changed");
static_assert(sizeof(TelemetryRecordV1) == 597U, "Telemetry record layout changed");

#endif // VCU_FIRMWARE_2024_NVM_TELEMETRY_FORMAT_H
