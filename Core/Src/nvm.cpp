#include "nvm.h"
#include "fatfs.h"
#include "clock.h"
#include "faults.h"
#include <sstream>
#include <cstring>

static FIL telemfile;
static FATFS fs;
static FRESULT res;
static FIL fsrc;
static FIL fdst;
static string telemfilename;

static bool telemetryBegan = false;

static char charBuffer[4096];

static void nvm_loadParameters(VcuParameters *vcuParameters) {
  UINT br = 0;
  BYTE buffer[sizeof(VcuParameters)] = {};

  // open source file (drive 1)
  res = f_open(
      &fsrc,
      "VcuParams.dat",
      FA_OPEN_EXISTING | FA_READ
  );

  if (res) {
    return;
  }

  // copy source to destination
  res = f_read(
      &fsrc,
      buffer,
      sizeof buffer,
      &br);

  if (!res) {
    *vcuParameters = *((VcuParameters *) buffer);
  }

  // close open files
  f_close(&fsrc);

}

static void nvm_saveParameters(VcuParameters *vcuParameters) {
  UINT bw = 0;

  // create destination file (drive 0)
  res = f_open(
      &fdst,
      "VcuParams.dat",
      FA_CREATE_ALWAYS | FA_WRITE
  );

  if (res) {
    FAULT_SET(&vcu_fault_vector, FAULT_VCU_NVM_NO_CREATE);
    f_close(&fdst);
    return;
  }

  // copy source to destination
  res = f_write(
      &fdst,
      vcuParameters,
      sizeof(VcuParameters),
      &bw
  );
  if (res || bw < sizeof(VcuParameters)) {
    FAULT_SET(&vcu_fault_vector, FAULT_VCU_NVM_NO_WRITE);
  }

  // close open files
  f_close(&fdst);

}

static void nvm_beginTelemetry() {
  // create new csv file and leave open to write telemetry
  res = f_open(
      &telemfile,
      telemfilename.c_str(),
      FA_CREATE_ALWAYS | FA_WRITE
  );

  if (res) {
    FAULT_SET(&vcu_fault_vector, FAULT_VCU_NVM_NO_CREATE);
    f_close(&telemfile);
    return;
  }

  // create headers for data
  f_printf(
          &telemfile,
          "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
          "Time","Inverter Enabled", "Inverter Torque Request", "PRNDL State", "Ready To Drive Buzzer", "Brake Light", "Enable Drag Reduction", "Pump Output", "Radiator Output", "Battery Fans Output", "Vehicle Displacement X", "Vehicle Displacement Y", "Vehicle Displacement Z", "Vehicle Velocity X", "Vehicle Velocity Y", "Vehicle Velocity Z", "Vehicle Acceleration X", "Vehicle Acceleration Y", "Vehicle Acceleration Z", "HV Battery", "LV Battery", "Dash Speed", "APPS Telemetry", "BSE Telemetry", "Steering Wheel Telemetry", "APPS Fault", "BSE Fault", "STOMPP Fault", "Steering Fault","Voltage", "Current", "State of Charge", "Pack Voltage Mean", "Pack Voltage Minimum", "Pack Voltage Maximum", "Pack Voltage Range", "Pack Temp Mean", "Pack Temp Minimum", "Pack Temp Maximum", "Pack Temp Range", "IMD", "AMS", "Contactor Status", "Cell Voltages", "Cell Temps","Volumetric Flow Rate", "Water Temp Inverter", "Water Temp Motor", "Water Temp Radiator", "Radiator Fan RPM Percentage", "LV Voltage", "LV State of Charge", "LV Current", "Voltage Input into DC", "Current Input into DC", "RPM", "Inverter Temp", "Motor Temp", "Motor Angle", "Resolver Angle", "Phase A Current", "Phase B Current", "Phase C Current", "BC Voltage", "AB Voltage", "Output Voltage", "Inverter Frequency", "Actual Torque", "Torque Command", "Fault Vector", "State Vector","APPS 1 Voltage", "APPS 2 Voltage", "BSE 1 Voltage", "BSE 2 Voltage", "Steer Voltage", "Suspension 1 Voltage", "Suspension 2 Voltage", "Front Left Wheel Speed", "Front Right Wheel Speed", "Back Left Wheel Speed", "Back Right Wheel Speed","VCU Acceleration X", "VCU Acceleration Y", "VCU Acceleration Z", "HVC Acceleration X", "HVC Acceleration Y", "HVC Acceleration Z", "PDU Acceleration X", "PDU Acceleration Y", "PDU Acceleration Z", "Front Left Acceleration X", "Front Left Acceleration Y", "Front Left Acceleration Z", "Front Right Acceleration X", "Front Right Acceleration Y", "Front Right Acceleration Z", "Back Left Acceleration X", "Back Left Acceleration Y", "Back Left Acceleration Z", "Back Right Acceleration X", "Back Right Acceleration Y", "Back Right Acceleration Z", "VCU Gyro X", "VCU Gyro Y", "VCU Gyro Z", "HVC Gyro X", "HVC Gyro Y", "HVC Gyro Z", "PDU Gyro X", "PDU Gyro Y", "PDU Gyro Z","Latitude", "Longitude", "Speed", "Heading", "Hour", "Minute", "Seconds", "Year", "Month", "Day", "Milliseconds"
          );

  // close file to save
  f_close(&telemfile);

}

