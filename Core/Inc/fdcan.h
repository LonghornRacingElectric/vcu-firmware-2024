/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.h
  * @brief   This file contains all the function prototypes for
  *          the fdcan.c file
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
#ifndef __FDCAN_H__
#define __FDCAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern FDCAN_HandleTypeDef hfdcan2;
extern FDCAN_TxHeaderTypeDef TxHeader;
extern FDCAN_RxHeaderTypeDef RxHeader;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_FDCAN2_Init(void);

/* USER CODE BEGIN Prototypes */
/**
 * @brief Send CAN message with given ID, DLC, and data
 * @param id
 * MUST BE A 11-BIT ID, WITH DEFINITION (e.g. VCU_INV_PARAMS_REQUEST)
 * @param dlc
 * MUST BE IN USING "FDCAN_DATA_BYTES_<insert size here>"
 * @param data
 * MUST BE EXACTLY THE SIZE OF DLC
 * @return error code if CAN fails to send, 0 otherwise
 */
unsigned int fdcan_send(uint32_t id, uint32_t dlc, uint8_t* data, uint32_t fault);

/**
 * @brief Update CAN mailboxes with the data in CAN FIFO
 * @param fault
 * FAULT TO SET IF CAN FAILS TO READ
 * @return error code if CAN fails to send, 0 otherwise
 */
unsigned int fdcan_processRxFifo(uint32_t fault);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_H__ */

