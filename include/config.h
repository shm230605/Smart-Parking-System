#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// SMART PARKING SYSTEM - HARDWARE CONFIGURATION
// ESP32 Dev Module
// ============================================================

// ---------------- OLED ----------------
#define OLED_WIDTH       128
#define OLED_HEIGHT       64
#define OLED_ADDRESS    0x3C

#define OLED_SDA_PIN      21
#define OLED_SCL_PIN      22

// ---------------- Parking Sensors ----------------
// Four parking slots.
// HC-SR04-style ultrasonic sensors.
//
// IMPORTANT:
// For real hardware, use a voltage divider on each HC-SR04
// ECHO line before connecting it to an ESP32 GPIO.

#define SLOT_COUNT 4

#define SLOT1_TRIG  5
#define SLOT1_ECHO  18

#define SLOT2_TRIG  19
#define SLOT2_ECHO  23

#define SLOT3_TRIG  25
#define SLOT3_ECHO  26

#define SLOT4_TRIG  27
#define SLOT4_ECHO  32

// Distance below this value = occupied.
#define OCCUPIED_DISTANCE_CM 12.0

// ---------------- Entrance / Exit ----------------
#define ENTRY_BUTTON_PIN  13
#define EXIT_BUTTON_PIN   14

// ---------------- LEDs ----------------
// One RGB status LED can be represented by 3 GPIOs.
#define STATUS_RED_PIN    16
#define STATUS_GREEN_PIN  17
#define STATUS_BLUE_PIN   4

// ---------------- Buzzer ----------------
#define BUZZER_PIN        15

// ---------------- Servo ----------------
// Entrance barrier servo.
#define SERVO_PIN          2

// ---------------- System ----------------
#define SERIAL_BAUD      115200

// Sensor timeout.
#define ULTRASONIC_TIMEOUT_US 30000UL

#endif