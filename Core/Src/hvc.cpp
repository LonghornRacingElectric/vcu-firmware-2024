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

float hvc_updateMean(const float oldData[], const float newData[4], float oldMean, size_t index, size_t numData) {
  float sum = oldMean * numData;
  for(int i = 0; i < 4; i++) { // Out with the old, in with the new
    if(numData < 4*index + i) {
      return oldMean; // This should never happen
    }
    sum -= oldData[4*index + i];
    sum += newData[i];
  }
  return sum / numData;
}

float hvc_findMin(const float newData[4], float prevMin) {
  for(int i = 0; i < 4; i++) {
    if(newData[i] < prevMin) return newData[i];
  }
  return prevMin;
}

float hvc_findMax(const float newData[4], float prevMax) {
  for(int i = 0; i < 4; i++) {
    if(newData[i] > prevMax) return newData[i];
  }
  return prevMax;
}

float hvc_findRange(float min, float max) {
  return max - min;
}


void hvc_periodic(HvcStatus *status, VcuOutput *vcuOutput) {
  // Atm there is no separation between normal segment and unique segment, both will be a value between 0 and 1
  hvc_updateCooling(vcuOutput->batteryFansOutput, vcuOutput->batteryFansOutput);

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


  // Ok so this will be more efficient only if hvc_periodic() is called faster than every 100 us (it takes about 100us for new voltage data to come in)
  for(int i = 0; i < VOLTS_MAILBOXES_NUM; i++) {
    auto voltageInbox = voltageInboxes[i];
    if(voltageInbox.isRecent) {
      voltageInbox.isRecent = false;
      float newVoltages[4] = {
        (float) can_readBytes(voltageInbox.data, 0, 1) / 10.0f,
        (float) can_readBytes(voltageInbox.data, 2, 3) / 10.0f,
        (float) can_readBytes(voltageInbox.data, 4, 5) / 10.0f,
        (float) can_readBytes(voltageInbox.data, 6, 7) / 10.0f
      };

      status->packVoltageMean = hvc_updateMean(status->cellVoltages, newVoltages, status->packVoltageMean, i, 4*VOLTS_MAILBOXES_NUM);
      status->packVoltageMin = hvc_findMin(newVoltages, status->packVoltageMin);
      status->packVoltageMax = hvc_findMax(newVoltages, status->packVoltageMax);
      status->packVoltageRange = hvc_findRange(status->packVoltageMin, status->packVoltageMax);

      status->cellVoltages[4*i] = newVoltages[0];
      status->cellVoltages[4*i + 1] = newVoltages[1];
      status->cellVoltages[4*i + 2] = newVoltages[2];
      status->cellVoltages[4*i + 3] = newVoltages[3];
      status->isRecent = true;
    }
  }

  for(int i = 0; i < TEMPS_MAILBOXES_NUM; i++) {
    auto tempInbox = tempInboxes[i];
    if(tempInbox.isRecent) {
      tempInbox.isRecent = false;
      float newTemps[4] = {
              (float) can_readBytes(tempInbox.data, 0, 1) / 10.0f,
              (float) can_readBytes(tempInbox.data, 2, 3) / 10.0f,
              (float) can_readBytes(tempInbox.data, 4, 5) / 10.0f,
              (float) can_readBytes(tempInbox.data, 6, 7) / 10.0f
      };

      status->packTempMean = hvc_updateMean(status->cellTemps, newTemps, status->packTempMean, i, 4*TEMPS_MAILBOXES_NUM);
      status->packTempMin = hvc_findMin(newTemps, status->packTempMin);
      status->packTempMax = hvc_findMax(newTemps, status->packTempMax);
      status->packTempRange = hvc_findRange(status->packTempMin, status->packTempMax);

      status->cellTemps[4*i] = newTemps[0];
      status->cellTemps[4*i + 1] = newTemps[1];
      status->cellTemps[4*i + 2] = newTemps[2];
      status->cellTemps[4*i + 3] = newTemps[3];
      status->isRecent = true;
    }
  }


}
