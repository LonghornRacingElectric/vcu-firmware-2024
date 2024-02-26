#include "wheelspeeds.h"
#include "angel_can_ids.h"
#include "angel_can.h"

static CanInbox whs_fr_inbox;
static CanInbox whs_fl_inbox;
static CanInbox whs_br_inbox;
static CanInbox whs_bl_inbox;

void wheelspeeds_init() {
    can_addInbox(UNSFR_VCU_DISP, &whs_fr_inbox);
    can_addInbox(UNSFL_VCU_DISP, &whs_fl_inbox);
    can_addInbox(UNSBR_VCU_DISP, &whs_br_inbox);
    can_addInbox(UNSBL_VCU_DISP, &whs_bl_inbox);
}

void wheelspeeds_periodic(WheelDisplacements* wheelDisplacements) {
    if(whs_fr_inbox.isRecent) {
        wheelDisplacements->fr = (float) can_readBytes(whs_fr_inbox.data, 0, 3) / 10.0f;
        whs_fr_inbox.isRecent = false;
    }

    if(whs_fl_inbox.isRecent) {
        wheelDisplacements->fl = (float) can_readBytes(whs_fl_inbox.data, 0, 3) / 10.0f;
        whs_fl_inbox.isRecent = false;
    }

    if(whs_br_inbox.isRecent) {
        wheelDisplacements->br = (float) can_readBytes(whs_br_inbox.data, 0, 3) / 10.0f;
        whs_br_inbox.isRecent = false;
    }

    if(whs_bl_inbox.isRecent) {
        wheelDisplacements->bl = (float) can_readBytes(whs_bl_inbox.data, 0, 3) / 10.0f;
        whs_bl_inbox.isRecent = false;
    }
}
