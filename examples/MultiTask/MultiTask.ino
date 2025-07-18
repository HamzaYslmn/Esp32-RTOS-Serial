/*
 * Multi-Task Example for Esp32-RTOS-Serial
 * 
 * This example demonstrates thread-safe serial communication
 * across multiple FreeRTOS tasks running simultaneously.
 * 
 * Created by Hamza Yesilmen
 * https://github.com/HamzaYslmn/Esp32-RTOS-Serial
 */

#include <rtosSerial.h>

// Task 1: Print a message every second
void task1(void* parameter) {
  static int counter = 0;  // Use static to ensure proper initialization
  while (true) {
    rtosPrintf("Task 1 - Counter: %d", counter++);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// Task 2: Handle input starting with "task2"
void task2(void* parameter) {
  while (true) {
    String input = rtosRead();
    if ((input.length() > 0) && (input.startsWith("task2"))) {
      rtosPrintln("Task 2 - Received: " + input);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// Task 3: Handle user input
void task3(void* parameter) {
  while (true) {
    String input = rtosRead();
    if ((input.length() > 0) && (input.startsWith("task3"))) {
      rtosPrintln("Task 3 - Received: " + input);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize the thread-safe serial interface
  rtosSerialInit();
  
  rtosPrintln("=== Esp32-RTOS-Serial Multi-Task Example ===");
  rtosPrintln("Three tasks will run simultaneously:");
  rtosPrintln("- Task 1: Counter every 1 second");
  rtosPrintln("- Task 2: Responds to input starting with 'task2'");
  rtosPrintln("- Task 3: Responds to input starting with 'task3'");
  rtosPrintln("Type 'task2 hello' or 'task3 world' to test");
  rtosPrintln("============================================");
  
  // Create tasks
  xTaskCreate(task1, "Task1", 2048, NULL, 1, NULL);
  xTaskCreate(task2, "Task2", 2048, NULL, 1, NULL);
  xTaskCreate(task3, "Task3", 2048, NULL, 1, NULL);
}

void loop() {
  // Main loop can also use serial safely
  rtosPrintln("Main loop - All tasks running safely");
  delay(5000);
}