static void nvm_writeTelemetry(VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus, PduStatus *pduStatus, InverterStatus *inverterStatus, AnalogVoltages *analogVoltages, WheelMagnetValues *wheelMagnetValues, ImuData *imuData, GpsData *gpsData) {
  static uint32_t counter = 0;

  // open telemfile
  if(counter == 0) {
    res = f_open(
        &telemfile,
        telemfilename.c_str(),
        FA_OPEN_EXISTING | FA_WRITE | FA_OPEN_APPEND
    );
    if (res) {
      FAULT_SET(&vcu_fault_vector, FAULT_VCU_NVM_NO_WRITE);
    }
  }

  // write row of data into file

  if(counter % 2 == 0) {
    sprintf(charBuffer,
        // time format
            "%4f,%d,%4f,%d,%d,%d,%d,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%d,%d,%d,%d,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%d,%d,%u,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%10llu,%10llu,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%4f,%10hhu,%10hhu,%10hhu,%10hhu,%10hhu,%10hhu,%10hu\n",
        // time data
            clock_getTime(),
        // vcu output data
            (int) vcuCoreOutput->enableInverter, vcuCoreOutput->inverterTorqueRequest, (int) vcuCoreOutput->prndlState,
            (int) vcuCoreOutput->r2dBuzzer, (int) vcuCoreOutput->brakeLight, (int) vcuCoreOutput->enableDragReduction,
            vcuCoreOutput->pumpOutput, vcuCoreOutput->radiatorOutput, vcuCoreOutput->batteryFansOutput,
            vcuCoreOutput->vehicleDisplacement.x, vcuCoreOutput->vehicleDisplacement.y,
            vcuCoreOutput->vehicleDisplacement.z, vcuCoreOutput->vehicleVelocity.x, vcuCoreOutput->vehicleVelocity.y,
            vcuCoreOutput->vehicleVelocity.z, vcuCoreOutput->vehicleAcceleration.x,
            vcuCoreOutput->vehicleAcceleration.y, vcuCoreOutput->vehicleAcceleration.z, vcuCoreOutput->hvBatterySoc,
            vcuCoreOutput->lvBatterySoc, vcuCoreOutput->dashSpeed, vcuCoreOutput->telemetryApps,
            vcuCoreOutput->telemetryBse, vcuCoreOutput->telemetrySteeringWheel, 0, 0, 0,
            0,//(int)vcuCoreOutput->faultApps, (int)vcuCoreOutput->faultBse, (int)vcuCoreOutput->faultStompp, (int)vcuCoreOutput->faultSteering,
        // hvc status data
            hvcStatus->packVoltage, hvcStatus->packCurrent, hvcStatus->stateOfCharge, hvcStatus->packVoltageMean,
            hvcStatus->packVoltageMin, hvcStatus->packVoltageMax, hvcStatus->packVoltageRange, hvcStatus->packTempMean,
            hvcStatus->packTempMin, hvcStatus->packTempMax, hvcStatus->packTempRange, (int) hvcStatus->imd,
            (int) hvcStatus->ams, hvcStatus->contactorStatus, hvcStatus->cellVoltages, hvcStatus->cellTemps,
        // pdu status data
            pduStatus->volumetricFlowRate, pduStatus->waterTempInverter, pduStatus->waterTempMotor,
            pduStatus->waterTempRadiator, pduStatus->radiatorFanRpmPercentage, pduStatus->lvVoltage, pduStatus->lvSoC,
            pduStatus->lvCurrent,
        // inverter status data
            inverterStatus->voltage, inverterStatus->current, inverterStatus->rpm, inverterStatus->inverterTemp,
            inverterStatus->motorTemp, inverterStatus->motorAngle, inverterStatus->resolverAngle,
            inverterStatus->phaseACurrent, inverterStatus->phaseBCurrent, inverterStatus->phaseCCurrent,
            inverterStatus->BCVoltage, inverterStatus->ABVoltage, inverterStatus->outputVoltage,
            inverterStatus->inverterFrequency, inverterStatus->torqueActual, inverterStatus->torqueCommand,
            inverterStatus->faultVector, inverterStatus->stateVector,
        // analog voltages data
            analogVoltages->apps1, analogVoltages->apps2, analogVoltages->bse1, analogVoltages->bse2,
            analogVoltages->steer, analogVoltages->sus1, analogVoltages->sus2,
        // wheel magnet values data
            wheelMagnetValues->fl, wheelMagnetValues->fr, wheelMagnetValues->bl, wheelMagnetValues->br,
        // imu data
            imuData->accelVcu.x, imuData->accelVcu.y, imuData->accelVcu.z, imuData->accelHvc.x, imuData->accelHvc.y,
            imuData->accelHvc.z, imuData->accelPdu.x, imuData->accelPdu.y, imuData->accelPdu.z, imuData->accelFl.x,
            imuData->accelFl.y, imuData->accelFl.z, imuData->accelFr.x, imuData->accelFr.y, imuData->accelFr.z,
            imuData->accelBl.x, imuData->accelBl.y, imuData->accelBl.z, imuData->accelBr.x, imuData->accelBr.y,
            imuData->accelBr.z, imuData->gyroVcu.x, imuData->gyroVcu.y, imuData->gyroVcu.z, imuData->gyroHvc.x,
            imuData->gyroHvc.y, imuData->gyroHvc.z, imuData->gyroPdu.x, imuData->gyroPdu.y, imuData->gyroPdu.z,
        // gps data
            gpsData->latitude, gpsData->longitude, gpsData->speed, gpsData->heading, gpsData->hour, gpsData->minute,
            gpsData->seconds, gpsData->year, gpsData->month, gpsData->day, gpsData->millis
    );
  }

  if(counter % 2 == 1) {
    f_printf(&telemfile, charBuffer);
  }

  if(counter == FILE_SAVE_INTERVAL - 1) {
    // close file to save
    f_close(&telemfile);
  }

  counter = (counter + 1) % FILE_SAVE_INTERVAL;
}

