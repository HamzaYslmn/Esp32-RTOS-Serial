#include "rtosSerial.h"

struct TaskBuffer {
  TaskHandle_t taskHandle;
  RingbufHandle_t ringbuf;
  size_t ringSize;
};

static SemaphoreHandle_t serialMutex = nullptr;
static TaskBuffer taskBuffers[MAX_TASK_BUFFERS];
static size_t registeredTasks = 0;
static size_t configuredRingSize = DEFAULT_RING_SIZE;
static TaskHandle_t readerTaskHandle = nullptr;

static void serialReaderTask(void* /*pv*/) {
  while (true) {
    if (Serial.available()) {
      String line = Serial.readStringUntil('\n');
      line.trim();
      if (line.length() > 0) {
        line += '\0';  // include terminator
        const char* raw = line.c_str();
        size_t len = line.length() + 1;

        // Broadcast to every registered ring buffer
        for (size_t i = 0; i < registeredTasks; ++i) {
          RingbufHandle_t rb = taskBuffers[i].ringbuf;
          if (!rb) continue;
          // If there is no space, discard the old message and try again
          if (xRingbufferSend(rb, raw, len, 0) != pdTRUE) {
            size_t rcvLen;
            char* discard = (char*)xRingbufferReceive(rb, &rcvLen, 0);
            if (discard) {
              vRingbufferReturnItem(rb, discard);
            }
            xRingbufferSend(rb, raw, len, 10 / portTICK_PERIOD_MS);
          }
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void rtosSerialInit() {
  rtosSerialInit(DEFAULT_RING_SIZE);
}

void rtosSerialInit(size_t ringSize) {
  if (ringSize < 64) ringSize = DEFAULT_RING_SIZE;
  if (ringSize > MAX_RING_SIZE) ringSize = MAX_RING_SIZE;

  configuredRingSize = ringSize;

  if (!serialMutex) {
    serialMutex = xSemaphoreCreateMutex();
    for (int i = 0; i < MAX_TASK_BUFFERS; ++i) {
      taskBuffers[i].taskHandle = nullptr;
      taskBuffers[i].ringbuf   = nullptr;
      taskBuffers[i].ringSize  = 0;
    }
    xTaskCreatePinnedToCore(serialReaderTask, "SerialReader", 4096, nullptr, 2, &readerTaskHandle, 1);
  }
}

static TaskBuffer* getOrRegisterTaskBuffer() {
  TaskHandle_t current = xTaskGetCurrentTaskHandle();

  // Find existing one
  for (size_t i = 0; i < registeredTasks; ++i) {
    if (taskBuffers[i].taskHandle == current) {
      return &taskBuffers[i];
    }
  }

  // New registration
  if (registeredTasks >= MAX_TASK_BUFFERS) return nullptr;

  TaskBuffer& tb = taskBuffers[registeredTasks];
  tb.taskHandle = current;
  tb.ringSize   = configuredRingSize;
  tb.ringbuf    = xRingbufferCreate(tb.ringSize, RINGBUF_TYPE_NOSPLIT);
  if (!tb.ringbuf) return nullptr;
  registeredTasks++;
  return &tb;
}

String rtosRead() {
  TaskBuffer* tb = getOrRegisterTaskBuffer();
  if (!tb || !tb->ringbuf) return "";

  size_t len;
  char* item = (char*)xRingbufferReceive(tb->ringbuf, &len, 0);
  if (item) {
    String res(item);
    vRingbufferReturnItem(tb->ringbuf, item);
    return res;
  }
  return "";
}

void rtosPrint(const String& msg) {
  if (!serialMutex) return;
  if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
    Serial.print(msg);
    xSemaphoreGive(serialMutex);
  }
}

void rtosPrintln(const String& msg) {
  if (!serialMutex) return;
  if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
    Serial.println(msg);
    xSemaphoreGive(serialMutex);
  }
}

void rtosPrintf(const char* format, ...) {
  if (!serialMutex) return;
  if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    Serial.println(buf);
    xSemaphoreGive(serialMutex);
  }
}