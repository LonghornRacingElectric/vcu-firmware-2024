#include "dash.h"
#include "angel_can.h"
#include "gps.h"
#include "all_imus.h"

CanOutbox dashOutbox;

// TODO only use core for this stuff
extern GpsData gpsData;
extern ImuData imuData;

void dash_init() {
  can_addOutbox(VCU_DASH_INFO1, 0.01f, &dashOutbox);
}

void dash_periodic(PduStatus* pduStatus, HvcStatus* hvcStatus, VcuOutput* vcuCoreOutput) {
  dashOutbox.dlc = 5;
//  dashOutbox.data[0] = gpsData.seconds;
  dashOutbox.data[0] = (int8_t) vcuCoreOutput->dashSpeed;
//  dashOutbox.data[1] = (int8_t) vcuCoreOutput->torque;
//  dashOutbox.data[2] = (int8_t) (vcuCoreOutput->power / 1000.0f);
//  dashOutbox.data[3] = (uint8_t) (hvcStatus->hvSoC * 100.0f);
//  dashOutbox.data[4] = (uint8_t) (pduStatus->lvSoC * 100.0f);
  dashOutbox.isRecent = true;
}
