#ifndef VCU_FIRMWARE_2024_NVM_H
#define VCU_FIRMWARE_2024_NVM_H

#include "VcuParameters.h"
#include "cellular.h"

void nvm_init();

/**
 * Load VCU parameters from a file on the SD card.
 * @param vcuParameters Pointer to VCU parameters.
 */
void nvm_loadParameters(VcuParameters* vcuParameters);

/**
 * Save VCU parameters to a file on the SD card.
 * @param vcuParameters Pointer to VCU parameters.
 */
void nvm_saveParameters(VcuParameters* vcuParameters);

/**
 * Create a new telemetry file with a unique name on the SD card.
 */
void nvm_beginTelemetry();

/**
 * Add a packet of data to the open file.
 * @param telemetry Pointer to telemetry data to be added.
 */
void nvm_addTelemetry();

/**
 * Save and close the file on the SD card.
 */
void nvm_endTelemetry();

#endif //VCU_FIRMWARE_2024_NVM_H
