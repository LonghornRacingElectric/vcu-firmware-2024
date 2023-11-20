#ifndef VCU_FIRMWARE_2024_DASH_H
#define VCU_FIRMWARE_2024_DASH_H

typedef struct DashInfo {
    float speed;
    float hvSoC;
    float lvSoC;
    bool ams;
    bool imd;
} DashInfo;

/**
 * Send a CAN packet (or two) to the dash with everything it needs to know.
 */
void dash_update(DashInfo* dashInfo);

#endif //VCU_FIRMWARE_2024_DASH_H
