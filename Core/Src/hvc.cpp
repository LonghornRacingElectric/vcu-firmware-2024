#include "hvc.h"
#include "angel_can.h"

#define VOLTS_MAILBOXES_NUM (HVC_VCU_CELL_VOLTAGES_END - HVC_VCU_CELL_VOLTAGES_START + 1)
#define TEMPS_MAILBOXES_NUM (HVC_VCU_CELL_TEMPS_END - HVC_VCU_CELL_TEMPS_START + 1)

static CanInbox voltageInboxes[VOLTS_MAILBOXES_NUM];
static CanInbox tempInboxes[TEMPS_MAILBOXES_NUM];
static CanInbox packStatusInbox;
static CanInbox amsImdInbox;
static CanInbox contactorStatusInbox;
static CanOutbox coolingOutbox;

void hvc_init() {
  can_addInboxes(HVC_VCU_CELL_VOLTAGES_START, HVC_VCU_CELL_VOLTAGES_END, voltageInboxes);
  can_addInboxes(HVC_VCU_CELL_TEMPS_START, HVC_VCU_CELL_TEMPS_END, tempInboxes);
  can_addInbox(HVC_VCU_PACK_STATUS, &packStatusInbox);
  can_addInbox(HVC_VCU_AMS_IMD, &amsImdInbox);
  can_addInbox(HVC_VCU_CONTACTOR_STATUS, &contactorStatusInbox);

  can_addOutbox(VCU_HVC_COOLING, 0.1f, &coolingOutbox);
}

void hvc_updateCooling(uint16_t battFanRpm, uint16_t battUniqueSegRpm) {
  coolingOutbox.dlc = 4;
  can_writeBytes(coolingOutbox.data, 0, 1, battFanRpm);
  can_writeBytes(coolingOutbox.data, 2, 3, battUniqueSegRpm);
  coolingOutbox.isRecent = true;
}

void hvc_periodic(HvcStatus *status, VcuOutput *vcuOutput) {
  // TODO vcuOutput->battFanRpm (need to update VCU core)
  hvc_updateCooling(1000, 1000);

  if (amsImdInbox.isRecent) {
    amsImdInbox.isRecent = false;
    status->imd = (bool) can_readBytes(amsImdInbox.data, 0, 0);
    status->ams = (bool) can_readBytes(amsImdInbox.data, 1, 1);
    status->isRecent = true;
  }
  if (packStatusInbox.isRecent) {
    packStatusInbox.isRecent = false;
    status->packVoltage = (float) can_readBytes(packStatusInbox.data, 0, 1) / 10.0f;
    status->packCurrent = (float) can_readBytes(packStatusInbox.data, 2, 3) / 10.0f;
    status->stateOfCharge = (float) can_readBytes(packStatusInbox.data, 4, 5) / 10.0f;
    status->isRecent = true;
  }
  if (contactorStatusInbox.isRecent) {
    contactorStatusInbox.isRecent = false;
    status->contactorStatus = (HvcStatus::ContactorStatus) contactorStatusInbox.data[0];
    status->isRecent = true;
  }

  // TODO still check voltage and temp mailboxes
}
