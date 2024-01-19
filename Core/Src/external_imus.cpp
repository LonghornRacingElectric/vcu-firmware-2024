#include "external_imus.h"
#include "angel_can_ids.h"
#include "angel_can.h"

static CanInbox imu_hvcaccel_inbox;
static CanInbox imu_hvcgyro_inbox;
static CanInbox imu_pduaccel_inbox;
static CanInbox imu_pdugyro_inbox;
static CanInbox imu_whsfr_inbox;
static CanInbox imu_whsfl_inbox;
static CanInbox imu_whsbr_inbox;
static CanInbox imu_whsbl_inbox;

void externalImus_init() {
    // TODO implement
    can_addInbox(HVC_VCU_IMU_ACCEL, &imu_hvcaccel_inbox);
    can_addInbox(HVC_VCU_IMU_GYRO, &imu_hvcgyro_inbox);
    can_addInbox(PDU_VCU_IMU_ACCEL, &imu_pduaccel_inbox);
    can_addInbox(PDU_VCU_IMU_GYRO, &imu_pdugyro_inbox);
    can_addInbox(WHS_VCU_IMU_1, &imu_whsfr_inbox);
    can_addInbox(WHS_VCU_IMU_2, &imu_whsfl_inbox);
    can_addInbox(WHS_VCU_IMU_3, &imu_whsbr_inbox);
    can_addInbox(WHS_VCU_IMU_4, &imu_whsbl_inbox);
}

void externalImus_getAccels(xyz* accel1, xyz* accel2, xyz* accelFl, xyz* accelFr, xyz* accelBl, xyz* accelBr) {
    if(imu_hvcaccel_inbox.isRecent) {
        accel1->x = (float) can_readBytes(imu_hvcaccel_inbox.data, 0, 1) / 100.0f;
        accel1->y = (float) can_readBytes(imu_hvcaccel_inbox.data, 2, 3) / 100.0f;
        accel1->z = (float) can_readBytes(imu_hvcaccel_inbox.data, 4, 5) / 100.0f;
        imu_hvcaccel_inbox.isRecent = false;
    }
    if(imu_pduaccel_inbox.isRecent) {
        accel2->x = (float) can_readBytes(imu_pduaccel_inbox.data, 0, 1) / 100.0f;
        accel2->y = (float) can_readBytes(imu_pduaccel_inbox.data, 2, 3) / 100.0f;
        accel2->z = (float) can_readBytes(imu_pduaccel_inbox.data, 4, 5) / 100.0f;
        imu_pduaccel_inbox.isRecent = false;
    }
    if(imu_whsfl_inbox.isRecent) {
        accelFl->x = (float) can_readBytes(imu_whsfl_inbox.data, 0, 1) / 100.0f;
        accelFl->y = (float) can_readBytes(imu_whsfl_inbox.data, 2, 3) / 100.0f;
        accelFl->z = (float) can_readBytes(imu_whsfl_inbox.data, 4, 5) / 100.0f;
        imu_whsfl_inbox.isRecent = false;
    }
    if(imu_whsfr_inbox.isRecent) {
        accelFr->x = (float) can_readBytes(imu_whsfr_inbox.data, 0, 1) / 100.0f;
        accelFr->y = (float) can_readBytes(imu_whsfr_inbox.data, 2, 3) / 100.0f;
        accelFr->z = (float) can_readBytes(imu_whsfr_inbox.data, 4, 5) / 100.0f;
        imu_whsfr_inbox.isRecent = false;
    }
    if(imu_whsbl_inbox.isRecent) {
        accelBl->x = (float) can_readBytes(imu_whsbl_inbox.data, 0, 1) / 100.0f;
        accelBl->y = (float) can_readBytes(imu_whsbl_inbox.data, 2, 3) / 100.0f;
        accelBl->z = (float) can_readBytes(imu_whsbl_inbox.data, 4, 5) / 100.0f;
        imu_whsbl_inbox.isRecent = false;
    }
    if(imu_whsbr_inbox.isRecent) {
        accelBr->x = (float) can_readBytes(imu_whsbr_inbox.data, 0, 1) / 100.0f;
        accelBr->y = (float) can_readBytes(imu_whsbr_inbox.data, 2, 3) / 100.0f;
        accelBr->z = (float) can_readBytes(imu_whsbr_inbox.data, 4, 5) / 100.0f;
        imu_whsbr_inbox.isRecent = false;
    }

}

void externalImus_getGyros(xyz* gyro1, xyz* gyro2) {
    if(imu_hvcgyro_inbox.isRecent) {
        gyro1->x = (float) can_readBytes(imu_hvcgyro_inbox.data, 0, 1) / 100.0f;
        gyro1->y = (float) can_readBytes(imu_hvcgyro_inbox.data, 2, 3) / 100.0f;
        gyro1->z = (float) can_readBytes(imu_hvcgyro_inbox.data, 4, 5) / 100.0f;
        imu_hvcgyro_inbox.isRecent = false;
    }
    if(imu_pdugyro_inbox.isRecent) {
        gyro2->x = (float) can_readBytes(imu_pdugyro_inbox.data, 0, 1) / 100.0f;
        gyro2->y = (float) can_readBytes(imu_pdugyro_inbox.data, 2, 3) / 100.0f;
        gyro2->z = (float) can_readBytes(imu_pdugyro_inbox.data, 4, 5) / 100.0f;
        imu_pdugyro_inbox.isRecent = false;
    }
}
