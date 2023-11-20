#ifndef VCU_FIRMWARE_2024_HVC_H
#define VCU_FIRMWARE_2024_HVC_H

typedef struct HvcStatus {
    bool isRecent;
    bool ok;
    float voltage;
    float current;
    float batteryTemp;
    bool imd;
    bool ams;
    // ...
} HvcStatus;

/**
 * Initialize CAN communication with the HVC by subscribing to certain CAN IDs for example.
 */
void hvc_init();

/**
 * Get latest HVC info.
 */
void hvc_getStatus(HvcStatus* status);


#endif //VCU_FIRMWARE_2024_HVC_H
