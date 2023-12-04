#ifndef VCU_FIRMWARE_2024_HVC_H
#define VCU_FIRMWARE_2024_HVC_H

#include <cstdint>

typedef struct HvcStatus {
    bool isRecent;
    bool ok;

    float pack_voltage;
    float pack_current;
    float stateOfCharge;
    float mean_pack_temp;
    float min_pack_temp;
    float max_pack_temp;

    bool imd;
    bool ams;

    enum ContactorStatus {
        UNKNOWN = 0,
        FULLY_OPEN = 1,
        PRECHARGE = 2,
        FULLY_CLOSED = 3,
        DISCHARGE = 4,
    } contactor_status;
    // ...
} HvcStatus;

/**
 * Initialize CAN communication with the HVC by subscribing to certain CAN IDs for example.
 */
void hvc_init();

/**
 * Send CAN packet to set cooling fan status and speed if needed.
 * @param battfan_turn_on true if cooling fan needs to turn on, false otherwise
 * @param battfan_rpm_req the desired RPM of the cooling fan cooling the battery segments
 * @param battfan_unique_rpm_req the desired RPM of the cooling fan cooling the unique segment of the battery
 * @return status of the CAN send, HAL_OK if successful
 */

uint32_t hvc_coolingRequest_send(bool battfan_turn_on, uint16_t battfan_rpm_req, uint16_t battfan_unique_rpm_req);

/**
 * Get latest HVC info NOT related to Voltage-sense and Temp-sense BMS sensors.
 * @param status HVCStatus struct to store the info in
 */
void hvc_getStatus(HvcStatus* status);

/**
 * Get latest voltage data from cells.
 */
void hvc_getVoltages();

/**
 * Get latest temperature data from cells.
 */
 void hvc_getTemps();


#endif //VCU_FIRMWARE_2024_HVC_H
