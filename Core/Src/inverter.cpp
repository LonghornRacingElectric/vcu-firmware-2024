#include <queue>
#include "inverter.h"
#include "angel_can.h"
#include "faults.h"
#include "timeouts.h"
#include "usb.h"
#include "clock.h"

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

typedef struct ParameterUpdate {
  uint16_t address;
  uint16_t value;
  float timeRemaining;
} ParameterUpdate;

static queue<ParameterUpdate> parameterUpdateQueue;

void inverter_init() {
  can_addInbox(INV_VOLTAGE, &voltageInbox, INV_TIMEOUT_FAST);
  can_addInbox(INV_CURRENT, &currentInbox, INV_TIMEOUT_FAST);
  can_addInbox(INV_TEMP3_DATA, &motorTempInbox, INV_TIMEOUT_SLOW);
  can_addInbox(INV_TEMP1_DATA, &inverterTempInbox, INV_TIMEOUT_SLOW);
  can_addInbox(INV_MOTOR_POSITIONS, &motorPosInbox, INV_TIMEOUT_FAST);
  can_addInbox(INV_STATE_CODES, &inverterStateInbox, INV_TIMEOUT_FAST);
  can_addInbox(INV_FAULT_CODES, &inverterFaultInbox, INV_TIMEOUT_FAST);
  can_addInbox(INV_TORQUE_TIMER, &torqueInfoInbox, INV_TIMEOUT_FAST);
  //can_addInbox(INV_HIGH_SPEED_MSG, &highSpeedInbox, INV_TIMEOUT_VERYFAST);
  can_addInbox(INV_VCU_PARAMS_RESPONSE, &paramsResponseInbox, 1.0f);

  can_addOutbox(VCU_INV_COMMAND, 0.003f, &torqueCommandOutbox);
  can_addOutbox(0x0C1, 0.1f, &paramsRequestOutbox);


//  inverter_writeParameter(148, 0x1CE5); // message selection
//  inverter_writeParameter(168, 10); // torque ramp, use 100 loaded
//  inverter_writeParameter(111, 6900); // motor over-speed fault RPM
//  inverter_writeParameter(128, 6500); // max RPM
//  inverter_writeParameter(127, 4000); // field weakening start RPM ("break speed")
//  inverter_writeParameter(169, 5100); // speed rate limit RPM
//  inverter_writeParameter(129, 2300); // torque limit
//  inverter_writeParameter(100, 3600); // Q axis current limit
//  inverter_writeParameter(101, 900); // D axis current limit
//  inverter_writeParameter(164, 100); // P gain
//  inverter_writeParameter(165, 360); // I gain
//  inverter_writeParameter(166, 0); // D gain
//  inverter_writeParameter(167, 0); // low-pass filter gain
//  inverter_writeParameter(187, 0); // shudder compensation
//  inverter_resetFaults();
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
    volatile bool resolverDisconnected = status->faultVector & 0x4000000000000000;
    volatile int x = 0;
    x++;
  }

  if (torqueInfoInbox.isRecent) {
    status->torqueCommand = can_readFloat(int16_t, &torqueInfoInbox, 0, 0.1f);
    status->torqueActual = can_readFloat(int16_t, &torqueInfoInbox, 2, 0.1f);
    torqueInfoInbox.isRecent = false;
    status->isRecent = true;
  }

  if(torqueInfoInbox.isTimeout || inverterFaultInbox.isTimeout ||
    inverterStateInbox.isTimeout || currentInbox.isTimeout ||
    voltageInbox.isTimeout || motorPosInbox.isTimeout ||
    motorTempInbox.isTimeout || inverterTempInbox.isTimeout) {
    FAULT_SET(&faultVector, FAULT_VCU_INV);
  }
  else {
    FAULT_CLEAR(&faultVector, FAULT_VCU_INV);
  }

  if (paramsResponseInbox.isRecent) {
    status->newData = can_readInt(uint16_t, &paramsResponseInbox, 4);
    paramsResponseInbox.isRecent = false;
  }
}


static void inverter_updateTorqueCommand(float torque, float rpm, bool enable_inverter) {
  can_writeFloat(int16_t, &torqueCommandOutbox, 0, torque, 0.1f);
  can_writeFloat(int16_t, &torqueCommandOutbox, 2, rpm, 1.0f);
  torqueCommandOutbox.data[4] = 1; // direction bool
  torqueCommandOutbox.data[5] = (uint8_t) enable_inverter; // Enable
  torqueCommandOutbox.data[6] = 0;
  torqueCommandOutbox.data[7] = 0;
  torqueCommandOutbox.dlc = 8;
  torqueCommandOutbox.isRecent = true;
}

void inverter_resetFaults() {
  // send a can message telling the inverter to reset faults by setting addr 20 to 0
  return inverter_writeParameter(20, 0);
}

void inverter_writeParameter(uint16_t address, uint16_t value) {
  ParameterUpdate parameterUpdate = {
    address, value, 1.0f
  };
  parameterUpdateQueue.push(parameterUpdate);
}

void inverter_readParameter(uint16_t address) {
  can_writeInt(uint16_t, &paramsRequestOutbox, 0, address); // param addr
  can_writeInt(uint8_t, &paramsRequestOutbox, 2, 0); // read
  paramsRequestOutbox.dlc = 8;
}

void inverter_updateParameterCommand(float deltaTime) {
  if (parameterUpdateQueue.empty()) {
    return;
  }

  ParameterUpdate parameterUpdate = parameterUpdateQueue.front();

  // send a can message telling the inverter to set params
  can_writeInt(uint16_t, &paramsRequestOutbox, 0, parameterUpdate.address); // param addr
  can_writeInt(uint8_t, &paramsRequestOutbox, 2, 1); // write
  can_writeInt(uint16_t, &paramsRequestOutbox, 4, parameterUpdate.value); // param value
  paramsRequestOutbox.dlc = 8;

  // weird inconsistency in cascadia documentation
  if(parameterUpdate.address == 148) {
    paramsRequestOutbox.data[6] = 0xFE;
    paramsRequestOutbox.data[7] = 0xFF;
  } else {
    paramsRequestOutbox.data[6] = 0;
    paramsRequestOutbox.data[7] = 0;
  }

  parameterUpdate.timeRemaining -= deltaTime;
  if(parameterUpdate.timeRemaining <= 0.0f && clock_getTime() > 3.0f) {
    parameterUpdateQueue.pop();
  }
}

void inverter_periodic(InverterStatus *status, VcuOutput* vcuCoreOutput, float deltaTime) {
  inverter_getStatus(status);
  inverter_updateTorqueCommand(vcuCoreOutput->inverterTorqueRequest, 0, vcuCoreOutput->enableInverter);
  inverter_updateParameterCommand(deltaTime);
//  inverter_readParameter(165);
}