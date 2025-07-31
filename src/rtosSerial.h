#ifndef RTOS_SERIAL_H
#define RTOS_SERIAL_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <freertos/ringbuf.h>

/*
 * Simple Thread-Safe Serial Interface (v2)
 * ----------------------------------------
 * - Same public API (rtosSerialInit, rtosPrint/ln/f, rtosRead)
 * - Incoming data with lock-free ring buffer for each task
 * - Background reader task broadcasts each line to all registered
 *   ring buffers
 * - Ring buffer size can be set with rtosSerialInit()
 * - Lightweight: single mutex for writes, reads are non-blocking
 */

#define MAX_TASK_BUFFERS 8
#define DEFAULT_RING_SIZE 512
#define MAX_RING_SIZE 1024

void rtosSerialInit();                 // default ring size
void rtosSerialInit(size_t ringSize);  // custom ring size

void rtosPrint(const String& msg);
void rtosPrintln(const String& msg);
void rtosPrintf(const char* format, ...);

String rtosRead();                     // Non-blocking; returns "" if no data
size_t rtosReadBytes(uint8_t* buf, size_t maxlen); // Non-blocking; returns number of bytes read

#endif  // RTOS_SERIAL_H