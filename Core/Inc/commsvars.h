//
// Created by yashk on 10/16/2023.
//

#ifndef VCU_FIRMWARE_2024_COMMSVARS_H
#define VCU_FIRMWARE_2024_COMMSVARS_H


#include "stm32h7xx_hal.h"

static ADC_HandleTypeDef hadc1;
static DMA_HandleTypeDef hdma_adc1;

static FDCAN_HandleTypeDef hfdcan1;

static SPI_HandleTypeDef hspi1;

static UART_HandleTypeDef huart3;

#endif //VCU_FIRMWARE_2024_COMMSVARS_H
