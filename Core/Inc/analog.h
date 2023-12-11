#ifndef VCU_FIRMWARE_2024_ANALOG_H
#define VCU_FIRMWARE_2024_ANALOG_H

#include "stm32h7xx_hal.h"
#define NUM_SENSORS 7
#define NUM_VALUES_PER_SENSOR 32
#define APPS1 0
#define APPS2 1
#define BSE1 2
#define BSE2 3
#define STEER 4
#define SUS1 5
#define SUS2 6

typedef struct AnalogVoltages {
    float apps1, apps2;
    float bse1, bse2;
    float steer;
    float sus1, sus2;
} AnalogVoltages;

/**
 * Calibrate the offset for the analog sensors.
 * @param hadc1
 * @return error code
 */
int analog_calibrate(ADC_HandleTypeDef* hadc1);

/**
 * Initialize the ADC and DMA for analog sensors.
 * @param hadc1
 * @return error code
 */
int analog_start(ADC_HandleTypeDef* hadc1);

/**
 * Stop the ADC and DMA for analog sensors.
 * @param hadc1
 * @return error code
 */
int analog_stop(ADC_HandleTypeDef* hadc1);

/**
 * Update the values in the struct with the latest voltages.
 * @param analogVoltages
 */
void analog_get_recent(AnalogVoltages& analogVoltages);

/**
 * Update the values in the struct with the average of the last polled voltages.
 * @param analogVoltages
 */
void analog_get_average(AnalogVoltages& analogVoltages);

#endif //VCU_FIRMWARE_2024_ANALOG_H
