#ifndef SIMPLE_SERIAL_H
#define SIMPLE_SERIAL_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

/*
 * Simple Thread-Safe Serial Interface
 * 
 * Lightweight mutex protection for multi-core ESP32
 * Safe for use across multiple FreeRTOS tasks
 * Each task gets its own configurable buffer for reading
 * Prevents RAM overflow with controlled memory allocation
 * 
 * Usage:
 *   rtosSerialInit();                    // Call once in setup with default 100-byte buffers
 *   rtosSerialInit(128);                 // Call once in setup with custom buffer size (128 bytes)
 *   rtosPrintln("Hello World");          // Thread-safe print with newline
 *   rtosPrint("Hello ");                 // Thread-safe print without newline
 *   rtosPrintf("Value: %d", 42);         // Thread-safe printf style
 *   String input = rtosRead();           // Thread-safe read (each task gets own buffer)
*/

// Maximum number of tasks that can have individual buffers
#define MAX_TASK_BUFFERS 8

// Default buffer size per task (100 bytes to prevent RAM overflow)
#define DEFAULT_BUFFER_SIZE 100

// Maximum allowed buffer size per task (safety limit)
#define MAX_ALLOWED_BUFFER_SIZE 512

// Simple thread-safe serial functions
void rtosSerialInit();                    // Initialize with default buffer size (100 bytes)
void rtosSerialInit(int bufferSize);      // Initialize with custom buffer size per task
void rtosPrint(const String& msg);
void rtosPrintln(const String& msg);
void rtosPrintf(const char* format, ...);
String rtosRead();

#endif
