#ifndef VCU_FIRMWARE_2024_PDU_H
#define VCU_FIRMWARE_2024_PDU_H

#include <cstdint>
#include "VcuModel.h"

enum BuzzerType {
    BUZZER_SILENT = 0,
    BUZZER_BUZZ = 1,
    BUZZER_DEEP_IN_THE_HEART_OF_TEXAS = 2,
};

struct PDUStatus {
    float volumetricFlowRate;
    float waterTempInverter;
    float waterTempMotor;
    float waterTempRadiator;

    float radiatorFanRpm;

    float lvVoltage;
    float lvSoC;
    float lvCurrent;

    bool isRecent = false;
};

/**
 * Initialize the PDU's CAN mailboxes and wire signals
 */
void pdu_init();

static uint32_t pdu_sendBrakeLight(float brightnessLeft, float brightnessRight);

static uint32_t pdu_sendBuzzer(BuzzerType buzzerType);

static uint32_t pdu_sendCoolingOutput(uint16_t radiatorFanRpm, float pumpPercentage);

void pdu_periodic(PDUStatus* status, VcuOutput* vcuOutput, float deltaTime);


#endif //VCU_FIRMWARE_2024_PDU_H
