#include "nvm.h"
#include "fatfs.h"
#include "clock.h"
#include <sstream>

static void nvm_loadParameters(VcuParameters* vcuParameters) {
    FATFS fs;
    FRESULT res;
    FIL fsrc;
    UINT br;
    BYTE buffer[4096];

    // mount work area
    f_mount(&fs, "", 1);

    // open source file (drive 1)
    res = f_open(
            &fsrc,
            "1:srcfile.dat",
            FA_OPEN_EXISTING | FA_READ
            );

    if (res) exit(res);

    // copy source to destination
        res = f_read(
                &fsrc,
                buffer,
                sizeof buffer,
                &br
        );
        if(res || br == 0) {

        } else {
            vcuParameters = (VcuParameters *) res;
        }

    // close open files
    f_close(&fsrc);

}

static void nvm_saveParameters(VcuParameters* vcuParameters) {
    FATFS fs;
    FRESULT res;
    FIL fdst;
    UINT bw;

    // mount work area
    f_mount(&fs, "", 0);

    // create destination file (drive 0)
    res = f_open(
            &fdst,
            "0:dstfile.dat",
            FA_OPEN_EXISTING | FA_WRITE
    );

    if (res) exit(res);

    // copy source to destination
    while(true) {
        res = f_write(
                &fdst,
                vcuParameters,
                sizeof vcuParameters,
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
    FATFS fs;
    FRESULT res;
    FIL fcsv;

    // mount work area
    f_mount(&fs, "", 0);

    // convert timestamp to string for file name
    std::stringstream ss;
    ss << timestamp;
    std::string time = ss.str();

    // create new csv file (drive 0)
    res = f_open(
            &fcsv,
            time.c_str(),
            FA_CREATE_ALWAYS
    );

    if (res) exit(res);

    // close open files
    f_close(&fcsv);
}

static void nvm_writeTelemetry(TelemetryRow* telemetryRow) {
    FATFS fs;
    FRESULT res;
    FIL fdst;
    BYTE buffer[4096];
    UINT bw;

    // mount work area
    f_mount(&fs, "", 0);

    // open source file (drive 1)
    res = f_open(
            &fdst,
            "0:dstfile.csv",
            FA_OPEN_EXISTING | FA_WRITE
    );

    // write row of data into file
    if (res) {
        f_write(
                &fdst,
                telemetryRow,
                sizeof telemetryRow,
                &bw
        );
    }
}

void nvm_init() {
  FATFS fs;
  FRESULT res;

  //mount default drive
  f_mount(&fs, "", 0);

  // load vcu parameters
  nvm_loadParameters();

  // create telemetry csv file
  nvm_beginTelemetry(clock_getDeltaTime());

}

void nvm_periodic(VcuParameters* vcuParameters, VcuOutput *vcuCoreOutput,
                  HvcStatus *hvcStatus, PduStatus *pduStatus, InverterStatus *inverterStatus,
                  AnalogVoltages *analogVoltages, WheelDisplacements *wheelDisplacements,
                  ImuData *imuData, GpsData *gpsData) {

    // save vcu parameters
    nvm_saveParameters(vcuParameters);

    // write row of telemetry data to file

}
