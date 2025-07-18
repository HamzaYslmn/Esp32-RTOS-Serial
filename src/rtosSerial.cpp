#include "rtosSerial.h"

// Simple mutex for thread safety
static SemaphoreHandle_t serialMutex = nullptr;

void rtosSerialInit() {
  if (serialMutex == nullptr) {
    serialMutex = xSemaphoreCreateMutex();
  }
}

void rtosPrint(const String& msg) {
  if (serialMutex && xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
    Serial.print(msg);
    xSemaphoreGive(serialMutex);
  }
}

void rtosPrintln(const String& msg) {
  if (serialMutex && xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
    Serial.println(msg);
    xSemaphoreGive(serialMutex);
  }
}

void rtosPrintf(const char* format, ...) {
  if (serialMutex && xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
    va_list args;
    va_start(args, format);
    Serial.printf(format, args);
    Serial.println();
    va_end(args);
    xSemaphoreGive(serialMutex);
  }
}

String rtosRead() {
  if (serialMutex && xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
    String result = "";
    if (Serial.available()) {
      result = Serial.readStringUntil('\n');
      result.trim();
    }
    xSemaphoreGive(serialMutex);
    return result;
  }
  return "";
}
