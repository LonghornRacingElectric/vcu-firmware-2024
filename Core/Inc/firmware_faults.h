//
// Created by yashk on 10/15/2023.
//

#ifndef VCU_FIRMWARE_2024_FIRMWARE_FAULTS_H
#define VCU_FIRMWARE_2024_FIRMWARE_FAULTS_H

#include <cstdint>

static uint32_t vcu_fault_vector = 0x0;
static uint32_t global_shutdown = 0;

typedef struct {
  int BrakePressed;
  int BrakeBroken;
  int MotorON;
  int MotorPressed;
  int BSPDShutdown;
} BSPD;

//Core Faults which are derived from VCU CORE Outputs, see Libs/VcuCore/src/VcuModel.h
#define APPS_FAULT 0x00000001
#define BSE_FAULT 0x00000002
#define STOMPP_FAULT 0x00000004
#define TORQUE_MAP_FAULT 0x00000008
#define TRACTION_CONTROL_FAULT 0x00000010
#define TRACK_POSITIONING_FAULT 0x00000020
#define STEERING_FAULT 0x0000040

//Write faults where data cannot be trasmitted
#define DRS_DATA_FAULT 0x00000100
#define TORQUEREQUEST_DATA_FAULT 0x00000200
#define CARDIAGNOSTICS_DATA_FAULT 0x00000400

//Read faults where data is corrupted, not received, or unable to be requested
#define ADC_DATA_FAULT 0x00010000
#define IMU_DATA_FAULT 0x00080000
#define WHEELSPEED_DATA_FAULT 0x00100000 //activates if 2+ wheelspeeds are unable to be read
#define CELLULAR_DATA_FAULT 0x01000000
#define GPS_DATA_FAULT 0x02000000
#define INVERTER_DATA_FAULT 0x04000000
#define HVC_DATA_FAULT 0x08000000
#define PDU_DATA_FAULT 0x10000000
#define GENERIC_CAN_DATA_FAULT 0x20000000
#define VCU_DATA_FAULT 0x40000000

#define set_fault(fault) vcu_fault_vector |= fault
#define clear_fault(fault) vcu_fault_vector &= ~fault
#define clear_all_faults() vcu_fault_vector = 0x0

#define VCU_FAULT_LIST vcu_fault_vector



#endif //VCU_FIRMWARE_2024_FIRMWARE_FAULTS_H
