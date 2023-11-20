#ifndef VCU_FIRMWARE_2024_THERMAL_H
#define VCU_FIRMWARE_2024_THERMAL_H

typedef struct Temperatures {
    float motorTemp;
    float inverterTemp;
    float coolantTemp1;
    float coolantTemp2;
    // TODO figure out what temp sensors we're working with
} Temperatures;

/**
 * Subscribe to CAN messages.
 */
void thermal_init();

/**
 * Updates struct with latest temperatures from CAN.
 */
void thermal_getTemps(Temperatures* temps);

/**
 * Send CAN message with cooling outputs.
 */
void thermal_setOutputs(float radiator, float pump, float batteryFans);

#endif //VCU_FIRMWARE_2024_THERMAL_H
