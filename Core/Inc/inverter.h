#ifndef VCU_FIRMWARE_2024_INVERTER_H
#define VCU_FIRMWARE_2024_INVERTER_H

// TODO wip

typedef struct InverterStatus {
    bool isRecent;
    float voltage;
    float current;
    float rpm;
    float inverterTemp;
    float motorTemp;
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
void inverter_sendTorqueCommand(float torque);

/**
 * Reset faults over CAN.
 */
void inverter_resetFaults();

#endif //VCU_FIRMWARE_2024_INVERTER_H
