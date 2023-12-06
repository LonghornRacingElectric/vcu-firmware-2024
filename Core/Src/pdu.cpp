#include "pdu.h"
#include "angel_can.h"
#include "faults.h"
#include <algorithm>

static CanRx tsal_status_mailbox;
static CanRx ms_status_mailbox;
static CanRx lvbatt_status_mailbox;
static CanRx water_status_mailbox;
static CanRx air_status_mailbox;

void pdu_init(){
//    can_addMailbox(PDU_VCU_TSAL_STATUS, &tsal_status_mailbox);
//    can_addMailbox(PDU_VCU_MS_STATUS, &ms_status_mailbox);
    can_addMailbox(PDU_LVBATT_INFO, &lvbatt_status_mailbox);
    can_addMailbox(PDU_WATERCOOL_INFO, &water_status_mailbox);
    can_addMailbox(PDU_FANCOOL_INFO, &air_status_mailbox);
}

uint32_t pdu_brakeLight_send(float bse1, float bse2, float threshold, bool bspd_brake_pressed, bool is_night) {

    float brightness_left = 0, brightness_right = 0;
    if(bse1 > threshold || bse2 > threshold){
        brightness_left = bse1 / 3.3f * 100;
        brightness_right = bse2 / 3.3f * 100;
    }
    else if(bspd_brake_pressed){
        brightness_left = 50;
        brightness_right = 50;
    }

    uint8_t data[3];
    data[0] = (uint8_t) brightness_left; // range from 0 to 100
    data[1] = (uint8_t) brightness_right; // rang from 0 to 100
    data[2] = (uint8_t) is_night;

    return can_send(VCU_PDU_BRAKELIGHT, 3, data);
}

uint32_t pdu_driveBuzzer_send(BuzzerType turn_on) {
    uint8_t data[2];
    data[0] = (uint8_t) turn_on;

    return can_send(VCU_PDU_HORN, 2, data);
}

uint32_t pdu_coolingRequest_send(uint16_t radiator_fan_rpm, float pump_percentage) {
    uint8_t data[4];
    if(pump_percentage < 0.0f || pump_percentage > 100.0f){
        pump_percentage = 0.0f;
    }
    can_writeBytes(data, 0, 1, radiator_fan_rpm); // max will occur at 8300 rpm, but higher will just be ignored
    can_writeBytes(data, 2, 3, (uint8_t) pump_percentage );

    return can_send(VCU_PDU_COOLING, 4, data);
}

uint32_t pdu_calcRemainingLVBattTime(float lv_soc, float lv_capacity, float lv_current){
    auto time = lv_soc * lv_capacity / std::min(lv_current, 0.5f); // in hours
    return (uint32_t) (time * 60); // in minutes
}

void pdu_getStatus(PDUStatus& status){
//    if(tsal_status_mailbox.isRecent){
//        status.tsal_status = (PDUStatus::TSALStatus) can_readBytes(tsal_status_mailbox.data, 0, 0);
//        tsal_status_mailbox.isRecent = false;
//        status.isRecent = true;
//    }
//    if(ms_status_mailbox.isRecent){
//        status.master_switch_status = (PDUStatus::MasterSwitchStatus) can_readBytes(ms_status_mailbox.data, 0, 0);
//        ms_status_mailbox.isRecent = false;
//        status.isRecent = true;
//    }
    if(lvbatt_status_mailbox.isRecent){
        status.lv_voltage = (float) can_readBytes(lvbatt_status_mailbox.data, 0, 1) / 10.0f; // Range is 0.0 - 28.0V
        status.lv_soc = (float) can_readBytes(lvbatt_status_mailbox.data, 2, 3) / 10.0f;
        status.lv_current = (float) can_readBytes(lvbatt_status_mailbox.data, 4, 5) / 100.0f; // Range is 0.00 - 100.00A
        status.lv_capacity = (float) can_readBytes(lvbatt_status_mailbox.data, 6, 7) / 10.0f; // Range is 0.0 - 28.0Ah
        lvbatt_status_mailbox.isRecent = false;
        status.isRecent = true;
    }
    if(water_status_mailbox.isRecent){
        status.volumetric_flow_rate = (float) can_readBytes(water_status_mailbox.data, 0, 0) / 10.0f; // range from 0.0 to 25.5 L/min
        status.cooling_pump_on = (bool) can_readBytes(water_status_mailbox.data, 1, 1);
        status.water_temp_motor = (float) can_readBytes(water_status_mailbox.data, 2, 3) / 10.0f;
        status.water_temp_inverter = (float) can_readBytes(water_status_mailbox.data, 4, 5) / 10.0f;
        status.water_temp_radiator = (float) can_readBytes(water_status_mailbox.data, 6, 7) / 10.0f;
        water_status_mailbox.isRecent = false;
        status.isRecent = true;
    }
    if(air_status_mailbox.isRecent){
        status.radiator_fan_rpm_1 = (float) can_readBytes(air_status_mailbox.data, 0, 1) / 10.0f;
        status.radiator_fan_rpm_2 = (float) can_readBytes(air_status_mailbox.data, 2, 3) / 10.0f;
        status.radiator_fan_rpm_3 = (float) can_readBytes(air_status_mailbox.data, 4, 5) / 10.0f;
        status.radiator_fans_on = (bool) can_readBytes(air_status_mailbox.data, 6, 6);
        air_status_mailbox.isRecent = false;
        status.isRecent = true;
    }
}