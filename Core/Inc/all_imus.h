#ifndef VCU_FIRMWARE_2024_ALL_IMUS_H
#define VCU_FIRMWARE_2024_ALL_IMUS_H

#include "imu.h"

typedef struct ImuData {
  xyz accel1;
  xyz accel2;
  xyz accel3;
  xyz accelFl;
  xyz accelFr;
  xyz accelBl;
  xyz accelBr;
  xyz gyro1;
  xyz gyro2;
  xyz gyro3;
} ImuData;

/**
 * Initialize by subscribing to certain CAN IDs for example.
 */
void allImus_init();

/**
 * Get latest IMU data from both local and external IMUs.
 */
void allImus_periodic(ImuData *imuData);

#endif //VCU_FIRMWARE_2024_ALL_IMUS_H
