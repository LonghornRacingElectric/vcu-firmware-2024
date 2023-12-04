#include "wheelspeeds.h"
#include "angel_can.h"

static CanRx WheelSpeed_mailbox_FL;
static CanRx WheelSpeed_mailbox_FR;
static CanRx WheelSpeed_mailbox_BL;
static CanRx WheelSpeed_mailbox_BR;


void wheelspeeds_init() {
    can_addMailbox(WHS_VCU_INFO_FL, &WheelSpeed_mailbox_FL);
    can_addMailbox(WHS_VCU_INFO_FR, &WheelSpeed_mailbox_FR);
    can_addMailbox(WHS_VCU_INFO_BL, &WheelSpeed_mailbox_BL);
    can_addMailbox(WHS_VCU_INFO_BR, &WheelSpeed_mailbox_BR);

}

void wheelspeeds_get(WheelDisplacements* wheelDisplacements) {
    //CAN data is 4 bytes, first two are the wheel speed, the 3rd 4th are currently undetermined.

    //TODO determine if this will return fixed point or smth (if i have to convert the raw data)
    wheelDisplacements->fl = (float) can_readBytes(WheelSpeed_mailbox_FL.data, 0, 1); //
    wheelDisplacements->fr = (float) can_readBytes(WheelSpeed_mailbox_FR.data, 0, 1); //
    wheelDisplacements->bl = (float) can_readBytes(WheelSpeed_mailbox_BL.data, 0, 1); //
    wheelDisplacements->br = (float) can_readBytes(WheelSpeed_mailbox_BR.data, 0, 1); //

}
