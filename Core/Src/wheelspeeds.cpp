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

void wheelspeeds_periodic(WheelMagnetValues* wheelMagnetValues) {
    if(whs_fr_inbox.isRecent) {
      wheelMagnetValues->fr = can_readFloat(int32_t, &whs_fr_inbox, 0, 0.00714f);
        whs_fr_inbox.isRecent = false;
    }

    if(whs_fl_inbox.isRecent) {
      wheelMagnetValues->fl = can_readFloat(int32_t, &whs_fl_inbox, 0, 0.00714f);
        whs_fl_inbox.isRecent = false;
    }

    if(whs_br_inbox.isRecent) {
      wheelMagnetValues->br = can_readFloat(int32_t, &whs_br_inbox, 0, 0.00714f);
        whs_br_inbox.isRecent = false;
    }

    if(whs_bl_inbox.isRecent) {
      wheelMagnetValues->bl = can_readFloat(int32_t, &whs_bl_inbox, 0, 0.00714f);
        whs_bl_inbox.isRecent = false;
    }
}
