#include "all_imus.h"
#include "angel_can_ids.h"
#include "angel_can.h"
#include "imu.h"

static CanInbox imu_hvcaccel_inbox;
static CanInbox imu_hvcgyro_inbox;
static CanInbox imu_pduaccel_inbox;
static CanInbox imu_pdugyro_inbox;
static CanInbox imu_unsSfr_inbox;
static CanInbox imu_unsSfl_inbox;
static CanInbox imu_unsSbr_inbox;
static CanInbox imu_unsSbl_inbox;

void allImus_init(SPI_HandleTypeDef *hspi) {
    imu_init(hspi);
    can_addInbox(HVC_VCU_IMU_ACCEL, &imu_hvcaccel_inbox);
    can_addInbox(HVC_VCU_IMU_GYRO, &imu_hvcgyro_inbox);
    can_addInbox(PDU_VCU_IMU_ACCEL, &imu_pduaccel_inbox);
    can_addInbox(PDU_VCU_IMU_GYRO, &imu_pdugyro_inbox);
    can_addInbox(UNSFR_VCU_IMU, &imu_unsSfr_inbox);
    can_addInbox(UNSFL_VCU_IMU, &imu_unsSfl_inbox);
    can_addInbox(UNSBR_VCU_IMU, &imu_unsSbr_inbox);
    can_addInbox(UNSBL_VCU_IMU, &imu_unsSbl_inbox);
}

static void externalImus_getAccels(xyz* accelHvc, xyz* accelPdu, xyz* accelFl, xyz* accelFr, xyz* accelBl, xyz* accelBr) {
  if(imu_hvcaccel_inbox.isRecent) {
    accelHvc->x = (float) can_readBytes(imu_hvcaccel_inbox.data, 0, 1) / 100.0f;
    accelHvc->y = (float) can_readBytes(imu_hvcaccel_inbox.data, 2, 3) / 100.0f;
    accelHvc->z = (float) can_readBytes(imu_hvcaccel_inbox.data, 4, 5) / 100.0f;
    imu_hvcaccel_inbox.isRecent = false;
  }
  if(imu_pduaccel_inbox.isRecent) {
    accelPdu->x = (float) can_readBytes(imu_pduaccel_inbox.data, 0, 1) / 100.0f;
    accelPdu->y = (float) can_readBytes(imu_pduaccel_inbox.data, 2, 3) / 100.0f;
    accelPdu->z = (float) can_readBytes(imu_pduaccel_inbox.data, 4, 5) / 100.0f;
    imu_pduaccel_inbox.isRecent = false;
  }
  if(imu_unsSfl_inbox.isRecent) {
    accelFl->x = (float) can_readBytes(imu_unsSfl_inbox.data, 0, 1) / 100.0f;
    accelFl->y = (float) can_readBytes(imu_unsSfl_inbox.data, 2, 3) / 100.0f;
    accelFl->z = (float) can_readBytes(imu_unsSfl_inbox.data, 4, 5) / 100.0f;
    imu_unsSfl_inbox.isRecent = false;
  }
  if(imu_unsSfr_inbox.isRecent) {
    accelFr->x = (float) can_readBytes(imu_unsSfr_inbox.data, 0, 1) / 100.0f;
    accelFr->y = (float) can_readBytes(imu_unsSfr_inbox.data, 2, 3) / 100.0f;
    accelFr->z = (float) can_readBytes(imu_unsSfr_inbox.data, 4, 5) / 100.0f;
    imu_unsSfr_inbox.isRecent = false;
  }
  if(imu_unsSbl_inbox.isRecent) {
    accelBl->x = (float) can_readBytes(imu_unsSbl_inbox.data, 0, 1) / 100.0f;
    accelBl->y = (float) can_readBytes(imu_unsSbl_inbox.data, 2, 3) / 100.0f;
    accelBl->z = (float) can_readBytes(imu_unsSbl_inbox.data, 4, 5) / 100.0f;
    imu_unsSbl_inbox.isRecent = false;
  }
  if(imu_unsSbr_inbox.isRecent) {
    accelBr->x = (float) can_readBytes(imu_unsSbr_inbox.data, 0, 1) / 100.0f;
    accelBr->y = (float) can_readBytes(imu_unsSbr_inbox.data, 2, 3) / 100.0f;
    accelBr->z = (float) can_readBytes(imu_unsSbr_inbox.data, 4, 5) / 100.0f;
    imu_unsSbr_inbox.isRecent = false;
  }

}

static void externalImus_getGyros(xyz* gyroHvc, xyz* gyroPdu) {
  if(imu_hvcgyro_inbox.isRecent) {
    gyroHvc->x = (float) can_readBytes(imu_hvcgyro_inbox.data, 0, 1) / 100.0f;
    gyroHvc->y = (float) can_readBytes(imu_hvcgyro_inbox.data, 2, 3) / 100.0f;
    gyroHvc->z = (float) can_readBytes(imu_hvcgyro_inbox.data, 4, 5) / 100.0f;
    imu_hvcgyro_inbox.isRecent = false;
  }
  if(imu_pdugyro_inbox.isRecent) {
    gyroPdu->x = (float) can_readBytes(imu_pdugyro_inbox.data, 0, 1) / 100.0f;
    gyroPdu->y = (float) can_readBytes(imu_pdugyro_inbox.data, 2, 3) / 100.0f;
    gyroPdu->z = (float) can_readBytes(imu_pdugyro_inbox.data, 4, 5) / 100.0f;
    imu_pdugyro_inbox.isRecent = false;
  }
}

void allImus_periodic(ImuData *imuData) {

  if (imu_isAccelReady()) imu_getAccel(&imuData->accelVcu);
  if (imu_isGyroReady()) imu_getGyro(&imuData->gyroVcu);

  externalImus_getAccels(&imuData->accelHvc, &imuData->accelPdu, &imuData->accelFl, &imuData->accelFr, &imuData->accelBl, &imuData->accelBr);
  externalImus_getGyros(&imuData->gyroHvc, &imuData->gyroPdu);
}