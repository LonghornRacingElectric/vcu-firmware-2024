#include "analog.h"

static uint32_t adcValues[NUM_SENSORS * NUM_VALUES_PER_SENSOR];

int analog_calibrate(ADC_HandleTypeDef* hadc){
    return HAL_ADCEx_Calibration_Start(hadc, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
}

int analog_start(ADC_HandleTypeDef* hadc){
    int error = analog_calibrate(hadc);
    if(error != 0) return error;
    error = HAL_ADCEx_MultiModeConfigChannel(hadc, ADC_MODE_INDEPENDENT);
    if(error != 0) return error;
    return HAL_ADCEx_MultiModeStart_DMA(hadc, adcValues, NUM_SENSORS * NUM_VALUES_PER_SENSOR);
}

int analog_stop(ADC_HandleTypeDef* hadc){
    return HAL_ADCEx_MultiModeStop_DMA(hadc);
}

static float analog_convert(uint32_t adc_value){
    if(adc_value >= (1 << 16)) return 0;
    return ((float) adc_value) * 3.3f / (1 << 16);
}

void analog_get_recent(AnalogVoltages& analogVoltages) {
    analogVoltages.apps1 = analog_convert(adcValues[APPS1]);
    analogVoltages.apps2 = analog_convert(adcValues[APPS2]);
    analogVoltages.bse1 = analog_convert(adcValues[BSE1]);
    analogVoltages.bse2 = analog_convert(adcValues[BSE2]);
    analogVoltages.steer = analog_convert(adcValues[STEER]);
    analogVoltages.sus1 = analog_convert(adcValues[SUS1]);
    analogVoltages.sus2 = analog_convert(adcValues[SUS2]);

}

void analog_get_average(AnalogVoltages& analogVoltages){
    uint32_t sum_apps1 = 0;
    uint32_t sum_apps2 = 0;
    uint32_t sum_bse1 = 0;
    uint32_t sum_bse2 = 0;
    uint32_t sum_steer = 0;
    uint32_t sum_sus1 = 0;
    uint32_t sum_sus2 = 0;
    for(int i = 0; i < NUM_VALUES_PER_SENSOR; i++){
        sum_apps1 += adcValues[APPS1 + i * NUM_SENSORS];
        sum_apps2 += adcValues[APPS2 + i * NUM_SENSORS];
        sum_bse1 += adcValues[BSE1 + i * NUM_SENSORS];
        sum_bse2 += adcValues[BSE2 + i * NUM_SENSORS];
        sum_steer += adcValues[STEER + i * NUM_SENSORS];
        sum_sus1 += adcValues[SUS1 + i * NUM_SENSORS];
        sum_sus2 += adcValues[SUS2 + i * NUM_SENSORS];
    }
    analogVoltages.apps1 = analog_convert(sum_apps1 / NUM_VALUES_PER_SENSOR);
    analogVoltages.apps2 = analog_convert(sum_apps2 / NUM_VALUES_PER_SENSOR);
    analogVoltages.bse1 = analog_convert(sum_bse1 / NUM_VALUES_PER_SENSOR);
    analogVoltages.bse2 = analog_convert(sum_bse2 / NUM_VALUES_PER_SENSOR);
    analogVoltages.steer = analog_convert(sum_steer / NUM_VALUES_PER_SENSOR);
    analogVoltages.sus1 = analog_convert(sum_sus1 / NUM_VALUES_PER_SENSOR);
    analogVoltages.sus2 = analog_convert(sum_sus2 / NUM_VALUES_PER_SENSOR);

}
