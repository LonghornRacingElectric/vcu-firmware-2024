#ifndef VCU_FIRMWARE_2024_BSPD_H
#define VCU_FIRMWARE_2024_BSPD_H

#include "stm32h7xx_hal.h"

#define BSPD_BrakePressed_Pin GPIO_PIN_13
#define BSPD_BrakePressed_GPIO_Port GPIOC
#define BSPD_MotorOn_Pin GPIO_PIN_14
#define BSPD_MotorOn_GPIO_Port GPIOC
#define BSPD_Shutdown_Pin GPIO_PIN_15
#define BSPD_Shutdown_GPIO_Port GPIOC
#define BSPD_BrakeFailure_Pin GPIO_PIN_3
#define BSPD_BrakeFailure_GPIO_Port GPIOC
#define BSPD_MotorFailure_Pin GPIO_PIN_2
#define BSPD_MotorFailure_GPIO_Port GPIOB


struct BSPD {
    bool brake_pressed;
    bool motor_on;
    bool shutdown;
    bool brake_failure;
    bool motor_failure;
};

/**
 * Get the state of the BSPD to be sent to telemetry.
 * @param bspd_state
 */
void bspd_get(BSPD& bspd_state);

#endif //VCU_FIRMWARE_2024_BSPD_H
