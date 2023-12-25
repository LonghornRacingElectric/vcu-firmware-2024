//
// Created by Matthew on 12/19/2023.
//

#include <cstring>
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
  // no-op is still slow enough
}

static void sdspi_deselect() {
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
  sdspi_delay();
}

static void sdspi_select() {
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
  sdspi_delay();
}

static void sdspi_sendCommand(uint8_t index, uint32_t argument, uint8_t crc) {
  uint8_t buffer[6];
  buffer[0] = index | 0b01000000;
  buffer[1] = argument >> 24;
  buffer[2] = (argument >> 16) & 0xFF;
  buffer[3] = (argument >> 8) & 0xFF;
  buffer[4] = argument & 0xFF;
  buffer[5] = (crc << 1) | 1;
  for(uint8_t byte : buffer) {
    for (uint8_t m = 0x80; m; m >>= 1) {
      bool bit = (bool) (byte & m);
      sdspi_setPinSdo(bit);
      sdspi_setPinClk(false);
      sdspi_delay();
      sdspi_setPinClk(true);
      sdspi_delay();
    }
  }
  sdspi_setPinSdo(true);
}

static uint8_t sdspi_receiveR1() {
  int i = 0;
  for(; i < 64; i++) {
    sdspi_setPinClk(false);
    sdspi_delay();
    sdspi_setPinClk(true);
    if(!sdspi_getPinSdi()) {
      sdspi_delay();
      break;
    }
    sdspi_delay();
  }
  if(i == 64) {
    Error_Handler();
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
  uint32_t word = 0;
  for(uint32_t m = 0x80000000U; m; m >>= 1) {
    sdspi_setPinClk(false);
    sdspi_delay();
    sdspi_setPinClk(true);
    uint8_t bit = sdspi_getPinSdi();
    word |= m * bit;
    sdspi_delay();
  }
  return word;
}

static void sdspi_receiveBlock(uint8_t* data, uint16_t* crc) {
  int i = 0;
  for(; i < 10000; i++) {
    sdspi_setPinClk(false);
    sdspi_delay();
    sdspi_setPinClk(true);
    if(!sdspi_getPinSdi()) {
      sdspi_delay();
      break;
    }
    sdspi_delay();
  }
  if(i == 10000) {
    Error_Handler();
  }

  for(i = 0; i < 512; i++, data++) {
    *data = 0;
    for(uint8_t m = 0x80u; m; m >>= 1) {
      sdspi_setPinClk(false);
      sdspi_delay();
      sdspi_setPinClk(true);
      uint8_t bit = sdspi_getPinSdi();
      *data |= m * bit;
      sdspi_delay();
    }
  }
  *crc = 0;
  for(uint16_t m = 0x8000u; m; m >>= 1) {
    sdspi_setPinClk(false);
    sdspi_delay();
    sdspi_setPinClk(true);
    uint8_t bit = sdspi_getPinSdi();
    *crc |= m * bit;
    sdspi_delay();
  }
}

static void sdspi_runClock() {
  for(int i = 0; i < 8; i++) {
    sdspi_setPinClk(false);
    sdspi_delay();
    sdspi_setPinClk(true);
    sdspi_delay();
  }
}

void sdspi_init() {
  uint8_t r1;
  uint32_t res;

  if(!sdspi_isCardPresent()) {
    Error_Handler();
  }

  // nonsense clock cycles
  sdspi_deselect();
  sdspi_setPinClk(true);
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

  uint32_t attempts = 0;
  uint32_t maxAttempts = 1000;
  for(; attempts < maxAttempts; attempts++) {
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

  if(attempts == maxAttempts) {
    Error_Handler();
  }
}

void sdspi_readBlock(uint32_t address, uint8_t data[512]) {
  uint8_t r1;
  uint16_t crc;

  sdspi_select();
  sdspi_sendCommand(17, address, 0);
  r1 = sdspi_receiveR1();
  if(r1 != 0x00) {
    Error_Handler();
  }
  sdspi_receiveBlock(data, &crc);
  sdspi_runClock();
  sdspi_deselect();

  if(crc != 0) {
    volatile int x = 0;
    x++;
  }
}
