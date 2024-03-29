#include "hvc.h"

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

void hvc_updateCooling(float battFanRpmPercentage, float battUniqueSegRpmPercentage) {
  coolingOutbox.dlc = 2;
  coolingOutbox.data[0] = (uint8_t) (battFanRpmPercentage * 100);
  coolingOutbox.data[1] = (uint8_t) (battUniqueSegRpmPercentage * 100);
  coolingOutbox.isRecent = true;
}

float hvc_updateMean(const float* newData, size_t numData) {
  float sum = 0;
  size_t removed = 0;
  for(int i = 0; i < numData; i++) {
    if(newData[i] == 0){
      removed++;
      continue;
    }
    sum += newData[i];
  }
  return sum / (float) (numData - removed);
}

float hvc_findMin(const float* newData, size_t numData) {
  float newMin = 0;
  for(int i = 0; i < numData; i++) {
    if(newData[i] == 0) continue;
    if(newData[i] < newMin) newMin = newData[i];
  }
  return newMin;
}

float hvc_findMax(const float* newData, size_t numData) {
  float newMax = 0;
  for(int i = 0; i < numData; i++) {
    if(newData[i] == 0) continue;
    if(newData[i] > newMax) newMax = newData[i];
  }
  return newMax;
}

float hvc_findRange(float min, float max) {
  return max - min;
}


void hvc_periodic(HvcStatus *status, VcuOutput *vcuOutput) {
  // Atm there is no separation between normal segment and unique segment, both will be a value between 0 and 1
  hvc_updateCooling(vcuOutput->batteryFansOutput, vcuOutput->batteryFansOutput);

  if (amsImdInbox.isRecent) {
    amsImdInbox.isRecent = false;
    status->imd = (bool) amsImdInbox.data[0];
    status->ams = (bool) amsImdInbox.data[1];
    status->isRecent = true;
  }
  if (packStatusInbox.isRecent) {
    packStatusInbox.isRecent = false;
    status->packVoltage = can_readFloat(uint16_t, &packStatusInbox, 0, 0.01f);
    status->packCurrent = can_readFloat(int16_t, &packStatusInbox, 2, 0.01f);
    status->stateOfCharge = can_readFloat(uint16_t, &packStatusInbox, 4, 0.01f);
    status->packTempMax = can_readInt(uint8_t, &packStatusInbox, 6);
    status->packTempMin = can_readInt(uint8_t, &packStatusInbox, 7);
    status->isRecent = true;
  }
  if (contactorStatusInbox.isRecent) {
    contactorStatusInbox.isRecent = false;
    status->contactorStatus = (HvcStatus::ContactorStatus) contactorStatusInbox.data[0];
    status->isRecent = true;
  }

  // Ok so this will be more efficient only if hvc_periodic() is called faster than every 100 us (it takes about 100us for new voltage data to come in)
  for(int i = 0; i < VOLTS_MAILBOXES_NUM; i++) {
    auto voltageInbox = voltageInboxes[i];
    if(voltageInbox.isRecent) {
      voltageInbox.isRecent = false;

      status->cellVoltages[4*i] = can_readFloat(uint16_t, &voltageInbox, 0, 0.0001f);
      status->cellVoltages[4*i + 1] = can_readFloat(uint16_t, &voltageInbox, 2, 0.0001f);
      status->cellVoltages[4*i + 2] = can_readFloat(uint16_t, &voltageInbox, 4, 0.0001f);
      status->cellVoltages[4*i + 3] = can_readFloat(uint16_t, &voltageInbox, 6, 0.0001f);
      status->isRecent = true;

      if(i == VOLTS_MAILBOXES_NUM - 1){ // If this is the last voltage inbox
        status->packVoltageMean = hvc_updateMean(status->cellVoltages, VOLTS_MAILBOXES_NUM*4);
        status->packVoltageMin = hvc_findMin(status->cellVoltages, VOLTS_MAILBOXES_NUM*4);
        status->packVoltageMax = hvc_findMax(status->cellVoltages, VOLTS_MAILBOXES_NUM*4);
        status->packVoltageRange = hvc_findRange(status->packVoltageMin, status->packVoltageMax);
      }
    }
  }

  for(int i = 0; i < TEMPS_MAILBOXES_NUM; i++) {
    auto tempInbox = tempInboxes[i];
    if(tempInbox.isRecent) {
      tempInbox.isRecent = false;

      status->cellTemps[4*i] = can_readFloat(uint16_t, &tempInbox, 0, 0.1f);
      status->cellTemps[4*i + 1] = can_readFloat(uint16_t, &tempInbox, 2, 0.1f);
      status->cellTemps[4*i + 2] = can_readFloat(uint16_t, &tempInbox, 4, 0.1f);
      status->cellTemps[4*i + 3] = can_readFloat(uint16_t, &tempInbox, 6, 0.1f);
      status->isRecent = true;

      if(i == TEMPS_MAILBOXES_NUM - 1){ // If this is the last temp inbox
        status->packTempMean = hvc_updateMean(status->cellTemps, TEMPS_MAILBOXES_NUM*4);
        // PackTemp Min and Max were already sent through CAN
        status->packTempRange = hvc_findRange(status->packTempMin, status->packTempMax);
      }
    }
  }


}
