#include "usb.h"
#include <cctype>
#include <cstring>
#include <cstdio>

static uint8_t buffer[1024];
static uint8_t rxByte = 0;
static volatile bool liveOutputEnabled = false;
static volatile bool pendingStartRequest = false;
static volatile bool pendingStopRequest = false;
static char commandBuffer[32];
static size_t commandLength = 0;

static void armSerialRx() {
  HAL_UART_Receive_IT(&hlpuart1, &rxByte, 1);
}

void println(float f) {
  std::string s = std::to_string(f);
  println(s);
}

void println(std::string& s) {
  if(s.size() > 1000) {
    return;
  }
  strcpy(reinterpret_cast<char *>(buffer), s.c_str());
  size_t size = s.size();
  buffer[size++] = '\n';
  buffer[size] = '\0';
  HAL_UART_Transmit(&hlpuart1, buffer, size, 100);
}

static bool commandEquals(const char* expected) {
  return strcmp(commandBuffer, expected) == 0;
}

static void resetCommandBuffer() {
  commandLength = 0;
  commandBuffer[0] = '\0';
}

static void handleCommand() {
  if (commandEquals("START")) {
    pendingStopRequest = false;
    pendingStartRequest = true;
  } else if (commandEquals("STOP")) {
    pendingStartRequest = false;
    pendingStopRequest = true;
  }
  resetCommandBuffer();
}

static void processByte(uint8_t byte) {
  if (byte == '\r' || byte == '\n') {
    if (commandLength > 0) {
      commandBuffer[commandLength] = '\0';
      handleCommand();
    }
    return;
  }

  if (commandLength >= sizeof(commandBuffer) - 1) {
    resetCommandBuffer();
    return;
  }

  commandBuffer[commandLength++] = static_cast<char>(std::toupper(static_cast<unsigned char>(byte)));
}

void serialControlInit() {
  resetCommandBuffer();
  liveOutputEnabled = false;
  pendingStartRequest = false;
  pendingStopRequest = false;
  armSerialRx();
}

void serialControlPeriodic() {
  if (pendingStopRequest) {
    pendingStopRequest = false;
    liveOutputEnabled = false;
    std::string message = "# live output disabled";
    println(message);
  }

  if (pendingStartRequest) {
    pendingStartRequest = false;
    liveOutputEnabled = true;
    std::string message = "# live output enabled";
    println(message);
  }
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == LPUART1) {
    processByte(rxByte);
    armSerialRx();
  }
}

extern "C" void serialControlHandleUartError(UART_HandleTypeDef* huart) {
  if (huart->Instance == LPUART1) {
    HAL_UART_AbortReceive(huart);
    armSerialRx();
  }
}

void printLiveControls(const AnalogVoltages& analogVoltages, const VcuOutput& vcuOutput) {
  static uint32_t lastPrintMs = 0;
  static bool printedHeader = false;

  if (!liveOutputEnabled) {
    printedHeader = false;
    return;
  }

  const uint32_t nowMs = HAL_GetTick();
  if ((nowMs - lastPrintMs) < 50U) {
    return;
  }
  lastPrintMs = nowMs;

  int size = 0;
  if (!printedHeader) {
    printedHeader = true;
    size = snprintf(reinterpret_cast<char*>(buffer), sizeof(buffer),
                    "# live_controls_csv v1\r\n"
                    "time_ms,apps1_v,apps2_v,apps_pct,bse1_v,bse2_v,bse_pct,steer_v,steer_deg\r\n");
    if (size > 0) {
      HAL_UART_Transmit(&hlpuart1, buffer, static_cast<uint16_t>(size), 100);
    }
  }

  size = snprintf(reinterpret_cast<char*>(buffer), sizeof(buffer),
                  "%lu,%.4f,%.4f,%.3f,%.4f,%.4f,%.3f,%.4f,%.3f\r\n",
                  static_cast<unsigned long>(nowMs),
                  analogVoltages.apps1,
                  analogVoltages.apps2,
                  vcuOutput.telemetryApps,
                  analogVoltages.bse1,
                  analogVoltages.bse2,
                  vcuOutput.telemetryBse,
                  analogVoltages.steer,
                  vcuOutput.telemetrySteeringWheel);

  if (size <= 0 || size >= static_cast<int>(sizeof(buffer))) {
    return;
  }

  HAL_UART_Transmit(&hlpuart1, buffer, static_cast<uint16_t>(size), 100);
}
