#include "nvm.h"
#include "fatfs.h"
#include "clock.h"
#include <sstream>

FIL telemfile;
FATFS fs;
FRESULT res;
FIL fsrc;
FIL fdst;

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

  // convert timestamp to string for file name
  timestamp += ".csv";

  // create new csv file and leave open to write telemetry
  res = f_open(
      &telemfile,
      timestamp.c_str(),
      FA_CREATE_ALWAYS
  );

  if (res) {
    // TODO fault
  }

}

static void nvm_writeTelemetry(TelemetryRow *telemetryRow) {
  UINT bw;

  // write row of data into file
  f_write(
      &telemfile,
      telemetryRow,
      sizeof(telemetryRow),
      &bw
  );
}

void nvm_init(VcuParameters *vcuParameters) {
  //mount default drive
  f_mount(&fs, "", 0);

  // load vcu parameters
  nvm_loadParameters(vcuParameters);

  // create telemetry csv file
  // placeholder for timestamp but should get time from gps clock
//  nvm_beginTelemetry("2023-11-02");

}

void nvm_periodic(VcuParameters *vcuParameters, VcuOutput *vcuCoreOutput,
                  HvcStatus *hvcStatus, PduStatus *pduStatus, InverterStatus *inverterStatus,
                  AnalogVoltages *analogVoltages, WheelDisplacements *wheelDisplacements,
                  ImuData *imuData, GpsData *gpsData) {

  // save vcu parameters once a second
  static float time = 0;
  if (clock_getTime() >= time + 1) {
    nvm_saveParameters(vcuParameters);
    time = clock_getTime();
  }
  //nvm_writeTelemetry()
}
