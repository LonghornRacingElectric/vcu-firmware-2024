/*
 * GetVCUInputs.cpp
 *
 * Receives data from the following components and stores them in VcuInput
 * Checks whether data can be received from the following components and sets faults accordingly
 * ADCs: apps1, apps2, bse1, bse2, steerpot
 * SPI: onboard-imu, eeprom (CS, MOSI, MISO, CLK)
 * - Onboard IMU: accel1, gyro1
 * UART: gps (UART_TX, UART_RX)
 * - GPS: lat, long, speed, heading
 * - NOTE: could be replaced with cell module.
 * CAN: inverter, hvc, pdu, whs1, whs2, whs3, whs4 (CAN_TX, CAN_RX)
 * - Inverter: inverter temp, inverter ready, motor temp
 * - HVC: battery temp, battery soc, accel2, gyro2 (IMU2)
 * - WHS: fl, fr, bl, br
 * - PDU: accel3, gyro3 (IMU2)
 * GPIO: drive switch (DASH), inverter reset (DASH), calibration (Master Switch ???), shutdown (from BSPD)
 *
 * Also decodes input data and places it in meaningful form, along with logic using GPIO pins.
 */


#include "GetVCUInputs.h"
#include <algorithm>
#include <cstdio>

using namespace std;


uint16_t adcData[ADC_BUF_SIZE];

FDCAN_TxHeaderTypeDef TxHeader;
FDCAN_RxHeaderTypeDef RxHeader;
#define REQUEST_VCU_DATA_ID 0x100
#define HVC_RESPONSE_ID 0x110
#define HVC_IMU2_RESPONSE_ID 0x111
#define INV_RESPONSE_ID 0x120
#define PDU_RESPONSE_ID 0x130
#define PDU_IMU3_RESPONSE_ID 0x131
#define WHS1_RESPONSE_ID 0x140
#define WHS2_RESPONSE_ID 0x150
#define WHS3_RESPONSE_ID 0x160
#define WHS4_RESPONSE_ID 0x170

uint8_t TxData[CAN_DATA_SIZE] = {0};
uint8_t RxData[CAN_DATA_SIZE] = {0};

uint8_t HVCData[CAN_DATA_SIZE] = {0};
uint8_t INVData[CAN_DATA_SIZE] = {0};
uint8_t PDUData[CAN_DATA_SIZE] = {0};
uint8_t WHSData[CAN_DATA_SIZE] = {0};

uint8_t IMU2Data[CAN_DATA_SIZE] = {0};
uint8_t IMU3Data[CAN_DATA_SIZE] = {0};

float IMUACCELSCALAR = 0;
float IMUGYROSCALAR = 0;

char TxUARTCmds[UART_BUF_SIZE] = {0};
char RxUARTCmds[UART_BUF_SIZE] = {0};

uint8_t TxSPI[SPI_BUF_SIZE] = {0};
uint8_t RxSPI[SPI_BUF_SIZE] = {0};


uint32_t Get_VCU_Inputs(VcuInput* input,
                        VcuParameters* params,
                        ADC_HandleTypeDef* hadc1,
                        FDCAN_HandleTypeDef* hfdcan1,
                        SPI_HandleTypeDef* hspi1,
                        UART_HandleTypeDef* huart1){
  uint32_t this_shutdown = 0;

  // Start polling ADC Data
  if(HAL_ADC_Start_DMA(hadc1, (uint32_t*)adcData, ADC_BUF_SIZE) != HAL_OK){
    return Critical_Error_Handler(ADC_DATA_FAULT);
  }
  // Request data from HVC, INV, PDU, WHS from CAN
  init_TX(REQUEST_VCU_DATA_ID);
  TxData[0] = 0x01;
  if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan1, &TxHeader, TxData) != HAL_OK) {
    return Critical_Error_Handler(VCU_DATA_FAULT);
  }

  //Request data from IMU
  HAL_SPI_Transmit_IT(hspi1, (uint8_t*)TxSPI, SPI_BUF_SIZE);
  HAL_Delay(1);

  input->apps1 = adcData[APPS1_CHANNEL] * params->apps1VoltageMax / 65536.0;
  input->apps2 = adcData[APPS2_CHANNEL] * params->apps2VoltageMax / 65536.0;
  input->bse1 = adcData[BSE1_CHANNEL] * params->bseVoltageMax / 65536.0;
  input->bse2 = adcData[BSE2_CHANNEL] * params->bseVoltageMax / 65536.0;
  input->steeringWheelPotVoltage = adcData[STEER_CHANNEL] * 5.0 / 65536.0;

  if(HAL_ADC_Stop_DMA(hadc1) != HAL_OK){
    return Critical_Error_Handler(ADC_DATA_FAULT);
  }

  // Next with UART
  // Start DMA with UART now
  sprintf((char*)TxUARTCmds, "Give me the data");

  HAL_UART_Transmit_IT(huart1, (uint8_t*)TxUARTCmds, UART_BUF_SIZE);
  HAL_UART_Receive_IT(huart1, (uint8_t*)RxUARTCmds, UART_BUF_SIZE);

  return 0;
  // Set data from HVC, INV, PDU, WHS from CAN
  if(update_HVC(input) != 0){
    set_fault(HVC_DATA_FAULT);
    return 1;
  }
  if(update_INV(input) != 0){
    set_fault(INVERTER_DATA_FAULT);
    return 1;
  }
  if(update_PDU(input) != 0){
    set_fault(PDU_DATA_FAULT);
  }
  if(update_WHS(input) != 0){
    set_fault(WHEELSPEED_DATA_FAULT);
  }
  if(update_IMU(input) != 0){
    set_fault(IMU_DATA_FAULT);
  }

  // Next with SPI (synchronous)
  HAL_SPI_Receive_IT(hspi1, (uint8_t*)RxSPI, SPI_BUF_SIZE);






  // Finally with GPIO, and commit logic based on this
    return 0;
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs){
  if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == RESET){
    return;
  }
  if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
  {
      /* Reception Error */
    Critical_Error_Handler(GENERIC_CAN_DATA_FAULT);
  }
  if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
  {
      /* Notification Error */
      Critical_Error_Handler(VCU_DATA_FAULT);
  }
  switch (RxHeader.Identifier){
    case HVC_RESPONSE_ID:
      copy(begin(RxData), end(RxData), HVCData);
      break;
    case INV_RESPONSE_ID:
      copy(begin(RxData), end(RxData), INVData);
      break;
    case PDU_RESPONSE_ID:
      copy(begin(RxData), end(RxData), PDUData);
      break;
    case WHS1_RESPONSE_ID:
      copy(begin(RxData), end(RxData), WHSData);
      break;
    case WHS2_RESPONSE_ID:
      copy(begin(RxData), end(RxData), WHSData + 4);
      break;
    case WHS3_RESPONSE_ID:
      copy(begin(RxData), end(RxData), WHSData + 8);
      break;
    case WHS4_RESPONSE_ID:
      copy(begin(RxData), end(RxData), WHSData + 12);
      break;
    case HVC_IMU2_RESPONSE_ID:
      copy(begin(RxData), end(RxData), IMU2Data);
      break;
    case PDU_IMU3_RESPONSE_ID:
      copy(begin(RxData), end(RxData), IMU3Data);
      break;
    default:
      break;
  }
}

