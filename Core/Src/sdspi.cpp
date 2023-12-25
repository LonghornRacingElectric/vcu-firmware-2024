//
// Created by Matthew on 12/19/2023.
//

#include "sdspi.h"
#include "main.h"

bool sdspi_isCardPresent() {
  return !HAL_GPIO_ReadPin(SD_DETECT_GPIO_Port, SD_DETECT_Pin);
}

static void sdspi_setPinSdo(bool on) {
  HAL_GPIO_WritePin(SD_SDO_GPIO_Port, SD_SDO_Pin, (GPIO_PinState) on);
}

static void sdspi_setPinClk(bool on) {
  HAL_GPIO_WritePin(SD_SCK_GPIO_Port, SD_SCK_Pin, (GPIO_PinState) on);
}

static bool sdspi_getPinSdi() {
  return (bool) HAL_GPIO_ReadPin(SD_SDI_GPIO_Port, SD_SDI_Pin);
}

static void sdspi_delay() {
  for(volatile int i = 0; i < 100; i++);
}

void sdspi_init() {
  sdspi_deselect();
  sdspi_setPinClk(true);
}

static void sdspi_sendCommand(uint8_t index, uint32_t argument, uint8_t crc) {
  uint8_t buffer[6];
  buffer[0] = index | 0b01000000;
  buffer[1] = argument >> 24;
  buffer[2] = (argument >> 16) & 0xFF;
  buffer[3] = (argument >> 8) & 0xFF;
  buffer[4] = argument & 0xFF;
  buffer[5] = (crc << 1) | 1;
  sdspi_send(buffer, 6);
  sdspi_setPinSdo(true);
}

static uint8_t sdspi_receiveR1() {
  for(int i = 0; i < 64; i++) {
    sdspi_setPinClk(false);
    sdspi_delay();
    sdspi_setPinClk(true);
    if(!sdspi_getPinSdi()) {
      sdspi_delay();
      break;
    }
    sdspi_delay();
  }
  uint8_t res = 0;
  for(uint8_t m = 0x40; m; m >>= 1) {
    sdspi_setPinClk(false);
    sdspi_delay();
    sdspi_setPinClk(true);
    uint8_t bit = sdspi_getPinSdi();
    res |= m * bit;
    sdspi_delay();
  }
  return res;
}

static uint32_t sdspi_receiveWord() {
  uint32_t ocr = 0;
  for(uint32_t m = 0x80000000U; m; m >>= 1) {
    sdspi_setPinClk(false);
    sdspi_delay();
    sdspi_setPinClk(true);
    uint8_t bit = sdspi_getPinSdi();
    ocr |= m * bit;
    sdspi_delay();
  }
  return ocr;
}

static void sdspi_runClock() {
  for(int i = 0; i < 8; i++) {
    sdspi_setPinClk(false);
    sdspi_delay();
    sdspi_setPinClk(true);
    sdspi_delay();
  }
}

void sdspi_start() {
  volatile uint8_t r1 = 0;
  volatile uint32_t res = 0;

  // nonsense clock cycles
  sdspi_setPinSdo(true);
  for(int i = 0; i < 100; i++) {
    sdspi_setPinClk(false);
    sdspi_delay();
    sdspi_setPinClk(true);
    sdspi_delay();
  }

  // CMD0 - reset
  sdspi_select();
  sdspi_sendCommand(0, 0, 0b1001010);
  r1 = sdspi_receiveR1();
  sdspi_runClock();
  sdspi_deselect();
  if(r1 != 0x01) {
    Error_Handler();
  }

  // CMD8 - check voltage range
  sdspi_select();
  sdspi_sendCommand(8, 0x000001AA, 0x43);
  r1 = sdspi_receiveR1();
  res = sdspi_receiveWord();
  sdspi_runClock();
  sdspi_deselect();
  if (r1 != 0x01 || res != 0x000001AA) {
    Error_Handler();
  }

  uint32_t i = 0;
  for(; i < 1000; i++) {
    // CMD 55 - app specific
    sdspi_select();
    sdspi_sendCommand(55, 0, 0x32);
    r1 = sdspi_receiveR1();
    sdspi_runClock();
    sdspi_deselect();
    if (r1 != 0x01) {
      Error_Handler();
    }

    // ACMD41 - initialize
    sdspi_select();
    sdspi_sendCommand(41, 0x40000000, 0x77);
    r1 = sdspi_receiveR1();
    sdspi_runClock();
    sdspi_deselect();
    if (r1 == 0x00) {
      break;
    } else if (r1 != 0x01) {
      Error_Handler();
    }

    HAL_Delay(1);
  }

  if(i == 1000) {
    Error_Handler();
  }
}

void sdspi_deselect() {
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
  sdspi_delay();
}

void sdspi_select() {
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
  sdspi_delay();
}

void sdspi_send(uint8_t* data, uint32_t length) {
  for(; length; length--, data++) {
    for(uint8_t m = 0x80; m; m >>= 1) {
      bool bit = (bool) (*data & m);
      sdspi_setPinSdo(bit);
      sdspi_setPinClk(false);
      sdspi_delay();
      sdspi_setPinClk(true);
      sdspi_delay();
    }
  }
}

void sdspi_receive(uint8_t* data, uint32_t length) {
  for(; length; length--, data++) {
    *data = 0;
    for(uint8_t m = 0x80; m; m >>= 1) {
      sdspi_setPinClk(false);
      sdspi_delay();
      sdspi_setPinClk(true);
      uint8_t bit = sdspi_getPinSdi();
      *data |= m * bit;
      sdspi_delay();
    }
  }
}