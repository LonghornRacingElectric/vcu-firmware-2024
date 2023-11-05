#include "inverter.h"
#include "can.h"

static CanRx voltageMailbox;
static CanRx currentMailbox;

void inverter_init() {
    // TODO implement

    // example:
    can_addMailbox(INV_VOLTAGE, &voltageMailbox);
    can_addMailbox(INV_CURRENT, &currentMailbox);
}

void inverter_getStatus(InverterStatus* status) {
    // example code

    if(voltageMailbox.isRecent) {
        status->voltage = voltageMailbox.data[3];
        voltageMailbox.isRecent = false;
        status->isRecent = true;
    }

    if(currentMailbox.isRecent) {
        status->current = currentMailbox.data[7];
        currentMailbox.isRecent = false;
        status->isRecent = true;
    }

}

void inverter_sendTorqueCommand(float torque) {
    static uint8_t tc[8];
    tc[0] = torque; // TODO super incorrect but this is an example
    can_send(VCU_INV_COMMAND, 8, tc);
}

void inverter_resetFaults() {
    // TODO implement
    // send a can message
}