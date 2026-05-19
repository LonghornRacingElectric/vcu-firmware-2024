/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
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
#include "fatfs.h"
#include "gps_time.h"

uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  if (gpsFatTimestamp.year == 0 || gpsFatTimestamp.month == 0 || gpsFatTimestamp.day == 0) {
    return ((DWORD)(_NORTC_YEAR - 1980U) << 25)
        | ((DWORD)_NORTC_MON << 21)
        | ((DWORD)_NORTC_MDAY << 16);
  }

  const DWORD fullYear = 2000U + (DWORD) gpsFatTimestamp.year;
  const DWORD clampedMonth = (gpsFatTimestamp.month >= 1U && gpsFatTimestamp.month <= 12U) ? gpsFatTimestamp.month : 1U;
  const DWORD clampedDay = (gpsFatTimestamp.day >= 1U && gpsFatTimestamp.day <= 31U) ? gpsFatTimestamp.day : 1U;
  const DWORD clampedHour = (gpsFatTimestamp.hour <= 23U) ? gpsFatTimestamp.hour : 0U;
  const DWORD clampedMinute = (gpsFatTimestamp.minute <= 59U) ? gpsFatTimestamp.minute : 0U;
  const DWORD clampedSecond = (gpsFatTimestamp.seconds <= 59U) ? gpsFatTimestamp.seconds : 0U;
  const DWORD fatYear = (fullYear >= 1980U) ? (fullYear - 1980U) : 0U;

  return (fatYear << 25)
      | (clampedMonth << 21)
      | (clampedDay << 16)
      | (clampedHour << 11)
      | (clampedMinute << 5)
      | (clampedSecond / 2U);
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */
