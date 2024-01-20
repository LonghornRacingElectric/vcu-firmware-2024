#include "wheelspeeds.h"
#include "angel_can_ids.h"
#include "angel_can.h"

static CanInbox whs_fr_inbox;
static CanInbox whs_fl_inbox;
static CanInbox whs_br_inbox;
static CanInbox whs_bl_inbox;

void wheelspeeds_init() {
    can_addInbox(UNS_VCU_DISP_1, &whs_fr_inbox);
    can_addInbox(UNS_VCU_DISP_2, &whs_fl_inbox);
    can_addInbox(UNS_VCU_DISP_3, &whs_br_inbox);
    can_addInbox(UNS_VCU_DISP_4, &whs_bl_inbox);
}

void wheelspeeds_get(WheelDisplacements* wheelDisplacements) {
    if(whs_fr_inbox.isRecent) {
        wheelDisplacements->fr = (float) can_readBytes(whs_fr_inbox.data, 0, 1) / 100.0f;
        whs_fr_inbox.isRecent = false;
    }

    if(whs_fl_inbox.isRecent) {
        wheelDisplacements->fl = (float) can_readBytes(whs_fl_inbox.data, 0, 1) / 100.0f;
        whs_fl_inbox.isRecent = false;
    }

    if(whs_br_inbox.isRecent) {
        wheelDisplacements->br = (float) can_readBytes(whs_br_inbox.data, 0, 1) / 100.0f;
        whs_br_inbox.isRecent = false;
    }

    if(whs_bl_inbox.isRecent) {
        wheelDisplacements->bl = (float) can_readBytes(whs_bl_inbox.data, 0, 1) / 100.0f;
        whs_bl_inbox.isRecent = false;
    }
}
