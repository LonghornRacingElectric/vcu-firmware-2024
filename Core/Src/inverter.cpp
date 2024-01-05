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
}

void inverter_getStatus(InverterStatus* status) {

    if(inverterTempInbox.isRecent) {
        auto temp = can_readBytes(inverterTempInbox.data, 0, 1) +
                    can_readBytes(inverterTempInbox.data, 2, 3) +
                    can_readBytes(inverterTempInbox.data, 4, 5);
        status->inverterTemp = (float) temp / 30.0f;
        inverterTempInbox.isRecent = false;
        status->isRecent = true;
    }

    if(motorTempInbox.isRecent) {
        status->motorTemp = (float) can_readBytes(motorTempInbox.data, 4, 5) / 10.0f;
        motorTempInbox.isRecent = false;
        status->isRecent = true;
    }

    if(motorPosInbox.isRecent) {
        status->motorAngle = (float) can_readBytes(motorPosInbox.data, 0, 1) / 10.0f;
        status->rpm = (float) can_readBytes(motorPosInbox.data, 2, 3); //Out of all of these, idk why this isnt shifted
        status->inverterFrequency = (float) can_readBytes(motorPosInbox.data, 4, 5) / 10.0f;
        status->resolverAngle = (float) can_readBytes(motorPosInbox.data, 6, 7) / 10.0f;
        motorPosInbox.isRecent = false;
        status->isRecent = true;
    }

    if(voltageInbox.isRecent) {
        status->voltage = (float) can_readBytes(voltageInbox.data, 0, 1) / 10.0f;
        status->outputVoltage = (float) can_readBytes(voltageInbox.data, 2, 3) / 10.0f;
        status->ABVoltage = (float) can_readBytes(voltageInbox.data, 4, 5) / 10.0f;
        status->BCVoltage = (float) can_readBytes(voltageInbox.data, 6, 7) / 10.0f;
        voltageInbox.isRecent = false;
        status->isRecent = true;
    }

    if(currentInbox.isRecent) {
        status->phaseACurrent = (float) can_readBytes(currentInbox.data, 0, 1) / 10.0f;
        status->phaseBCurrent = (float) can_readBytes(currentInbox.data, 2, 3) / 10.0f;
        status->phaseCCurrent = (float) can_readBytes(currentInbox.data, 4, 5) / 10.0f;
        status->current = (float) can_readBytes(currentInbox.data, 6, 7) / 10.0f;
        currentInbox.isRecent = false;
        status->isRecent = true;
    }

    if(inverterStateInbox.isRecent) {
        status->stateVector = can_readBytes(inverterStateInbox.data, 0, 7);
        inverterStateInbox.isRecent = false;
        status->isRecent = true;
    }

    if(inverterFaultInbox.isRecent) {
        status->faultVector = can_readBytes(inverterFaultInbox.data, 0, 1);
        inverterFaultInbox.isRecent = false;
        status->isRecent = true;
    }

    if(torqueInfoInbox.isRecent) {
        status->torqueActual = (float) can_readBytes(torqueInfoInbox.data, 0, 1) / 10.0f;
        status->torqueCommand = (float) can_readBytes(torqueInfoInbox.data, 2, 3) / 10.0f;
        torqueInfoInbox.isRecent = false;
        status->isRecent = true;
    }

    if(paramsResponseInbox.isRecent) {
        auto data = (uint16_t) can_readBytes(paramsResponseInbox.data, 4, 5);
        if(paramsResponseInbox.data[2] == 0){
            FAULT_SET(&vcu_fault_vector, FAULT_VCU_INVPARAMS);
        }
        paramsResponseInbox.isRecent = false;
    }

}

void inverter_updateTorqueCommand(float torque, float rpm, bool enable_inverter) {
    auto tc = (int16_t) (torque * 10.0f);
    auto sc = (int16_t) rpm;
    can_writeBytes(torqueCommandOutbox.data, 0, 1, tc);
    can_writeBytes(torqueCommandOutbox.data, 2, 3, sc);
    torqueCommandOutbox.data[5] = (uint8_t) enable_inverter; // Enable
    torqueCommandOutbox.isRecent = true;
}

unsigned int inverter_resetFaults() {
    // send a can message telling the inverter to reset faults by setting addr 20 to 0
    return inverter_paramsIO(20, 0, true);
}

unsigned int inverter_paramsIO(uint16_t param_addr, uint16_t param_value, bool write){
    // send a can message telling the inverter to set params
    static uint8_t set_params[8];
    can_writeBytes(set_params, 0, 1, param_addr); //param addr
    can_writeBytes(set_params, 4, 5, param_value);  //param value
    set_params[2] = (uint8_t) write; //sets to write mode

    return can_send(VCU_INV_PARAMS_REQUEST, 8, set_params);
}