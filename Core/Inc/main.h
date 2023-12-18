/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BSPD_BrakePressed_Pin GPIO_PIN_13
#define BSPD_BrakePressed_GPIO_Port GPIOC
#define BSPD_MotorOn_Pin GPIO_PIN_14
#define BSPD_MotorOn_GPIO_Port GPIOC
#define BSPD_Shutdown_Pin GPIO_PIN_15
#define BSPD_Shutdown_GPIO_Port GPIOC
#define IN_Button_Pin GPIO_PIN_0
#define IN_Button_GPIO_Port GPIOC
#define SPI_MOSI_Pin GPIO_PIN_1
#define SPI_MOSI_GPIO_Port GPIOC
#define SPI_MISO_Pin GPIO_PIN_2
#define SPI_MISO_GPIO_Port GPIOC
#define BSPD_BrakeFailure_Pin GPIO_PIN_3
#define BSPD_BrakeFailure_GPIO_Port GPIOC
#define OUT_LEDB_Pin GPIO_PIN_0
#define OUT_LEDB_GPIO_Port GPIOA
#define OUT_LEDR_Pin GPIO_PIN_1
#define OUT_LEDR_GPIO_Port GPIOA
#define OUT_LEDG_Pin GPIO_PIN_2
#define OUT_LEDG_GPIO_Port GPIOA
#define ADC_Steer_Pin GPIO_PIN_3
#define ADC_Steer_GPIO_Port GPIOA
#define ADC_Sus1_Pin GPIO_PIN_4
#define ADC_Sus1_GPIO_Port GPIOA
#define OUT_ReadyToDrive_Pin GPIO_PIN_5
#define OUT_ReadyToDrive_GPIO_Port GPIOA
#define ADC_Sus2_Pin GPIO_PIN_6
#define ADC_Sus2_GPIO_Port GPIOA
#define ADC_APPS1_Pin GPIO_PIN_7
#define ADC_APPS1_GPIO_Port GPIOA
#define ADC_APPS2_Pin GPIO_PIN_4
#define ADC_APPS2_GPIO_Port GPIOC
#define ADC_BSE2_Pin GPIO_PIN_5
#define ADC_BSE2_GPIO_Port GPIOC
#define ADC_BSE1_Pin GPIO_PIN_0
#define ADC_BSE1_GPIO_Port GPIOB
#define IN_DriveSwitch_Pin GPIO_PIN_1
#define IN_DriveSwitch_GPIO_Port GPIOB
#define BSPD_MotorFailure_Pin GPIO_PIN_2
#define BSPD_MotorFailure_GPIO_Port GPIOB
#define SPI_SCK_Pin GPIO_PIN_10
#define SPI_SCK_GPIO_Port GPIOB
#define SPI_CS_IMU_Pin GPIO_PIN_12
#define SPI_CS_IMU_GPIO_Port GPIOB
#define CAN_Term_Pin GPIO_PIN_6
#define CAN_Term_GPIO_Port GPIOC
#define OUT_IMD_LED_Pin GPIO_PIN_7
#define OUT_IMD_LED_GPIO_Port GPIOC
#define CELL_UART_RX_Pin GPIO_PIN_8
#define CELL_UART_RX_GPIO_Port GPIOA
#define GPS_UART_TX_Pin GPIO_PIN_9
#define GPS_UART_TX_GPIO_Port GPIOA
#define GPS_UART_RX_Pin GPIO_PIN_10
#define GPS_UART_RX_GPIO_Port GPIOA
#define SPI_CS_EEPROM_Pin GPIO_PIN_11
#define SPI_CS_EEPROM_GPIO_Port GPIOA
#define OUT_AMS_LED_Pin GPIO_PIN_12
#define OUT_AMS_LED_GPIO_Port GPIOA
#define JTMS_Pin GPIO_PIN_13
#define JTMS_GPIO_Port GPIOA
#define JTCK_Pin GPIO_PIN_14
#define JTCK_GPIO_Port GPIOA
#define CELL_UART_TX_Pin GPIO_PIN_4
#define CELL_UART_TX_GPIO_Port GPIOB
#define CAN_RX_Pin GPIO_PIN_5
#define CAN_RX_GPIO_Port GPIOB
#define CAN_TX_Pin GPIO_PIN_6
#define CAN_TX_GPIO_Port GPIOB
#define NVM_DETECT_Pin GPIO_PIN_9
#define NVM_DETECT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
