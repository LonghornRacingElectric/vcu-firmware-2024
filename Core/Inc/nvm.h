#ifndef VCU_FIRMWARE_2024_NVM_H
#define VCU_FIRMWARE_2024_NVM_H

#include "VcuParameters.h"
#include "cellular.h"

#define NVM_TELEMETRY_SAMPLE_PERIOD_MS 0U
#define NVM_TELEMETRY_WRITE_BUFFER_SIZE 8192U
#define NVM_TELEMETRY_WRITE_THRESHOLD_BYTES 4096U
#define NVM_TELEMETRY_SYNC_INTERVAL_MS 100U

typedef enum {
  NVM_TELEMETRY_WAITING_FOR_GPS = 0,
  NVM_TELEMETRY_LOGGING = 1,
  NVM_TELEMETRY_FAILED = 2,
} NvmTelemetryStatus;

/**
 * Load VCU parameters from a file on the SD card.
 * @param vcuParameters Pointer to VCU parameters.
 */
static void nvm_loadParameters(VcuParameters* vcuParameters);

/**
 * Save VCU parameters to a file on the SD card.
 * @param vcuParameters Pointer to VCU parameters.
 */
static void nvm_saveParameters(VcuParameters* vcuParameters);

/**
 * Create a new binary telemetry file on the SD card with a unique name based on the GPS time.
 */
static bool nvm_beginTelemetry();

/**
 * Capture one telemetry sample and append it to the open binary log.
 */
static void nvm_writeTelemetry(VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus, PduStatus *pduStatus, InverterStatus *inverterStatus, AnalogVoltages *analogVoltages, WheelMagnetValues *wheelMagnetValues, ImuData *imuData, GpsData *gpsData);

/**
 * Called once on startup. Load VCU parameters from SD card if they exist.
 * Also open a telemetry binary file named based on the current timestamp from the GPS real-time clock.
 */
void nvm_init(VcuParameters *vcuParameters);

/**
 * Called many times per second. Save the VCU Parameters if they've changed and
 * write one binary telemetry record.
 */
void nvm_periodic(VcuParameters* vcuParameters, VcuOutput *vcuCoreOutput,
                  HvcStatus *hvcStatus, PduStatus *pduStatus, InverterStatus *inverterStatus,
                  AnalogVoltages *analogVoltages, WheelMagnetValues *wheelMagnetValues,
                  ImuData *imuData, GpsData *gpsData);

NvmTelemetryStatus nvm_getTelemetryStatus(void);

#endif //VCU_FIRMWARE_2024_NVM_H
