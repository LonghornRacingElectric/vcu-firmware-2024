#include "nvm.h"
#include "fatfs.h"
#include "clock.h"
#include "usb.h"
#include <sstream>

FIL telemfile;
FATFS fs;
FRESULT res;
FIL fsrc;
FIL fdst;
string telemfilename;

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
    return;
    // TODO fault
  }

  // copy source to destination
  res = f_write(
      &fdst,
      vcuParameters,
      sizeof(VcuParameters),
      &bw
  );
  if (res || bw < sizeof(VcuParameters)) {
    // TODO don't brick the car, but indicate we're out of storage with some kind of fault
  }

  // close open files
  f_close(&fdst);

}

static void nvm_beginTelemetry(std::string timestamp) {
  FRESULT res;

  // add timestamp info to file name
  telemfilename = timestamp + ".csv";

  // create new csv file and leave open to write telemetry
  res = f_open(
      &telemfile,
      telemfilename.c_str(),
      FA_CREATE_ALWAYS | FA_WRITE
  );

  if (res) {
    // TODO fault
  }

  // create headers for data
  f_printf(
          &telemfile,
          "%s,%s,%s,%s\r\n",
          "Time", "Acceleration X", "Acceleration Y", "Acceleration Z"
          );

  // close file to save
  f_close(&telemfile);

}

static void nvm_writeTelemetry(ImuData *imuData) {
  FRESULT res;
  // open telemfile
  res = f_open(
          &telemfile,
          telemfilename.c_str(),
          FA_OPEN_EXISTING | FA_WRITE | FA_OPEN_APPEND
          );
  if(res) {

  }
  // write row of data into file
  f_printf(
          &telemfile,
          "%f,%f,%f,%f",
          clock_getTime(), imuData->accelVcu.x, imuData->accelVcu.y, imuData->accelVcu.z
  );

  // close file to save
  f_close(&telemfile);
}

void nvm_init(VcuParameters *vcuParameters, GpsData *gpsData) {
  //mount default drive
  f_mount(&fs, "", 0);

  // load vcu parameters
  nvm_loadParameters(vcuParameters);

  // create telemetry csv file using time from gps clock
//  char time[20];
//  sprintf(
//          time,
//          "%04d/%02d/%02d %02d:%02d:%02d",
//          gpsData->year, gpsData->month, gpsData->day, gpsData->hour, gpsData->minute, gpsData->seconds
//          );
  nvm_beginTelemetry("2023-03-24");

}

void nvm_periodic(VcuParameters *vcuParameters, VcuOutput *vcuCoreOutput,
                  HvcStatus *hvcStatus, PduStatus *pduStatus, InverterStatus *inverterStatus,
                  AnalogVoltages *analogVoltages, WheelMagnetValues *wheelMagnetValues,
                  ImuData *imuData, GpsData *gpsData) {

  // save vcu parameters once a second
  static float time = 0;
  if (clock_getTime() >= time + 1) {
    nvm_saveParameters(vcuParameters);
    time = clock_getTime();
  }

  nvm_writeTelemetry(imuData);

}
