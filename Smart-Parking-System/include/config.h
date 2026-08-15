#pragma once

// ===============================
// Smart Parking System Configuration
// ===============================

#include <Arduino.h>

constexpr uint8_t SLOT_COUNT = 4;

// OLED I2C
constexpr uint8_t OLED_SDA = 21;
constexpr uint8_t OLED_SCL = 22;
constexpr uint8_t OLED_ADDRESS = 0x3C;

// Servo gate
constexpr uint8_t SERVO_PIN = 18;
constexpr uint16_t SERVO_FREQ_HZ = 50;
constexpr uint8_t SERVO_RESOLUTION_BITS = 16;
constexpr uint16_t SERVO_CHANNEL = 3; // used on ESP32 Arduino core 3.x
constexpr uint8_t GATE_CLOSED_ANGLE = 0;
constexpr uint8_t GATE_OPEN_ANGLE = 90;

// Buzzer
constexpr uint8_t BUZZER_PIN = 19;

// HC-SR04 TRIG/ECHO pairs
constexpr uint8_t TRIG_PINS[SLOT_COUNT] = {5, 16, 27, 25};
constexpr uint8_t ECHO_PINS[SLOT_COUNT] = {17, 4, 26, 33};

// Two LEDs per slot: green = free, red = occupied.
// IMPORTANT: GPIO34 is input-only on the ESP32, so the old mapping
// {15,34} was invalid for a red LED. GPIO0 is used instead.
constexpr uint8_t GREEN_LED_PINS[SLOT_COUNT] = {12, 14, 32, 15};
constexpr uint8_t RED_LED_PINS[SLOT_COUNT]   = {2, 13, 23, 0};

// Parking thresholds (cm)
constexpr float OCCUPIED_THRESHOLD_CM[SLOT_COUNT] = {35.0f, 35.0f, 35.0f, 35.0f};

// Sensor filtering / debounce
constexpr uint8_t SENSOR_SAMPLES = 5;
constexpr uint32_t ECHO_TIMEOUT_US = 25000;
constexpr uint32_t STATE_DEBOUNCE_MS = 800;
constexpr uint32_t SENSOR_SCAN_INTERVAL_MS = 150;

// UI / alert timing
constexpr uint32_t OLED_UPDATE_MS = 500;
constexpr uint32_t SERIAL_UPDATE_MS = 2000;
constexpr uint32_t FULL_ALERT_INTERVAL_MS = 3000;

// Web
constexpr uint16_t WEB_PORT = 80;

// Wi-Fi
// For Wokwi use:
//   SSID = "Wokwi-GUEST"
//   PASSWORD = ""
// For your real Wi-Fi, replace both values.
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";

// If station Wi-Fi fails, ESP32 creates this fallback AP.
constexpr char AP_SSID[] = "SmartParking-ESP32";
constexpr char AP_PASSWORD[] = "12345678";

// Safety: gate opens only through the web "Open Gate" command.
constexpr uint32_t GATE_OPEN_TIME_MS = 2000;
