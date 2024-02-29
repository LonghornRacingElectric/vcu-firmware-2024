#include "nvm.h"

static void nvm_loadParameters(VcuParameters* vcuParameters) {
  // TODO implement
}

static void nvm_saveParameters(VcuParameters* vcuParameters) {
  // TODO implement
}

static void nvm_beginTelemetry(uint64_t timestamp) {
  // TODO implement
}

static void nvm_writeTelemetry(TelemetryRow* telemetryRow) {
  // TODO implement
}

void nvm_init() {
  // TODO implement
}

void nvm_periodic(VcuParameters* vcuParameters, VcuOutput *vcuCoreOutput,
                  HvcStatus *hvcStatus, PduStatus *pduStatus, InverterStatus *inverterStatus,
                  AnalogVoltages *analogVoltages, WheelMagnetValues *wheelMagnetValues,
                  ImuData *imuData, GpsData *gpsData) {
  // TODO implement
}
