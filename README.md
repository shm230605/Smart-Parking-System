# 🚗 Smart Parking System

An ESP32-based smart parking management system that monitors parking-slot occupancy in real time, displays parking availability on an OLED screen, controls an entry/exit barrier, and provides visual and audible status indications.

The project is designed for both **physical ESP32 hardware** and **Wokwi simulation**.

---

## 📌 Project Overview

The Smart Parking System automatically monitors four parking slots using ultrasonic distance sensors.

The system:

- Detects whether each parking slot is available or occupied.
- Displays parking availability on a 128×64 OLED display.
- Shows the number of available parking spaces.
- Controls an entry/exit barrier using a servo motor.
- Uses LEDs to indicate individual slot availability.
- Uses a buzzer for system notifications.
- Provides entry and exit push buttons.
- Supports Wokwi simulation.
- Runs on an ESP32 Dev Module using PlatformIO.

---

## ✨ Features

### 🅿️ Four Parking Slots

The system contains four independent parking slots.

Each slot is monitored using an HC-SR04 ultrasonic sensor.

```text
Slot 1 → Ultrasonic Sensor
Slot 2 → Ultrasonic Sensor
Slot 3 → Ultrasonic Sensor
Slot 4 → Ultrasonic Sensor
```

The current firmware uses:

```text
Distance ≤ 15 cm → OCCUPIED
Distance > 15 cm → AVAILABLE
```

---

### 🖥️ OLED Display

A 128×64 SSD1306 OLED display provides the current parking status.

Example:

```text
SMART PARKING
----------------
S1: FREE   S2: FULL
S3: FREE   S4: FREE

AVAILABLE: 3
```

The display updates according to the detected parking-slot status.

---

### 🚦 Entry and Exit Control

Two push buttons are provided:

```text
ENTRY BUTTON
EXIT BUTTON
```

The entry button can be used to simulate a vehicle entering the parking area.

The exit button can be used to simulate a vehicle leaving the parking area.

A servo motor represents the parking barrier.

```text
Barrier CLOSED → 0°
Barrier OPEN   → 90°
```

---

### 💡 Slot Status LEDs

Each parking slot has an LED indicator.

| Slot | ESP32 GPIO | LED Status |
|------|------------|------------|
| Slot 1 | GPIO 23 | ON = Available |
| Slot 2 | GPIO 17 | ON = Available |
| Slot 3 | GPIO 2 | ON = Available |
| Slot 4 | GPIO 15 | ON = Available |

When a slot becomes occupied, its corresponding LED turns off.

---

### 🔊 Buzzer

The buzzer is connected to GPIO 19.

It provides audible feedback for system events and notifications.

---

## 🧰 Hardware Components

| Component | Quantity |
|-----------|----------|
| ESP32 Dev Module | 1 |
| HC-SR04 Ultrasonic Sensor | 4 |
| SSD1306 128×64 OLED | 1 |
| Servo Motor | 1 |
| Buzzer | 1 |
| Push Button | 2 |
| LEDs | 4 |
| Resistors | As required |
| Jumper Wires | As required |
| Breadboard | 1 |

---

## 🔌 Pin Mapping

|   Component    | Pin / Signal |ESP32 GPIO|
|----------------|--------------|----------|
|  OLED SSD1306  |      SDA     | GPIO 21  |
|  OLED SSD1306  |      SCL     | GPIO 22  |
| HC-SR04 Slot 1 |      TRIG    | GPIO 13  |
| HC-SR04 Slot 1 |      ECHO    | GPIO 14  |
| HC-SR04 Slot 2 |      TRIG    | GPIO 25  |
| HC-SR04 Slot 2 |      ECHO    | GPIO 26  |
| HC-SR04 Slot 3 |      TRIG    | GPIO 27  |
| HC-SR04 Slot 3 |      ECHO    | GPIO 32  |
| HC-SR04 Slot 4 |      TRIG    | GPIO 33  |
| HC-SR04 Slot 4 |      ECHO    | GPIO 34  |
| Servo Barrier  |     Signal   | GPIO 18  |
| Buzzer | Signal|     GPIO 19  |          |
| Entry Button   |     Signal   |  GPIO 4  |
|   Exit Button  |     Signal   | GPIO 16  |
|    Slot 1 LED  |     Signal   | GPIO 23  |
|   Slot 2 LED   |     Signal   | GPIO 17  |
|   Slot 3 LED   |     Signal   |  GPIO 2  |
|   Slot 4 LED   |     Signal   | GPIO 15  |

