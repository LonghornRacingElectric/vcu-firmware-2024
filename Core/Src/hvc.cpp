#include "hvc.h"
#include "angel_can.h"

static CanRx Hvc_mailbox_voltage;
static CanRx Hvc_mailbox_current;
static CanRx Hvc_mailbox_batteryTemp;
static CanRx Hvc_mailbox_imd;
static CanRx Hvc_mailbox_ams;


void hvc_init() {
    can_addMailbox(HVC_VCU_INFO_VOLTAGE,&Hvc_mailbox_voltage);

}

void hvc_getStatus(HvcStatus* status) {
    // TODO implement
}
