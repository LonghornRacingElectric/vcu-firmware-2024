#ifndef VCU_FIRMWARE_2024_HVC_H
#define VCU_FIRMWARE_2024_HVC_H

#include <cstdint>
#include "VcuModel.h"

typedef struct HvcStatus {
    bool isRecent;
    bool ok;

    float packVoltage;
    float packCurrent;
    float stateOfCharge;
    float packTempMean;
    float packTempMin;
    float packTempMax;

    bool imd;
    bool ams;

    enum ContactorStatus {
        UNKNOWN = 0,
        FULLY_OPEN = 1,
        PRECHARGE = 2,
        FULLY_CLOSED = 3,
        DISCHARGE = 4,
    } contactorStatus;
    // ...
} HvcStatus;

/**
 * Initialize CAN communication with the HVC by subscribing to certain CAN IDs for example.
 */
void hvc_init();

/**
 * Send CAN packet to set cooling fan status and speed if needed.
 * @param battFanRpm the desired RPM of the cooling fan cooling the battery segments
 * @param battUniqueSegRpm the desired RPM of the cooling fan cooling the unique segment of the battery
 * @return status of the CAN send, HAL_OK if successful
 */
static uint32_t hvc_setCoolingOutput(uint16_t battFanRpm, uint16_t battUniqueSegRpm);

/**
 * Checks CAN mailboxes and updates status accordingly. Sends CAN messages when it's time.
 */
void hvc_periodic(HvcStatus* status, VcuOutput* vcuOutput, float deltaTime);


#endif //VCU_FIRMWARE_2024_HVC_H