For the complete pin reference, see [`pin-mapping.md`](pin-mapping.md).

---

## 🏗️ System Architecture

```text
                 ┌─────────────────────┐
                 │       ESP32         │
                 │   Main Controller   │
                 └──────────┬──────────┘
                            │
          ┌─────────────────┼─────────────────┐
          │                 │                 │
          ▼                 ▼                 ▼
   ┌─────────────┐   ┌─────────────┐   ┌─────────────┐
   │ Ultrasonic  │   │    OLED     │   │ Entry/Exit  │
   │  Sensors    │   │   Display   │   │   Buttons   │
   │    S1-S4    │   │  SSD1306    │   │             │
   └─────────────┘   └─────────────┘   └──────┬──────┘
                                               │
                                               ▼
                                        ┌─────────────┐
                                        │    Servo    │
                                        │   Barrier   │
                                        └─────────────┘

          ┌─────────────────┐     ┌─────────────────┐
          │    Slot LEDs    │     │     Buzzer      │
          │      S1-S4      │     │                 │
          └─────────────────┘     └─────────────────┘
```

---

## 🔄 System Operation

The basic operating sequence is:

```text
START
  │
  ▼
Initialize ESP32
  │
  ├── Initialize OLED
  ├── Initialize Ultrasonic Sensors
  ├── Initialize LEDs
  ├── Initialize Servo
  └── Initialize Buzzer
  │
  ▼
Read Parking Sensors
  │
  ▼
Determine Slot Status
  │
  ├── Available
  └── Occupied
  │
  ▼
Update OLED Display
  │
  ▼
Update Slot LEDs
  │
  ▼
Check Entry / Exit Buttons
  │
  ▼
Control Barrier
  │
  ▼
Repeat
```

---

## 📊 Parking Status Logic

Each parking slot is independently monitored.

```text
                 Distance
                    │
                    ▼
              ┌───────────┐
              │ ≤ 15 cm ? │
              └─────┬─────┘
                    │
           ┌────────┴────────┐
           │                 │
          YES                NO
           │                 │
           ▼                 ▼
      OCCUPIED           AVAILABLE
           │                 │
           ▼                 ▼
        LED OFF            LED ON
```

The available-slot count is calculated from the four slot states.

---

## 🧪 Wokwi Simulation

The project includes Wokwi simulation files:

```text
simulation/
├── diagram.json
└── wokwi.toml
```

The simulation allows the parking system to be tested without physical hardware.

### Simulation Workflow

1. Open the project in VS Code.
2. Build the PlatformIO project.
3. Start the Wokwi simulation.
4. Observe the OLED display.
5. Change ultrasonic sensor distances.
6. Observe individual parking-slot status changes.
7. Test the entry button.
8. Test the exit button.
9. Observe the servo barrier.
10. Observe the LEDs and buzzer.

---

## 🛠️ The project uses:

```text
Platform: Espressif32
Board: ESP32 Dev Module
Framework: Arduino
Language: C++
```

---

## 📁 Project Structure

```text
Smart-Parking-System/
│
├── README.md
├── .gitignore
├── pin-mapping.md
├── platformio.ini
│
├── src/
│   └── main.cpp
│
└── simulation/
    ├── diagram.json
    └── wokwi.toml
```

---


## 🚧 Project Limitations

This project is a prototype smart-parking system.

It currently provides:

- Four parking slots
- Local OLED status
- Ultrasonic occupancy detection
- Servo barrier control
- Entry/exit buttons
- LED indicators
- Buzzer notifications
- Wokwi simulation

It does not currently include:

- Cloud database
- Mobile application
- RFID authentication
- License-plate recognition
- Payment processing
- Remote monitoring
- Camera-based vehicle detection

---

## 🔮 Future Improvements

Possible future improvements include:

- RFID-based vehicle identification
- Automatic number-plate recognition
- Mobile/web dashboard
- Cloud-based parking records
- Real-time IoT monitoring
- Parking reservation
- Vehicle model classification
- Entry/exit logging
- Automatic gate control
- Database integration
- More parking slots
- Emergency/manual override
- Improved power management

---

## 👨‍💻 Development

The project is developed using:

```text
ESP32
Arduino Framework
PlatformIO
Wokwi
C++
```

---
## 👨‍💻 Author
Shresthaa Maiti
 
---

## ⭐ Project Summary

The Smart Parking System demonstrates how an ESP32 can combine sensors, actuators, user controls, and a display into a practical embedded-system application.

The system continuously monitors four parking spaces and provides real-time visual and audible feedback while controlling a simulated parking barrier.
