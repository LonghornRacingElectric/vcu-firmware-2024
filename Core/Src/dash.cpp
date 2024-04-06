#include "dash.h"
#include "angel_can.h"
#include "VcuModel.h"

CanOutbox dashOutbox;

void dash_init() {
  can_addOutbox(VCU_DASH_INFO1, 0.01f, &dashOutbox);
}

void dash_periodic(PduStatus* pduStatus, HvcStatus* hvcStatus, VcuOutput* vcuCoreOutput) {
  dashOutbox.dlc = 5;
  // TODO modify VCU core to output dash info
  // Minimum and Maximum speedss for a corner
  // Lap time
  // Brake and Gas Pedal %s
  // Fault Errors
  // Battery
  if(vcuCoreOutput->dashSpeed > 0) {
    volatile int x = 0;
    x++;
  }
  dashOutbox.data[0] = (int8_t) vcuCoreOutput->dashSpeed;
//  dashOutbox.data[1] = (int8_t) vcuCoreOutput->torque;
//  dashOutbox.data[2] = (int8_t) (vcuCoreOutput->power / 1000.0f);
//  dashOutbox.data[3] = (uint8_t) (hvcStatus->hvSoC * 100.0f);
//  dashOutbox.data[4] = (uint8_t) (pduStatus->lvSoC * 100.0f);
  dashOutbox.isRecent = true;
}
