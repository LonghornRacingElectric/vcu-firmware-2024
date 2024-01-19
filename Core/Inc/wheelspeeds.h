#ifndef VCU_FIRMWARE_2024_WHEELSPEEDS_H
#define VCU_FIRMWARE_2024_WHEELSPEEDS_H

typedef struct WheelDisplacements { // radians
    float fl;
    float fr;
    float bl;
    float br;
} WheelDisplacements;

/**
 * Subscribe to CAN messages.
 */
void wheelspeeds_init();

/**
 * Get latest wheel displacements from CAN.
 */
void wheelspeeds_get(WheelDisplacements* wheelDisplacements);

#endif //VCU_FIRMWARE_2024_WHEELSPEEDS_H
