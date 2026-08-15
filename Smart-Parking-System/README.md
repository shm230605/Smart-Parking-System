# Smart Parking System - ESP32

Professional PlatformIO + Wokwi project for a 4-slot smart parking prototype.

## Main features

- Four HC-SR04 parking sensors
- Filtered distance measurement
- 35 cm configurable occupancy threshold
- 800 ms state debounce
- Green/red slot LEDs
- SSD1306 128x64 I2C OLED
- SG90 servo barrier gate
- Buzzer parking-full alert
- ESP32 Wi-Fi web dashboard
- JSON API at `/api/status`
- Web gate controls
- Wokwi simulation

## Important fixes

1. Do not type library names or documentation text into PowerShell.
2. Adafruit libraries are installed through `platformio.ini` and PlatformIO.
3. GPIO34 cannot drive an LED on a classic ESP32, so Slot 4 red LED was moved to GPIO0.
4. The old `ledcSetup()` / `ledcAttachPin()` calls were replaced with the Arduino-ESP32 3.x LEDC API.
5. `/api/status` is an HTTP route. Open it in a browser or call it with an HTTP client; it is not a PowerShell command.
6. Wokwi configuration is a file, not a terminal command.

## Build

From the project root:

```powershell
pio pkg install
pio run
```

For a physical ESP32:

```powershell
pio run -t upload
pio device monitor
```

## Wokwi

Compile first:

```powershell
pio run
```

Then open `diagram.json` in VS Code with the Wokwi extension and start the simulator.

HC-SR04 distances can be changed in the Wokwi sensor controls.

## Wi-Fi

For Wokwi the default credentials are:

- SSID: `Wokwi-GUEST`
- Password: empty

For real hardware, change `WIFI_SSID` and `WIFI_PASSWORD` in `include/config.h`.

If station Wi-Fi fails, the ESP32 starts:

- SSID: `SmartParking-ESP32`
- Password: `12345678`

## Hardware safety

HC-SR04 ECHO can be 5 V. A classic ESP32 GPIO is 3.3 V logic. Use a resistor divider or suitable level shifter on every ECHO line for real hardware.

Power the servo from a suitable 5 V supply and share GND with the ESP32.

GPIO0 is a boot-strapping pin. The provided LED wiring uses the LED/resistor arrangement so it is not actively pulled low during reset. If your physical board behaves differently at boot, use an I/O expander for the LED bank instead.

## Pin map

| Function | GPIO |
|---|---:|
| OLED SDA | 21 |
| OLED SCL | 22 |
| Servo | 18 |
| Buzzer | 19 |
| S1 TRIG/ECHO | 5 / 17 |
| S2 TRIG/ECHO | 16 / 4 |
| S3 TRIG/ECHO | 27 / 26 |
| S4 TRIG/ECHO | 25 / 33 |
| S1 green/red | 12 / 2 |
| S2 green/red | 14 / 13 |
| S3 green/red | 32 / 23 |
| S4 green/red | 15 / 0 |

## API

- `/` - dashboard
- `/api/status` - JSON status
- `/api/gate/open` - open gate if a slot is available
- `/api/gate/close` - close gate