void nvm_init(VcuParameters *vcuParameters) {
  //mount default drive
  res = f_mount(&fs, "", 0);
  if(res) {
    FAULT_SET(&vcu_fault_vector, FAULT_VCU_NVM_NO_MOUNT);
  }

  // load vcu parameters
//  nvm_loadParameters(vcuParameters);
}

void nvm_periodic(VcuParameters *vcuParameters, VcuOutput *vcuCoreOutput,
                  HvcStatus *hvcStatus, PduStatus *pduStatus, InverterStatus *inverterStatus,
                  AnalogVoltages *analogVoltages, WheelMagnetValues *wheelMagnetValues,
                  ImuData *imuData, GpsData *gpsData) {

  // save vcu parameters once a second
//  static float time = 0;
//  if (clock_getTime() >= time + 1) {
//    nvm_saveParameters(vcuParameters);
//    time = clock_getTime();
//  }

  if(!telemetryBegan && gpsData->year != 0) {
    // create telemetry csv file using time from gps clock
    char time[64];
    sprintf(
            time,
            "Log__20%02d_%02d_%02d__%02d_%02d_%02d.csv",
            gpsData->year, gpsData->month, gpsData->day, gpsData->hour, gpsData->minute, gpsData->seconds
    );
    telemfilename = std::string(time);

    nvm_beginTelemetry();
    telemetryBegan = true;
  }

  // call write telemetry to write rows
  if(telemetryBegan) {
    nvm_writeTelemetry(vcuCoreOutput, hvcStatus, pduStatus, inverterStatus, analogVoltages, wheelMagnetValues, imuData,
                       gpsData);
  }

}
