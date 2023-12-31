#ifndef VCU_FIRMWARE_2024_EXTERNAL_IMUS_H
#define VCU_FIRMWARE_2024_EXTERNAL_IMUS_H

#include "imu.h"

/**
 * Initialize by subscribing to certain CAN IDs for example.
 */
void externalImus_init();

/**
 * Put latest acceleration vectors in corresponding pointers (2 from body, 4 from wheels).
 */
void externalImus_getAccels(xyz* accel1, xyz* accel2, xyz* accelFl, xyz* accelFr, xyz* accelBl, xyz* accelBr);

/**
 * Put latest gyro vectors in corresponding pointers (2 from body).
 */
void externalImus_getGyros(xyz* gyro1, xyz* gyro2);

#endif //VCU_FIRMWARE_2024_EXTERNAL_IMUS_H
