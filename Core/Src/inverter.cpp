#include "inverter.h"
#include "fdcan.h"
#include "can.h"
#include "faults.h"
using namespace std;


static CanRx voltageMailbox;
static CanRx currentMailbox;
static CanRx motorTempMailbox;
static CanRx inverterTempMailbox;
static CanRx motorPosMailbox;
static CanRx inverterStateMailbox;
static CanRx inverterFaultMailbox;
static CanRx torqueInfoMailbox;
static CanRx highSpeedMailbox;
static CanRx paramsResponseMailbox;



void inverter_init() {
    // TODO implement

    // example:
    can_addMailbox(INV_VOLTAGE, 0x7FF, &voltageMailbox);
    can_addMailbox(INV_CURRENT, 0x7FF,&currentMailbox);
    can_addMailbox(INV_TEMP3_DATA, 0x7FF,&motorTempMailbox);
    can_addMailbox(INV_TEMP1_DATA, 0x7FF,&inverterTempMailbox);
    can_addMailbox(INV_MOTOR_POSITIONS, 0x7FF,&motorPosMailbox);
    can_addMailbox(INV_STATE_CODES, 0x7FF,&inverterStateMailbox);
    can_addMailbox(INV_FAULT_CODES, 0x7FF,&inverterFaultMailbox);
    can_addMailbox(INV_TORQUE_TIMER, 0x7FF,&torqueInfoMailbox);
    // can_addMailbox(INV_HIGH_SPEED_MSG, 0x7FF,&highSpeedMailbox);
    can_addMailbox(INV_VCU_PARAMS_RESPONSE, 0x7FF,&paramsResponseMailbox);

}

void inverter_getStatus(InverterStatus* status) {

    if(inverterTempMailbox.isRecent) {
        auto temp = can_readBytes(inverterTempMailbox.data, 0, 1) +
                    can_readBytes(inverterTempMailbox.data, 2, 3) +
                    can_readBytes(inverterTempMailbox.data, 4, 5);
        status->inverterTemp = (float) temp / 30.0f;
        inverterTempMailbox.isRecent = false;
        status->isRecent = true;
    }

    if(motorTempMailbox.isRecent) {
        status->motorTemp = (float) can_readBytes(motorTempMailbox.data, 4, 5) / 10.0f;
        motorTempMailbox.isRecent = false;
        status->isRecent = true;
    }

    if(motorPosMailbox.isRecent) {
        status->motorAngle = (float) can_readBytes(motorPosMailbox.data, 0, 1) / 10.0f;
        status->rpm = (float) can_readBytes(motorPosMailbox.data, 2, 3); //Out of all of these, idk why this isnt shifted
        status->inverterFrequency = (float) can_readBytes(motorPosMailbox.data, 4, 5) / 10.0f;
        status->resolverAngle = (float) can_readBytes(motorPosMailbox.data, 6, 7) / 10.0f;
        motorPosMailbox.isRecent = false;
        status->isRecent = true;
    }

    if(voltageMailbox.isRecent) {
        status->voltage = (float) can_readBytes(voltageMailbox.data, 0, 1) / 10.0f;
        status->outputVoltage = (float) can_readBytes(voltageMailbox.data, 2, 3) / 10.0f;
        status->ABVoltage = (float) can_readBytes(voltageMailbox.data, 4, 5) / 10.0f;
        status->BCVoltage = (float) can_readBytes(voltageMailbox.data, 6, 7) / 10.0f;
        voltageMailbox.isRecent = false;
        status->isRecent = true;
    }

    if(currentMailbox.isRecent) {
        status->phaseACurrent = (float) can_readBytes(currentMailbox.data, 0, 1) / 10.0f;
        status->phaseBCurrent = (float) can_readBytes(currentMailbox.data, 2, 3) / 10.0f;
        status->phaseCCurrent = (float) can_readBytes(currentMailbox.data, 4, 5) / 10.0f;
        status->current = (float) can_readBytes(currentMailbox.data, 6, 7) / 10.0f;
        currentMailbox.isRecent = false;
        status->isRecent = true;
    }

    if(inverterStateMailbox.isRecent) {
        status->stateVector = can_readBytes(inverterStateMailbox.data, 0, 7);
        inverterStateMailbox.isRecent = false;
        status->isRecent = true;
    }

    if(inverterFaultMailbox.isRecent) {
        status->faultVector = can_readBytes(inverterFaultMailbox.data, 0, 1);
        inverterFaultMailbox.isRecent = false;
        status->isRecent = true;
    }

    if(torqueInfoMailbox.isRecent) {
        status->torqueActual = (float) can_readBytes(torqueInfoMailbox.data, 0, 1) / 10.0f;
        status->torqueCommand = (float) can_readBytes(torqueInfoMailbox.data, 2, 3) / 10.0f;
        torqueInfoMailbox.isRecent = false;
        status->isRecent = true;
    }

    if(paramsResponseMailbox.isRecent) {
        auto data = (uint16_t) can_readBytes(paramsResponseMailbox.data, 4, 5);
        if(paramsResponseMailbox.data[2] == 0){
            fault_set(&vcu_fault_vector, FAULT_VCU_INVPARAMS);
        }
        paramsResponseMailbox.isRecent = false;
    }

}

unsigned int inverter_sendTorqueCommand(float torque, float rpm, bool enable_inverter) {
    static uint8_t torque_command[8];
    auto tc = (int16_t) (torque * 10.0f);
    auto sc = (int16_t) rpm;
    can_writeBytes(torque_command, 0, 1, tc);
    can_writeBytes(torque_command, 2, 3, sc);
    torque_command[5] = (uint8_t) enable_inverter; // Enable

    return can_send(VCU_INV_COMMAND, FDCAN_DATA_BYTES_8, torque_command);
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

    return can_send(VCU_INV_PARAMS_REQUEST, FDCAN_DATA_BYTES_8, set_params);
}