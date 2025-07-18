/*
 * Basic Usage Example for Esp32-RTOS-Serial
 * 
 * This example demonstrates basic thread-safe serial communication
 * using the rtosSerial library.
 * 
 * Created by Hamza Yesilmen
 * https://github.com/HamzaYslmn/Esp32-RTOS-Serial
 */

#include <rtosSerial.h>

void setup() {
  Serial.begin(115200);
  
  // Initialize the thread-safe serial interface
  rtosSerialInit();
  
  rtosPrintln("=== Esp32-RTOS-Serial Basic Example ===");
  rtosPrintln("Thread-safe serial communication initialized");
  rtosPrintln("Type something and press Enter to echo back");
  rtosPrintln("=========================================");
}

void loop() {
  // Thread-safe serial operations
  rtosPrint("Current millis: ");
  rtosPrintf("%lu", millis());
  
  // Check for user input
  String input = rtosRead();
  if (input.length() > 0) {
    rtosPrintln("You typed: " + input);
  }
  
  delay(1000);
}
