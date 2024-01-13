#include "indicators.h"
#include "main.h"

void indicators_periodic(HvcStatus *hvcStatus, VcuOutput *vcuCoreOutput) {
  HAL_GPIO_WritePin(OUT_AMS_LED_GPIO_Port, OUT_AMS_LED_Pin, (GPIO_PinState) hvcStatus->ams);
  HAL_GPIO_WritePin(OUT_IMD_LED_GPIO_Port, OUT_AMS_LED_Pin, (GPIO_PinState) hvcStatus->imd);
  HAL_GPIO_WritePin(OUT_ReadyToDrive_GPIO_Port, OUT_ReadyToDrive_Pin, (GPIO_PinState) vcuCoreOutput->r2dBuzzer);
}