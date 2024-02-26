#include "pdu.h"
#include "angel_can.h"
#include "faults.h"

static CanInbox lvbattStatusInbox;
static CanInbox thermalStatusInbox;

static CanOutbox brakeLightOutbox;
static CanOutbox buzzerOutbox;
static CanOutbox coolingOutbox;

void pdu_init() {
  can_addInbox(PDU_VCU_LVBAT, &lvbattStatusInbox);
  can_addInbox(PDU_VCU_THERMAL, &thermalStatusInbox);

  can_addOutbox(VCU_PDU_BRAKELIGHT, 0.01f, &brakeLightOutbox);
  can_addOutbox(VCU_PDU_BUZZER, 0.01f, &buzzerOutbox);
  can_addOutbox(VCU_PDU_COOLING, 0.1f, &coolingOutbox);
}

static void pdu_updateBrakeLight(float brightness) {
  brakeLightOutbox.dlc = 1;
  brakeLightOutbox.data[0] = (uint8_t) (brightness * 100.0f);
  brakeLightOutbox.isRecent = true;
}

static void pdu_updateBuzzer(BuzzerType buzzerType) {
  buzzerOutbox.dlc = 1;
  buzzerOutbox.data[0] = (uint8_t) buzzerType;
  buzzerOutbox.isRecent = true;
}

static void pdu_updateCoolingOutput(float radiatorFanRpmPercentage, float pumpPercentage) {
  coolingOutbox.dlc = 2;
  coolingOutbox.data[0] = (uint8_t) (radiatorFanRpmPercentage * 100.0f); // max will occur at 8300 rpm (100%), but higher will just be ignored
  coolingOutbox.data[1] = (uint8_t) (pumpPercentage * 100.0f);
  coolingOutbox.isRecent = true;
}

void pdu_periodic(PduStatus *status, VcuOutput *vcuOutput) {

  // At the moment, vcuCore only outputs a boolean for on and off in general.
  pdu_updateBrakeLight((float) vcuOutput->brakeLight);

  pdu_updateBuzzer(vcuOutput->r2dBuzzer ? BUZZER_BUZZ : BUZZER_SILENT);

  // At the moment, they are only set to max values by vcuCore
  // They are percentages, so they will be sent as floats between 0 and 1.
  pdu_updateCoolingOutput(vcuOutput->radiatorOutput, vcuOutput->pumpOutput);

  if (lvbattStatusInbox.isRecent) {
    lvbattStatusInbox.isRecent = false;
    status->lvVoltage =
        (float) can_readBytes(lvbattStatusInbox.data, 0, 1) / 10.0f; // in 0.1V units, range is 0.0 - 28.0V
    status->lvSoC = (float) can_readBytes(lvbattStatusInbox.data, 2, 3) / 10.0f; // in 0.1% units
    status->lvCurrent =
        (float) can_readBytes(lvbattStatusInbox.data, 4, 5) / 100.0f; // in 0.01A units, range is 0.00 - 100.00A
    status->isRecent = true;
  }
  if (thermalStatusInbox.isRecent) {
    thermalStatusInbox.isRecent = false;
    status->volumetricFlowRate = (float) thermalStatusInbox.data[0] / 10.0f; // range from 0.0 to 25.5 L/min
    status->waterTempMotor = (float) thermalStatusInbox.data[1]; // degrees C
    status->waterTempInverter = (float) thermalStatusInbox.data[2]; // degrees C
    status->waterTempRadiator = (float) thermalStatusInbox.data[3]; // degrees C
    status->radiatorFanRpmPercentage =
        (float) thermalStatusInbox.data[4] / 100.0f; // range from 0 to 100, but will be sent as a float between 0 and 1
    status->isRecent = true;
  }
}