#include "bspd.h"

void bspd_get(BSPD& bspd_state){
    bspd_state.brake_pressed = HAL_GPIO_ReadPin(BSPD_BrakePressed_GPIO_Port, BSPD_BrakePressed_Pin) == GPIO_PIN_SET;
    bspd_state.motor_on = HAL_GPIO_ReadPin(BSPD_MotorOn_GPIO_Port, BSPD_MotorOn_Pin) == GPIO_PIN_SET;
    bspd_state.shutdown = HAL_GPIO_ReadPin(BSPD_Shutdown_GPIO_Port, BSPD_Shutdown_Pin) == GPIO_PIN_SET;
    bspd_state.brake_failure = HAL_GPIO_ReadPin(BSPD_BrakeFailure_GPIO_Port, BSPD_BrakeFailure_Pin) == GPIO_PIN_SET;
    bspd_state.motor_failure = HAL_GPIO_ReadPin(BSPD_MotorFailure_GPIO_Port, BSPD_MotorFailure_Pin) == GPIO_PIN_SET;
}