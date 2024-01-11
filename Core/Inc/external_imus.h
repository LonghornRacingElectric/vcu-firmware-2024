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
void externalImus_getAccels(xyz1* accel1, xyz1* accel2, xyz1* accelFl, xyz1* accelFr, xyz1* accelBl, xyz1* accelBr);

/**
 * Put latest gyro vectors in corresponding pointers (2 from body).
 */
void externalImus_getGyros(xyz1* gyro1, xyz1* gyro2);

#endif //VCU_FIRMWARE_2024_EXTERNAL_IMUS_H
