#ifndef VCU_FIRMWARE_2024_INVERTER_H
#define VCU_FIRMWARE_2024_INVERTER_H

// TODO wip
#include "stm32h7xx_hal.h"
#include <string>

typedef struct InverterStatus {
    bool isRecent;
    float voltage; //input voltage into DC side
    float current; //input current into DC side
    float rpm;
    float inverterTemp;
    float motorTemp;

    float motorAngle;
    float resolverAngle;

    float phaseACurrent;
    float phaseBCurrent;
    float phaseCCurrent;

    float BCVoltage;
    float ABVoltage;
    float outputVoltage;

    float inverterFrequency;

    float torqueActual;
    float torqueCommand;

    uint64_t faultVector;
    uint64_t stateVector;
    // ...
} InverterStatus;

/**
 * Initialize inverter CAN communication.
 */
void inverter_init();

/**
 * Get latest inverter info.
 */
void inverter_getStatus(InverterStatus* status);

/**
 * Send a torque command over CAN.
 */
unsigned int inverter_sendTorqueCommand(float torque, float rpm, bool enable_inverter);

/**
 * Reset faults over CAN.
 * @return error_code
 */
unsigned int inverter_resetFaults();

/**
 * Read/write inverter parameters over CAN.
 * @param param_addr
 * @param param_value
 * @param write
 * @return error_code
 */
unsigned int inverter_paramsIO(uint16_t param_addr, uint16_t param_value, bool write);

#endif //VCU_FIRMWARE_2024_INVERTER_H
