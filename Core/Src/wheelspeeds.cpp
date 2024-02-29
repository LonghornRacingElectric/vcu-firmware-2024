#include "wheelspeeds.h"
#include "angel_can_ids.h"
#include "angel_can.h"

static CanInbox whs_fr_inbox;
static CanInbox whs_fl_inbox;
static CanInbox whs_br_inbox;
static CanInbox whs_bl_inbox;

void wheelspeeds_init() {
    can_addInbox(UNSFR_VCU_MAGNET, &whs_fr_inbox);
    can_addInbox(UNSFL_VCU_MAGNET, &whs_fl_inbox);
    can_addInbox(UNSBR_VCU_MAGNET, &whs_br_inbox);
    can_addInbox(UNSBL_VCU_MAGNET, &whs_bl_inbox);
}

void wheelspeeds_periodic(WheelDisplacements* wheelDisplacements) {
    if(whs_fr_inbox.isRecent) {
        wheelDisplacements->fr = can_readFloat(uint32_t, &whs_fr_inbox, 0, 0.001f);
        whs_fr_inbox.isRecent = false;
    }

    if(whs_fl_inbox.isRecent) {
        wheelDisplacements->fl = can_readFloat(uint32_t, &whs_fl_inbox, 0, 0.001f);
        whs_fl_inbox.isRecent = false;
    }

    if(whs_br_inbox.isRecent) {
        wheelDisplacements->br = can_readFloat(uint32_t, &whs_br_inbox, 0, 0.001f);
        whs_br_inbox.isRecent = false;
    }

    if(whs_bl_inbox.isRecent) {
        wheelDisplacements->bl = can_readFloat(uint32_t, &whs_bl_inbox, 0, 0.001f);
        whs_bl_inbox.isRecent = false;
    }
}
