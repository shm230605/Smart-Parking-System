# Smart Parking System — Warnings and Notes

## 1. ESP32 Power

Use a suitable regulated power supply for the ESP32 and connected peripherals.

## 2. Servo Motor

A servo motor may require more current than the ESP32 can safely provide.

For a physical installation, use an appropriate external servo power supply and connect the grounds together.

## 3. Ultrasonic Sensor Echo Voltage

Check the voltage level of the ultrasonic sensor's ECHO output before connecting it directly to an ESP32 GPIO.

The ESP32 GPIO pins are not 5 V tolerant.

Use an appropriate voltage-divider or level-shifting circuit when required.

## 4. GPIO Selection

Do not change GPIO assignments in `main.cpp` without also updating the circuit/simulation wiring.

The following must remain consistent:

- `main.cpp`
- `diagram.json`
- physical circuit
- pin-mapping documentation

## 5. OLED Address

The OLED configuration uses the I2C address defined by the working project.

If the physical OLED does not respond, verify its I2C address and wiring.

## 6. Wokwi vs Physical Hardware

Successful Wokwi simulation does not guarantee that a physical circuit will work without modification.

Physical hardware may require:

- correct power supply
- common ground
- voltage-level protection
- appropriate current capability
- correct sensor wiring

## 7. Buzzer

The buzzer should only operate during configured system events.

Continuous buzzing may indicate incorrect wiring or incorrect software configuration.

## 8. Servo Movement

Keep the mechanical barrier lightweight and ensure that the servo does not become mechanically blocked.

## 9. Backup

Before modifying the working firmware, create a backup copy of:

```text
src/main.cpp