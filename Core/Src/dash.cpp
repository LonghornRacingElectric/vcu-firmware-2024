#include "dash.h"
#include "angel_can.h"

CanOutbox dashOutbox;

void dash_init() {
  can_addOutbox(VCU_DASH_INFO1, 0.01f, &dashOutbox);
}

void dash_periodic(PduStatus* pduStatus, HvcStatus* hvcStatus, VcuOutput* vcuCoreOutput) {
  dashOutbox.dlc = 5;
  // TODO modify VCU core to output dash info
//  dashOutbox.data[0] = (int8_t) vcuCoreOutput->speed;
//  dashOutbox.data[1] = (int8_t) vcuCoreOutput->torque;
//  dashOutbox.data[2] = (int8_t) (vcuCoreOutput->power / 1000.0f);
//  dashOutbox.data[3] = (uint8_t) (hvcStatus->hvSoC * 100.0f);
//  dashOutbox.data[4] = (uint8_t) (pduStatus->lvSoC * 100.0f);
  dashOutbox.isRecent = true;
}
