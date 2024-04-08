#include "inverter.h"
#include "fdcan.h"
#include "angel_can.h"
#include "faults.h"

using namespace std;


static CanInbox voltageInbox;
static CanInbox currentInbox;
static CanInbox motorTempInbox;
static CanInbox inverterTempInbox;
static CanInbox motorPosInbox;
static CanInbox inverterStateInbox;
static CanInbox inverterFaultInbox;
static CanInbox torqueInfoInbox;
static CanInbox highSpeedInbox;
static CanInbox paramsResponseInbox;

static CanOutbox torqueCommandOutbox;
static CanOutbox paramsRequestOutbox;

void inverter_init() {
  can_addInbox(INV_VOLTAGE, &voltageInbox);
  can_addInbox(INV_CURRENT, &currentInbox);
  can_addInbox(INV_TEMP3_DATA, &motorTempInbox);
  can_addInbox(INV_TEMP1_DATA, &inverterTempInbox);
  can_addInbox(INV_MOTOR_POSITIONS, &motorPosInbox);
  can_addInbox(INV_STATE_CODES, &inverterStateInbox);
  can_addInbox(INV_FAULT_CODES, &inverterFaultInbox);
  can_addInbox(INV_TORQUE_TIMER, &torqueInfoInbox);
  // can_addInbox(INV_HIGH_SPEED_MSG, &highSpeedInbox);
  can_addInbox(INV_VCU_PARAMS_RESPONSE, &paramsResponseInbox);

  can_addOutbox(VCU_INV_COMMAND, 0.003f, &torqueCommandOutbox);
  can_addOutbox(VCU_INV_PARAMS_REQUEST, 1.0f, &paramsRequestOutbox);
}

static void inverter_getStatus(InverterStatus *status) {

  if (inverterTempInbox.isRecent) {
    auto temp = can_readFloat(int16_t, &inverterTempInbox, 0, 0.1f) +
                can_readFloat(int16_t, &inverterTempInbox, 2, 0.1f) +
                can_readFloat(int16_t, &inverterTempInbox, 4, 0.1f);
    status->inverterTemp = temp / 3.0f;
    inverterTempInbox.isRecent = false;
    status->isRecent = true;
  }

  if (motorTempInbox.isRecent) {
    status->motorTemp = can_readFloat(int16_t, &motorTempInbox, 4, 0.1f);
    motorTempInbox.isRecent = false;
    status->isRecent = true;
  }

  if (motorPosInbox.isRecent) {
    status->motorAngle = can_readFloat(int16_t, &motorPosInbox, 0, 0.1f);
    status->rpm = can_readInt(int16_t, &motorPosInbox, 2); //Out of all of these, idk why this isnt shifted
    status->inverterFrequency = can_readFloat(int16_t, &motorPosInbox, 4, 0.1f);
    status->resolverAngle = can_readFloat(int16_t, &motorPosInbox, 6, 0.1f);
    motorPosInbox.isRecent = false;
    status->isRecent = true;
  }

  if (voltageInbox.isRecent) {
    status->voltage = can_readFloat(int16_t, &voltageInbox, 0, 0.1f);
    status->outputVoltage = can_readFloat(int16_t, &voltageInbox, 2, 0.1f);
    status->ABVoltage = can_readFloat(int16_t, &voltageInbox, 4, 0.1f);
    status->BCVoltage = can_readFloat(int16_t, &voltageInbox, 6, 0.1f);
    voltageInbox.isRecent = false;
    status->isRecent = true;
  }

  if (currentInbox.isRecent) {
    status->phaseACurrent = can_readFloat(int16_t, &currentInbox, 0, 0.1f);
    status->phaseBCurrent = can_readFloat(int16_t, &currentInbox, 2, 0.1f);
    status->phaseCCurrent = can_readFloat(int16_t, &currentInbox, 4, 0.1f);
    status->current = can_readFloat(int16_t, &currentInbox, 6, 0.1f);
    currentInbox.isRecent = false;
    status->isRecent = true;
  }

  if (inverterStateInbox.isRecent) {
    status->stateVector = can_readInt(uint64_t, &inverterStateInbox, 0);
    inverterStateInbox.isRecent = false;
    status->isRecent = true;
  }

  if (inverterFaultInbox.isRecent) {
    status->faultVector = can_readInt(uint64_t, &inverterFaultInbox, 0);
    inverterFaultInbox.isRecent = false;
    status->isRecent = true;
  }

  if (torqueInfoInbox.isRecent) {
    status->torqueCommand = can_readFloat(int16_t, &torqueInfoInbox, 0, 0.1f);
    status->torqueActual = can_readFloat(int16_t, &torqueInfoInbox, 2, 0.1f);
    torqueInfoInbox.isRecent = false;
    status->isRecent = true;
  }

  if (paramsResponseInbox.isRecent) {
    auto data = can_readInt(uint16_t, &paramsResponseInbox, 4);
    if (paramsResponseInbox.data[2] == 0) {
      FAULT_SET(&vcu_fault_vector, FAULT_VCU_INV_PARAMS);
    }
    else{
      status->newData = data;
    }
    paramsResponseInbox.isRecent = false;
  }
}


static void inverter_updateTorqueCommand(float torque, float rpm, bool enable_inverter) {
  can_writeFloat(int16_t, &torqueCommandOutbox, 0, torque, 0.1f);
  can_writeFloat(int16_t, &torqueCommandOutbox, 2, rpm, 1.0f);
  torqueCommandOutbox.data[5] = (uint8_t) enable_inverter; // Enable
  torqueCommandOutbox.isRecent = true;
}

unsigned int inverter_resetFaults() {
  // send a can message telling the inverter to reset faults by setting addr 20 to 0
  return inverter_paramsIO(20, 0, true);
}

unsigned int inverter_paramsIO(uint16_t param_addr, uint16_t param_value, bool write) {
  // send a can message telling the inverter to set params
  can_writeInt(uint16_t, &paramsRequestOutbox, 0, param_addr); //param addr
  can_writeInt(uint8_t, &paramsRequestOutbox, 2, write); //param value (dummy value)
  can_writeInt(uint16_t, &paramsRequestOutbox, 4, param_value); //param value
  paramsRequestOutbox.isRecent = true;
  return 0;
}

void inverter_periodic(InverterStatus *status, VcuOutput* vcuCoreOutput) {
  inverter_getStatus(status);
  inverter_updateTorqueCommand(vcuCoreOutput->inverterTorqueRequest, 0, vcuCoreOutput->enableInverter);
}