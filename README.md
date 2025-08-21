# Esp32-RTOS-Serial

A lightweight, thread-safe Serial communication library for ESP32 with FreeRTOS support.

## Features

- **Thread-Safe**: Uses mutex protection for multi-core ESP32 applications
- **Lightweight**: Minimal overhead with simple mutex implementation
- **Easy to Use**: Drop-in replacement for standard Serial functions
- **FreeRTOS Compatible**: Safe for use across multiple FreeRTOS tasks

## Installation

### Arduino Library Manager
1. Open Arduino IDE
2. Go to Sketch → Include Library → Manage Libraries
3. Search for "Esp32-RTOS-Serial"
4. Click Install

### Manual Installation
1. Download this repository as ZIP
2. In Arduino IDE: Sketch → Include Library → Add .ZIP Library
3. Select the downloaded ZIP file

## Usage

```cpp
#include <rtosSerial.h>

void setup() {
  Serial.begin(115200);
  rtosSerialInit(128); // Initialize the thread-safe serial interface, 128byte buffer each task
}

void loop() {
  // Thread-safe serial operations
  rtosPrintln("Hello World");
  rtosPrint("Hello ");
  rtosPrintf("Value: %d", 42);
  
  // Thread-safe read 
  String input = rtosRead();
  if (input.length() > 0) {
    rtosPrintln("Received: " + input);
  }
  
  delay(1000);
}
```

## API Reference

### Functions

#### `void rtosSerialInit()`
Initialize the thread-safe serial interface. **Must be called once in setup()**.

#### `void rtosPrint(const String& msg)`
Thread-safe print without newline.

#### `void rtosPrintln(const String& msg)`
Thread-safe print with newline.

#### `void rtosPrintf(const char* format, ...)`
Thread-safe printf-style printing with automatic newline.

#### `String rtosRead()`
Thread-safe read until newline. Returns empty string if no data available.
**Note**: Use only in one task to avoid conflicts.

## Multi-Task Example

```cpp
#include <rtosSerial.h>

void task1(void* parameter) {
  while (true) {
    rtosPrintln("Task 1 running");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void task2(void* parameter) {
  while (true) {
    rtosPrintf("Task 2 counter: %d", millis());
    vTaskDelay(pdMS_TO_TICKS(1500));
  }
}

void setup() {
  Serial.begin(115200);
  rtosSerialInit();
  
  xTaskCreate(task1, "Task1", 2048, NULL, 1, NULL);
  xTaskCreate(task2, "Task2", 2048, NULL, 1, NULL);
}

void loop() {
  // Main loop can also use serial safely
  rtosPrintln("Main loop");
  delay(2000);
}
```

## Requirements

- ESP32 board
- Arduino IDE with ESP32 board package
- FreeRTOS (included with ESP32 Arduino Core)

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Author

**Hamza Yesilmen**
- GitHub: [@HamzaYslmn](https://github.com/HamzaYslmn)
- Email: resmiyslmn@gmail.com