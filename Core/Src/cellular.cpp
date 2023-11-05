#include "cellular.h"

void cellular_sendTelemetry(TelemetryPacket* telemetry) {
    // TODO implement
}

void cellular_getGps(GpsData* gpsData) {
    // TODO implement
}

bool cellular_areParametersUpdated() {
    // TODO implement
    return false;
}

void cellular_updateParameters(VcuParameters* vcuParameters) {
    // TODO implement
}

void cellular_periodic() {
    // TODO implement
}

static void cellular_sendAT(std::string command) {

}
