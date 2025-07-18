#include "rtosSerial.h"

// Task buffer structure
struct TaskBuffer {
  TaskHandle_t taskHandle;
  String buffer;
  bool hasData;
};

// Simple mutex for thread safety
static SemaphoreHandle_t serialMutex = nullptr;
static TaskBuffer taskBuffers[MAX_TASK_BUFFERS];
static int registeredTasks = 0;
static TaskHandle_t readerTaskHandle = nullptr;

// Reader task that broadcasts serial input to all registered tasks
void serialReaderTask(void* parameter) {
  while (true) {
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      
      if (input.length() > 0) {
        // Broadcast to all registered task buffers
        if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
          for (int i = 0; i < registeredTasks; i++) {
            taskBuffers[i].buffer = input;  // Overwrite previous (keep only latest)
            taskBuffers[i].hasData = true;
          }
          xSemaphoreGive(serialMutex);
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Small delay to prevent busy waiting
  }
}

void rtosSerialInit() {
  if (serialMutex == nullptr) {
    serialMutex = xSemaphoreCreateMutex();
    
    // Initialize task buffers
    for (int i = 0; i < MAX_TASK_BUFFERS; i++) {
      taskBuffers[i].taskHandle = nullptr;
      taskBuffers[i].buffer = "";
      taskBuffers[i].hasData = false;
    }
    
    // Create the serial reader task
    xTaskCreate(serialReaderTask, "SerialReader", 2048, NULL, 2, &readerTaskHandle);
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
  TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();
  
  if (serialMutex && xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
    // Find or register this task
    int taskIndex = -1;
    
    // Look for existing registration
    for (int i = 0; i < registeredTasks; i++) {
      if (taskBuffers[i].taskHandle == currentTask) {
        taskIndex = i;
        break;
      }
    }
    
    // Register new task if not found and space available
    if (taskIndex == -1 && registeredTasks < MAX_TASK_BUFFERS) {
      taskIndex = registeredTasks;
      taskBuffers[taskIndex].taskHandle = currentTask;
      taskBuffers[taskIndex].buffer = "";
      taskBuffers[taskIndex].hasData = false;
      registeredTasks++;
    }
    
    String result = "";
    if (taskIndex != -1 && taskBuffers[taskIndex].hasData) {
      result = taskBuffers[taskIndex].buffer;
      taskBuffers[taskIndex].buffer = "";  // Clear after reading
      taskBuffers[taskIndex].hasData = false;
    }
    
    xSemaphoreGive(serialMutex);
    return result;
  }
  return "";
}
