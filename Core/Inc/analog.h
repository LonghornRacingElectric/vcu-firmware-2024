#ifndef VCU_FIRMWARE_2024_ANALOG_H
#define VCU_FIRMWARE_2024_ANALOG_H

typedef struct AnalogVoltages {
    float apps1, apps2;
    float bse1, bse2;
    float steer;
    float sus1, sus2;
} AnalogVoltages;

/**
 * Update the values in the struct with the latest voltages.
 * @param analogVoltages
 */
void analog_get(AnalogVoltages* analogVoltages);

#endif //VCU_FIRMWARE_2024_ANALOG_H
