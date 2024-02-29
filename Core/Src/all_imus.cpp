#include "all_imus.h"
#include "angel_can_ids.h"
#include "angel_can.h"

static CanInbox imu_hvcaccel_inbox;
static CanInbox imu_hvcgyro_inbox;
static CanInbox imu_pduaccel_inbox;
static CanInbox imu_pdugyro_inbox;
static CanInbox imu_unsSfr_inbox;
static CanInbox imu_unsSfl_inbox;
static CanInbox imu_unsSbr_inbox;
static CanInbox imu_unsSbl_inbox;

void allImus_init() {
    imu_init();
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
    accelHvc->x = can_readFloat(int16_t, &imu_hvcaccel_inbox, 0, 0.01f);
    accelHvc->y = can_readFloat(int16_t, &imu_hvcaccel_inbox, 2, 0.01f);
    accelHvc->z = can_readFloat(int16_t, &imu_hvcaccel_inbox, 4, 0.01f);
    imu_hvcaccel_inbox.isRecent = false;
  }
  if(imu_pduaccel_inbox.isRecent) {
    accelPdu->x = can_readFloat(int16_t, &imu_pduaccel_inbox, 0, 0.01f);
    accelPdu->y = can_readFloat(int16_t, &imu_pduaccel_inbox, 2, 0.01f);
    accelPdu->z = can_readFloat(int16_t, &imu_pduaccel_inbox, 4, 0.01f);
    imu_pduaccel_inbox.isRecent = false;
  }
  if(imu_unsSfl_inbox.isRecent) {
    accelFl->x = can_readFloat(int16_t, &imu_unsSfl_inbox, 0, 0.01f);
    accelFl->y = can_readFloat(int16_t, &imu_unsSfl_inbox, 2, 0.01f);
    accelFl->z = can_readFloat(int16_t, &imu_unsSfl_inbox, 4, 0.01f);
    imu_unsSfl_inbox.isRecent = false;
  }
  if(imu_unsSfr_inbox.isRecent) {
    accelFr->x = can_readFloat(int16_t, &imu_unsSfr_inbox, 0, 0.01f);
    accelFr->y = can_readFloat(int16_t, &imu_unsSfr_inbox, 2, 0.01f);
    accelFr->z = can_readFloat(int16_t, &imu_unsSfr_inbox, 4, 0.01f);
    imu_unsSfr_inbox.isRecent = false;
  }
  if(imu_unsSbl_inbox.isRecent) {
    accelBl->x = can_readFloat(int16_t, &imu_unsSbl_inbox, 0, 0.01f);
    accelBl->y = can_readFloat(int16_t, &imu_unsSbl_inbox, 2, 0.01f);
    accelBl->z = can_readFloat(int16_t, &imu_unsSbl_inbox, 4, 0.01f);
    imu_unsSbl_inbox.isRecent = false;
  }
  if(imu_unsSbr_inbox.isRecent) {
    accelBr->x = can_readFloat(int16_t, &imu_unsSbr_inbox, 0, 0.01f);
    accelBr->y = can_readFloat(int16_t, &imu_unsSbr_inbox, 2, 0.01f);
    accelBr->z = can_readFloat(int16_t, &imu_unsSbr_inbox, 4, 0.01f);
    imu_unsSbr_inbox.isRecent = false;
  }

}

static void externalImus_getGyros(xyz* gyroHvc, xyz* gyroPdu) {
  if(imu_hvcgyro_inbox.isRecent) {
    gyroHvc->x = can_readFloat(int16_t, &imu_hvcgyro_inbox, 0, 0.01f);
    gyroHvc->y = can_readFloat(int16_t, &imu_hvcgyro_inbox, 2, 0.01f);
    gyroHvc->z = can_readFloat(int16_t, &imu_hvcgyro_inbox, 4, 0.01f);
    imu_hvcgyro_inbox.isRecent = false;
  }
  if(imu_pdugyro_inbox.isRecent) {
    gyroPdu->x = can_readFloat(int16_t, &imu_pdugyro_inbox, 0, 0.01f);
    gyroPdu->y = can_readFloat(int16_t, &imu_pdugyro_inbox, 2, 0.01f);
    gyroPdu->z = can_readFloat(int16_t, &imu_pdugyro_inbox, 4, 0.01f);
    imu_pdugyro_inbox.isRecent = false;
  }
}

void allImus_periodic(ImuData *imuData) {

//  if accel_ready() imu_getAccel(&imuData->accelVcu);
//  if gyro_ready() imu_getGyro(&imuData->gyroVcu);

  externalImus_getAccels(&imuData->accelHvc, &imuData->accelPdu, &imuData->accelFl, &imuData->accelFr, &imuData->accelBl, &imuData->accelBr);
  externalImus_getGyros(&imuData->gyroHvc, &imuData->gyroPdu);
}