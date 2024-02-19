#include "nvm.h"
#include "fatfs.h"
#include "clock.h"
#include <sstream>

static void nvm_loadParameters(VcuParameters* vcuParameters) {
    FRESULT res;
    FIL fsrc;
    UINT br;
    BYTE buffer[4096];

    // open source file (drive 1)
    res = f_open(
            &fsrc,
            "VcuParams.dat",
            FA_OPEN_EXISTING | FA_READ
            );

    if (res) exit(res);

    // copy source to destination
    res = f_read(
            &fsrc,
            buffer,
            sizeof buffer,
            &br );

    if(res) {
        vcuParameters = (VcuParameters*)res;
    }

    // close open files
    f_close(&fsrc);

}

static void nvm_saveParameters(VcuParameters* vcuParameters) {
    FRESULT res;
    FIL fdst;
    UINT bw;

    // create destination file (drive 0)
    res = f_open(
            &fdst,
            "VcuParams.dat",
            FA_OPEN_EXISTING | FA_WRITE
    );

    if (res) exit(res);

    // copy source to destination
    while(true) {
        res = f_write(
                &fdst,
                vcuParameters,
                sizeof (vcuParameters),
                &bw
        );
        if(res || bw < sizeof vcuParameters) {
            break;
        }
    }

    // close open files
    f_close(&fdst);

}

static void nvm_beginTelemetry(uint64_t timestamp) {
    FRESULT res;
    FIL fcsv;

    // convert timestamp to string for file name
    std::stringstream ss;
    ss << timestamp;
    std::string time = ss.str();
    time += ".csv";

    // create new csv file and leave open to write telemetry
    res = f_open(
            &fcsv,
            time.c_str(),
            FA_CREATE_ALWAYS
    );

    if (res) exit(res);

}

static void nvm_writeTelemetry(TelemetryRow* telemetryRow) {
    FRESULT res;
    FIL fdst;
    UINT bw;

    // write row of data into file
    if (res) {
        f_write(
                &fdst,
                telemetryRow,
                sizeof (telemetryRow),
                &bw
        );
    }
}

void nvm_init(VcuParameters* vcuParameters) {
  FATFS fs;
  FRESULT res;

  //mount default drive
  f_mount(&fs, "", 0);

  // load vcu parameters
  nvm_loadParameters(vcuParameters);

  // create telemetry csv file
  // placeholder for timestamp but should get time from gps clock
  nvm_beginTelemetry(2023-11-02);

}

void nvm_periodic(VcuParameters* vcuParameters, VcuOutput *vcuCoreOutput,
                  HvcStatus *hvcStatus, PduStatus *pduStatus, InverterStatus *inverterStatus,
                  AnalogVoltages *analogVoltages, WheelDisplacements *wheelDisplacements,
                  ImuData *imuData, GpsData *gpsData) {

    // save vcu parameters if changed
    nvm_saveParameters(vcuParameters);

    // create telemetry row and write into csv file
    //TelemetryRow telemetryRow;
    //nvm_writeTelemetry(&telemetryRow);

}
