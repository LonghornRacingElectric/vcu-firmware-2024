#ifndef VCU_FIRMWARE_2024_PDU_H
#define VCU_FIRMWARE_2024_PDU_H

#include <cstdint>

struct PDUStatus {
    // Please help me fill this out I have no damn clue
    enum TSALStatus {
        OFF = 0,
        GREEN = 1,
        RED = 2
    } tsal_status;

    float volumetric_flow_rate;
    float water_temp;
    bool cooling_pump_on;
    float radiator_fan_rpm_1;
    float radiator_fan_rpm_2;
    float radiator_fan_rpm_3;
    bool radiator_fans_on;

    enum MasterSwitchStatus {
        NV = 0,
        LV = 1,
        HV = 2
    } master_switch_status;

    float lv_voltage;
    float lv_soc;
    float lv_current;
    float lv_capacity;

    bool isRecent = false;

};

/**
 * Initialize the PDU's CAN mailboxes and wire signals
 */
void pdu_init();

/**
 * Send CAN packet to set brake light status and brightness if needed.
 * @param bse1 Brake sensor 1 value in volts
 * @param bse2 Brake sensor 2 value in volts
 * @param threshold Threshold for brake sensor values to activate brake light
 * @param bspd_brake_pressed true if the brake pedal is pressed, false otherwise. Derived from BSPD
 * @param is_night true if it is night time, false otherwise. This will be dim the lights if true
 * @return status of the CAN send, HAL_OK if successful
 */
uint32_t pdu_brakeLight_send(float bse1, float bse2, float threshold, bool bspd_brake_pressed, bool is_night);

/**
 * Send CAN packet to set buzzer status and time if needed.
 * @param turn_on true if buzzer needs to turn on, false otherwise
 * @param time how long the buzzer should last in ms
 * @return status of the CAN send, HAL_OK if successful
 */
uint32_t pdu_driveBuzzer_send(bool turn_on, uint8_t time_buzzed);

/**
 * Send CAN packet to request cooling parameters, namely radiator fan RPM and water pump volumetric flow rate
 * @param radiator_turn_on true if radiator fans needs to turn on, false otherwise
 * @param radiator_fan_rpm_req speed of fans in RPM
 * @param water_cooling_turn_on true if water cooling pump needs to turn on, false otherwise
 * @param water_pump_vfr_req push rate of pump in RPM
 * @return status of the CAN send, HAL_OK if successful
 */
uint32_t pdu_coolingRequest_send(bool radiator_turn_on, uint16_t radiator_fan_rpm_req, bool water_cooling_turn_on, uint16_t water_pump_vfr_req);

/**
 * Get the status of the PDU and update the corresponding struct
 * @param status PDUStatus struct to store the status in
 */
void pdu_getStatus(PDUStatus& status);

/**
 * Calculate the remaining time of the LV battery based on the state of charge, capacity, and current
 * Note: this is not the most accurate way to calculate remaining time, but it is the easiest
 * @param lv_soc
 * @param lv_capacity
 * @param lv_current
 * @return remaining time in minutes
 */
uint32_t pdu_calcRemainingLVBattTime(float lv_soc, float lv_capacity, float lv_current);


#endif //VCU_FIRMWARE_2024_PDU_H
