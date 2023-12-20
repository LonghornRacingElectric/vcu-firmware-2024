//
// Created by Matthew on 12/19/2023.
//

#include "bbspi.h"
#include "main.h"

static void bbspi_setPinCsImu(bool on) {
  HAL_GPIO_WritePin(SPI_CS_IMU_GPIO_Port, SPI_CS_IMU_Pin, (GPIO_PinState) on);
}

static void bbspi_setPinCsEeprom(bool on) {
  HAL_GPIO_WritePin(SPI_CS_EEPROM_GPIO_Port, SPI_CS_EEPROM_Pin, (GPIO_PinState) on);
}

static void bbspi_setPinSdo(bool on) {
  HAL_GPIO_WritePin(BBSPI_SDO_GPIO_Port, BBSPI_SDO_Pin, (GPIO_PinState) on);
}

static void bbspi_setPinClk(bool on) {
  HAL_GPIO_WritePin(BBSPI_CLK_GPIO_Port, BBSPI_CLK_Pin, (GPIO_PinState) on);
}

static bool bbspi_getPinSdi() {
  return (bool) HAL_GPIO_ReadPin(BBSPI_SDI_GPIO_Port, BBSPI_SDI_Pin);
}

static void bbspi_delay() {
  for(volatile int i = 0; i < 10; i++);
}

void bbspi_init() {
  bbspi_setPinCsImu(true);
  bbspi_setPinCsEeprom(true);
  bbspi_setPinClk(true);
}

void bbspi_deselect() {
  bbspi_setPinCsImu(true);
  bbspi_setPinCsEeprom(true);
  bbspi_delay();
}

void bbspi_selectImu() {
  bbspi_deselect();
  bbspi_setPinCsImu(false);
  bbspi_delay();
}

void bbspi_selectEeprom() {
  bbspi_deselect();
  bbspi_setPinCsEeprom(false);
  bbspi_delay();
}

void bbspi_send(uint8_t* data, uint32_t length) {
  for(; length; length--, data++) {
    for(uint8_t m = 0x80; m; m >>= 1) {
      bool bit = (bool) (*data & m);

      bbspi_setPinSdo(bit);
      bbspi_setPinClk(false);
      bbspi_delay();
      bbspi_setPinClk(true);
      bbspi_delay();
    }
  }
}

void bbspi_receive(uint8_t* data, uint32_t length) {
  for(; length; length--, data++) {
    *data = 0;
    for(uint8_t m = 0x80; m; m >>= 1) {
      bbspi_setPinClk(false);
      bbspi_delay();
      bbspi_setPinClk(true);
      uint8_t bit = bbspi_getPinSdi();
      *data |= m * bit;
      bbspi_delay();
    }
  }
}