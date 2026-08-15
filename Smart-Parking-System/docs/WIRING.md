# Wiring Notes

Use a common ground.

## OLED
SDA -> GPIO21
SCL -> GPIO22
VCC -> 3.3V
GND -> GND

## HC-SR04
S1 TRIG 5, ECHO 17
S2 TRIG 16, ECHO 4
S3 TRIG 27, ECHO 26
S4 TRIG 25, ECHO 33

For physical hardware, put a voltage divider on every ECHO signal before it reaches the ESP32.

## LEDs
Each LED needs a 220 ohm series resistor.

Green: 12,14,32,15
Red: 2,13,23,0

GPIO34 must NOT be used as an LED output because it is input-only on the classic ESP32.

## Servo
Signal -> GPIO18
Power -> suitable 5V supply
GND -> common ground

## Buzzer
Signal -> GPIO19
GND -> common ground
