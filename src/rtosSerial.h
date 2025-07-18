#ifndef SIMPLE_SERIAL_H
#define SIMPLE_SERIAL_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/*
 * Simple Thread-Safe Serial Interface
 * 
 * Lightweight mutex protection for multi-core ESP32
 * Safe for use across multiple FreeRTOS tasks
 * 
 * Usage:
 *   rtosSerialInit();                    // Call once in setup (required)
 *   rtosPrintln("Hello World");          // Thread-safe print with newline
 *   rtosPrint("Hello ");                 // Thread-safe print without newline
 *   rtosPrintf("Value: %d", 42);         // Thread-safe printf style
 *   String input = rtosRead();           // Thread-safe read (safe for all tasks)
*/

// Simple thread-safe serial functions
void rtosSerialInit();
void rtosPrint(const String& msg);
void rtosPrintln(const String& msg);
void rtosPrintf(const char* format, ...);
String rtosRead();

#endif
