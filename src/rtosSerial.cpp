#include "rtosSerial.h"

// Task buffer structure
struct TaskBuffer {
  TaskHandle_t taskHandle;
  char* buffer;  // Dynamic buffer to support configurable size
  bool hasData;
  int bufferSize;  // Track buffer size for each task
};

// Simple mutex for thread safety
static SemaphoreHandle_t serialMutex = nullptr;
static TaskBuffer taskBuffers[MAX_TASK_BUFFERS];
static int registeredTasks = 0;
static TaskHandle_t readerTaskHandle = nullptr;
static int configuredBufferSize = DEFAULT_BUFFER_SIZE;  // Track configured buffer size

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
            if (taskBuffers[i].buffer != nullptr) {
              // Copy input to buffer, truncate if necessary
              strncpy(taskBuffers[i].buffer, input.c_str(), taskBuffers[i].bufferSize - 1);
              taskBuffers[i].buffer[taskBuffers[i].bufferSize - 1] = '\0';  // Ensure null termination
              taskBuffers[i].hasData = true;
            }
          }
          xSemaphoreGive(serialMutex);
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Small delay to prevent busy waiting
  }
}

void rtosSerialInit() {
  rtosSerialInit(DEFAULT_BUFFER_SIZE);  // Use default buffer size
}

void rtosSerialInit(int bufferSize) {
  // Validate buffer size
  if (bufferSize < 1) {
    bufferSize = DEFAULT_BUFFER_SIZE;
  } else if (bufferSize > MAX_ALLOWED_BUFFER_SIZE) {
    bufferSize = MAX_ALLOWED_BUFFER_SIZE;
  }
  
  if (serialMutex == nullptr) {
    configuredBufferSize = bufferSize;
    serialMutex = xSemaphoreCreateMutex();
    
    // Initialize task buffers
    for (int i = 0; i < MAX_TASK_BUFFERS; i++) {
      taskBuffers[i].taskHandle = nullptr;
      taskBuffers[i].buffer = nullptr;  // Will be allocated when task registers
      taskBuffers[i].hasData = false;
      taskBuffers[i].bufferSize = 0;
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
    char buffer[256];  // Local buffer for formatted string
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.println(buffer);
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
      taskBuffers[taskIndex].bufferSize = configuredBufferSize;
      taskBuffers[taskIndex].buffer = (char*)malloc(configuredBufferSize);
      if (taskBuffers[taskIndex].buffer != nullptr) {
        taskBuffers[taskIndex].buffer[0] = '\0';  // Initialize empty string
        taskBuffers[taskIndex].hasData = false;
        registeredTasks++;
      } else {
        // Failed to allocate memory, reset task index
        taskIndex = -1;
      }
    }
    
    String result = "";
    if (taskIndex != -1 && taskBuffers[taskIndex].hasData && taskBuffers[taskIndex].buffer != nullptr) {
      result = String(taskBuffers[taskIndex].buffer);
      taskBuffers[taskIndex].buffer[0] = '\0';  // Clear after reading
      taskBuffers[taskIndex].hasData = false;
    }
    
    xSemaphoreGive(serialMutex);
    return result;
  }
  return "";
}
