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
  HAL_GPIO_WritePin(IMU_SPI_MISO_GPIO_Port, IMU_SPI_MISO_Pin, (GPIO_PinState) on);
}

static void bbspi_setPinClk(bool on) {
  HAL_GPIO_WritePin(IMU_SPI_CLK_GPIO_Port, IMU_SPI_CLK_Pin, (GPIO_PinState) on);
}

static bool bbspi_getPinSdi() {
  return (bool) HAL_GPIO_ReadPin(IMU_SPI_MOSI_GPIO_Port, IMU_SPI_MOSI_Pin);
}

static void bbspi_delay() {
  // no-op, the overhead from doing this in software is enough delay
  // for reference, this implementation runs at around 750 kHz (peripherals support up to 10 MHz)
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