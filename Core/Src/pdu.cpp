#include "pdu.h"
#include "angel_can.h"
#include "faults.h"

static CanInbox lvbattStatusInbox;
static CanInbox waterStatusInbox;
static CanInbox airStatusInbox; // TODO unused

static CanOutbox brakeLightOutbox;
static CanOutbox buzzerOutbox;
static CanOutbox coolingOutbox;

void pdu_init() {
  can_addInbox(PDU_VCU_LVBAT, &lvbattStatusInbox);
  can_addInbox(PDU_VCU_THERMAL, &waterStatusInbox);

  can_addOutbox(VCU_PDU_BRAKELIGHT, 0.01f, &brakeLightOutbox);
  can_addOutbox(VCU_PDU_BUZZER, 0.01f, &buzzerOutbox);
  can_addOutbox(VCU_PDU_COOLING, 0.25f, &coolingOutbox);
}

static void pdu_updateBrakeLight(float brightnessLeft, float brightnessRight) {
  brakeLightOutbox.data[0] = (uint8_t) (brightnessLeft * 255.0f);
  brakeLightOutbox.data[1] = (uint8_t) (brightnessRight * 255.0f);
  brakeLightOutbox.isRecent = true;
}

static void pdu_updateBuzzer(BuzzerType buzzerType) {
  buzzerOutbox.data[0] = (uint8_t) buzzerType;
  buzzerOutbox.isRecent = true;
}

static void pdu_updateCoolingOutput(float radiatorFanRpm, float pumpPercentage) {
  can_writeBytes(coolingOutbox.data, 0, 1, (uint16_t) radiatorFanRpm); // max will occur at 8300 rpm, but higher will just be ignored
  coolingOutbox.data[2] = (uint8_t) pumpPercentage;
  coolingOutbox.isRecent = true;
}

void pdu_periodic(PDUStatus *status, VcuOutput *vcuOutput, float deltaTime) {

  // TODO update vcu core to output brake lights
  // TODO pdu_sendBrakeLight(vcuOutput->brakeLightLeft)
  pdu_updateBuzzer(vcuOutput->r2dBuzzer ? BUZZER_BUZZ : BUZZER_SILENT);
  // TODO update vcu core to output cooling percentages
  // TODO pdu_sendCoolingOutput(vcuOutput->radiatorFanRpm, vcuOutput->pumpPercentage);

  if (lvbattStatusInbox.isRecent) {
    lvbattStatusInbox.isRecent = false;
    status->lvVoltage =
        (float) can_readBytes(lvbattStatusInbox.data, 0, 1) / 10.0f; // in 0.1V units, range is 0.0 - 28.0V
    status->lvSoC = (float) can_readBytes(lvbattStatusInbox.data, 2, 3) / 10.0f; // in 0.1% units
    status->lvCurrent =
        (float) can_readBytes(lvbattStatusInbox.data, 4, 5) / 100.0f; // in 0.01A units, range is 0.00 - 100.00A
    status->isRecent = true;
  }
  if (waterStatusInbox.isRecent) {
    waterStatusInbox.isRecent = false;
    status->volumetricFlowRate = (float) waterStatusInbox.data[0] / 10.0f; // range from 0.0 to 25.5 L/min
    status->waterTempMotor = (float) waterStatusInbox.data[1]; // degrees C
    status->waterTempInverter = (float) waterStatusInbox.data[2]; // degrees C
    status->waterTempRadiator = (float) waterStatusInbox.data[3]; // degrees C
    status->radiatorFanRpm =
        (float) can_readBytes(airStatusInbox.data, 0, 1) / 10.0f; // TODO this needs to be looked at
    status->isRecent = true;
  }
}