void init_TX(uint32_t identifier){
  TxHeader.Identifier = identifier;
  TxHeader.IdType = FDCAN_STANDARD_ID;
  TxHeader.TxFrameType = FDCAN_DATA_FRAME;
  TxHeader.DataLength = FDCAN_DLC_BYTES_16;
  TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
  TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
  TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  TxHeader.MessageMarker = 0;
}

int update_HVC(VcuInput* input){
  if(HVCData[2] == 0){
    return 1;
  }
  input->batteryTemp = HVCData[0];
  input->batterySoc = HVCData[1];
  return 0;
}

int update_INV(VcuInput* input){
  if(INVData[2] == 0){
    return 1;
  }
  input->inverterTemp = INVData[0];
  input->motorTemp = INVData[1];
  return 0;
}

int update_PDU(VcuInput* input){
  if(INVData[0] == 0){
    return 1;
  }
  return 0;
}

int update_WHS(VcuInput* input){
  return 0;
}

int update_IMU(VcuInput* input){
  if(IMU2Data[12] == 0){
    return 1;
  }
  uint16_t imu2AccelX = IMU2Data[0] << 8 | IMU2Data[1];
  uint16_t imu2AccelY = IMU2Data[2] << 8 | IMU2Data[3];
  uint16_t imu2AccelZ = IMU2Data[4] << 8 | IMU2Data[5];
  uint16_t imu2GyroX = IMU2Data[6] << 8 | IMU2Data[7];
  uint16_t imu2GyroY = IMU2Data[8] << 8 | IMU2Data[9];
  uint16_t imu2GyroZ = IMU2Data[10] << 8 | IMU2Data[11];

  input->imu2Accel = {imu2AccelX * IMUACCELSCALAR, imu2AccelY * IMUACCELSCALAR, imu2AccelZ * IMUACCELSCALAR};
  input->imu2Gyro = {imu2GyroX * IMUGYROSCALAR, imu2GyroY * IMUGYROSCALAR, imu2GyroZ * IMUGYROSCALAR};

  if(IMU2Data[12] == 0){
    return 1;
  }
  uint16_t imu3AccelX = IMU3Data[0] << 8 | IMU3Data[1];
  uint16_t imu3AccelY = IMU3Data[2] << 8 | IMU3Data[3];
  uint16_t imu3AccelZ = IMU3Data[4] << 8 | IMU3Data[5];
  uint16_t imu3GyroX = IMU3Data[6] << 8 | IMU3Data[7];
  uint16_t imu3GyroY = IMU3Data[8] << 8 | IMU3Data[9];
  uint16_t imu3GyroZ = IMU3Data[10] << 8 | IMU3Data[11];

  input->imu3Accel = {imu3AccelX * IMUACCELSCALAR, imu3AccelY * IMUACCELSCALAR, imu3AccelZ * IMUACCELSCALAR};
  input->imu3Gyro = {imu3GyroX * IMUGYROSCALAR, imu3GyroY * IMUGYROSCALAR, imu3GyroZ * IMUGYROSCALAR};



  return 0;
}

int Critical_Error_Handler(uint32_t fault_type){
  global_shutdown = 1;
  set_fault(fault_type);
  return 1;
}

int Noncritical_Error_Handler(uint32_t fault_type){
  set_fault(fault_type);
  return 0;
}
