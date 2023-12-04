#include "hvc.h"
#include "angel_can.h"
#include <unordered_map>

#define VOLTS_MAILBOXES_NUM (HVC_VCU_CELL_VOLTAGES_END - HVC_VCU_CELL_VOLTAGES_START + 1)
#define TEMPS_MAILBOXES_NUM (HVC_VCU_CELL_TEMPS_END - HVC_VCU_CELL_TEMPS_START + 1)

static CanRx hvc_voltage_mailboxes[VOLTS_MAILBOXES_NUM];
static CanRx hvc_temp_mailboxes[TEMPS_MAILBOXES_NUM];
static CanRx hvc_pack_status_mailbox;
static CanRx hvc_ams_imd_mailbox;
static CanRx hvc_contactor_status_mailbox;

void hvc_init() {
    can_addMailboxes(HVC_VCU_CELL_VOLTAGES_START, HVC_VCU_CELL_VOLTAGES_END, hvc_voltage_mailboxes);
    can_addMailboxes(HVC_VCU_CELL_TEMPS_START, HVC_VCU_CELL_TEMPS_END, hvc_temp_mailboxes);
    can_addMailbox(HVC_VCU_PACK_STATUS, &hvc_pack_status_mailbox);
    can_addMailbox(HVC_VCU_AMS_IMD, &hvc_ams_imd_mailbox);
    can_addMailbox(HVC_VCU_CONTACTOR_STATUS, &hvc_contactor_status_mailbox);
}

uint32_t hvc_coolingRequest_send(bool battfan_turn_on, uint16_t battfan_rpm_req, uint16_t battfan_unique_rpm_req) {
    uint8_t data[5];
    data[0] = (uint8_t) battfan_turn_on;
    can_writeBytes(data, 1, 2, battfan_rpm_req );
    can_writeBytes(data, 3, 4, battfan_unique_rpm_req );

    return can_send(VCU_HVC_COOLING, 5, data);
}

void hvc_getStatus(HvcStatus* status) {
    if(hvc_ams_imd_mailbox.isRecent){ // Demand packet receive (when amd / imd changes)
        status->ok = true;
        status->imd = (bool) can_readBytes(hvc_ams_imd_mailbox.data, 0, 0);
        status->ams = (bool) can_readBytes(hvc_ams_imd_mailbox.data, 1, 1);
        hvc_ams_imd_mailbox.isRecent = false;
        status->isRecent = true;
    }
    if(hvc_pack_status_mailbox.isRecent) { // Continuous packet receive (every 3 ms)
        status->ok = true;
        status->pack_voltage = (float) can_readBytes(hvc_pack_status_mailbox.data, 0, 1) / 10.0f;
        status->pack_current = (float) can_readBytes(hvc_pack_status_mailbox.data, 2, 3) / 10.0f;
        status->stateOfCharge = (float) can_readBytes(hvc_pack_status_mailbox.data, 4, 5) / 10.0f;
        hvc_pack_status_mailbox.isRecent = false;
        status->isRecent = true;
    }
    if(hvc_contactor_status_mailbox.isRecent) { // Demand packet receive (when Contactor status changes)
        status->ok = true;
        status->contactor_status = (HvcStatus::ContactorStatus) hvc_contactor_status_mailbox.data[0];
        hvc_contactor_status_mailbox.isRecent = false;
        status->isRecent = true;
    }
}
