# Smart Parking System — Testing Report

## 1. Test Objective

The objective of testing is to verify that the ESP32 Smart Parking System correctly detects, displays, and manages parking-slot status and that the connected components operate as expected.

## 2. Hardware / Simulation Components

- ESP32 development board
- SSD1306 OLED display
- Ultrasonic sensors
- Servo motor
- Push buttons
- LEDs
- Buzzer
- Wokwi simulation environment
- PlatformIO

## 3. Functional Tests

| Test ID | Test | Expected Result | Status |
|---|---|---|---|
| T01 | ESP32 starts | System initializes successfully | PASS |
| T02 | OLED initialization | OLED displays startup/system information | PASS |
| T03 | All slots free | OLED shows all slots as FREE | PASS |
| T04 | Vehicle enters Slot 1 | Slot 1 changes to FULL | PASS |
| T05 | Vehicle enters Slot 3 | Slot 3 changes to FULL | PASS |
| T06 | Vehicle exits Slot 1 | Slot 1 changes to FREE | PASS |
| T07 | Multiple occupied slots | Correct number of FULL slots displayed | PASS |
| T08 | All slots occupied | OLED reports PARKING FULL | PASS |
| T09 | Entry operation | Barrier responds correctly | PASS |
| T10 | Exit operation | Barrier responds correctly | PASS |
| T11 | LED status | LED status corresponds to parking availability | PASS |
| T12 | Buzzer event | Buzzer responds to configured events | PASS |

## 4. OLED Display Tests

The following display states were tested:

1. Startup screen
2. All parking slots available
3. One occupied slot
4. Multiple occupied slots
5. All parking slots occupied

## 5. Simulation Tests

The Wokwi simulation was used to verify the behavior of the ESP32 system before physical hardware deployment.

## 6. Build Test

The project was compiled using PlatformIO.

Expected result:

```text
SUCCESS