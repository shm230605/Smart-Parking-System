#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// ============================================================
// SMART PARKING SYSTEM
// ESP32 + OLED + 4x HC-SR04 + Servo + Buzzer + LEDs + Buttons
//
// FEATURES
// ------------------------------------------------------------
// 1. Four parking slots
// 2. OLED status display
// 3. HC-SR04 vehicle detection
// 4. ENTRY button: fills first available slot
// 5. EXIT button: frees first occupied slot
// 6. Servo barrier
// 7. Buzzer indication
// 8. Individual slot LEDs
// 9. Serial diagnostic information
// ============================================================


// ============================================================
// OLED
// ============================================================

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDRESS 0x3C

#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SSD1306 display(
    OLED_WIDTH,
    OLED_HEIGHT,
    &Wire,
    -1
);


// ============================================================
// PARKING SLOT SENSOR PINS
// ============================================================

// SLOT 1
#define SLOT1_TRIG 13
#define SLOT1_ECHO 14

// SLOT 2
#define SLOT2_TRIG 25
#define SLOT2_ECHO 26

// SLOT 3
#define SLOT3_TRIG 27
#define SLOT3_ECHO 32

// SLOT 4
#define SLOT4_TRIG 33
#define SLOT4_ECHO 34


// ============================================================
// SERVO
// ============================================================

#define SERVO_PIN 18

Servo barrierServo;

const int BARRIER_CLOSED_ANGLE = 0;
const int BARRIER_OPEN_ANGLE   = 90;


// ============================================================
// BUZZER
// ============================================================

#define BUZZER_PIN 19


// ============================================================
// ENTRY / EXIT BUTTONS
// ============================================================

#define ENTRY_BUTTON_PIN 4
#define EXIT_BUTTON_PIN 16


// ============================================================
// SLOT LEDs
// ============================================================

#define SLOT1_LED 23
#define SLOT2_LED 17
#define SLOT3_LED 2
#define SLOT4_LED 15


// ============================================================
// PARKING SETTINGS
// ============================================================

// If a vehicle is closer than this distance,
// the slot is considered occupied.
//
// Wokwi HC-SR04 supports a distance range of 2cm-400cm.
const float OCCUPIED_DISTANCE_CM = 15.0;


// ============================================================
// SLOT DATA
// ============================================================

bool slotOccupied[4] = {
    false,
    false,
    false,
    false
};


// Manual occupancy is used by ENTRY/EXIT.
//
// This is separate from ultrasonic detection so that pressing
// ENTRY actually fills a slot even when no virtual car is near
// the ultrasonic sensor.
bool manualOccupied[4] = {
    false,
    false,
    false,
    false
};


// Latest measured distances
float slotDistance[4] = {
    400.0,
    400.0,
    400.0,
    400.0
};


// ============================================================
// TIMERS
// ============================================================

unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastSerialUpdate = 0;

const unsigned long SENSOR_INTERVAL = 250;
const unsigned long DISPLAY_INTERVAL = 250;
const unsigned long SERIAL_INTERVAL = 2000;


// ============================================================
// BUTTON DEBOUNCE
// ============================================================

bool lastEntryReading = HIGH;
bool lastExitReading  = HIGH;

bool stableEntryState = HIGH;
bool stableExitState  = HIGH;

unsigned long entryDebounceTime = 0;
unsigned long exitDebounceTime  = 0;

const unsigned long DEBOUNCE_DELAY = 40;


// ============================================================
// FUNCTION DECLARATIONS
// ============================================================

float readDistanceCM(uint8_t trigPin, uint8_t echoPin);

void updateSensors();

void updateParkingState();

void handleButtons();

bool buttonPressed(
    uint8_t pin,
    bool &lastReading,
    bool &stableState,
    unsigned long &lastDebounce
);

void processEntry();

void processExit();

void openBarrier();

void closeBarrier();

void beepShort();

void beepSuccess();

void beepError();

void updateSlotLEDs();

void updateOLED();

void printSerialStatus();

int countFreeSlots();

int countOccupiedSlots();

int findFirstFreeSlot();

int findFirstOccupiedSlot();


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(500);

    Serial.println();
    Serial.println("======================================");
    Serial.println("     SMART PARKING SYSTEM");
    Serial.println("======================================");
    Serial.println("System starting...");
    Serial.println();


    // --------------------------------------------------------
    // I2C
    // --------------------------------------------------------

    Wire.begin(
        OLED_SDA,
        OLED_SCL
    );


    // --------------------------------------------------------
    // OLED
    // --------------------------------------------------------

    if (!display.begin(
            SSD1306_SWITCHCAPVCC,
            OLED_ADDRESS
        ))
    {
        Serial.println("ERROR: OLED initialization failed!");

        while (true)
        {
            delay(1000);
        }
    }

    Serial.println("OLED initialized successfully.");


    // --------------------------------------------------------
    // OLED startup screen
    // --------------------------------------------------------

    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("SMART PARKING");

    display.setCursor(0, 16);
    display.println("SYSTEM STARTING...");

    display.setCursor(0, 32);
    display.println("ESP32 ONLINE");

    display.setCursor(0, 48);
    display.println("PLEASE WAIT");

    display.display();

    delay(1200);


    // --------------------------------------------------------
    // HC-SR04
    // --------------------------------------------------------

    pinMode(SLOT1_TRIG, OUTPUT);
    pinMode(SLOT1_ECHO, INPUT);

    pinMode(SLOT2_TRIG, OUTPUT);
    pinMode(SLOT2_ECHO, INPUT);

    pinMode(SLOT3_TRIG, OUTPUT);
    pinMode(SLOT3_ECHO, INPUT);

    pinMode(SLOT4_TRIG, OUTPUT);
    pinMode(SLOT4_ECHO, INPUT);

    digitalWrite(SLOT1_TRIG, LOW);
    digitalWrite(SLOT2_TRIG, LOW);
    digitalWrite(SLOT3_TRIG, LOW);
    digitalWrite(SLOT4_TRIG, LOW);


    // --------------------------------------------------------
    // Buttons
    // --------------------------------------------------------

    pinMode(
        ENTRY_BUTTON_PIN,
        INPUT_PULLUP
    );

    pinMode(
        EXIT_BUTTON_PIN,
        INPUT_PULLUP
    );


    // --------------------------------------------------------
    // LEDs
    // --------------------------------------------------------

    pinMode(SLOT1_LED, OUTPUT);
    pinMode(SLOT2_LED, OUTPUT);
    pinMode(SLOT3_LED, OUTPUT);
    pinMode(SLOT4_LED, OUTPUT);

    digitalWrite(SLOT1_LED, LOW);
    digitalWrite(SLOT2_LED, LOW);
    digitalWrite(SLOT3_LED, LOW);
    digitalWrite(SLOT4_LED, LOW);


    // --------------------------------------------------------
    // Buzzer
    // --------------------------------------------------------

    pinMode(
        BUZZER_PIN,
        OUTPUT
    );

    digitalWrite(
        BUZZER_PIN,
        LOW
    );


    // --------------------------------------------------------
    // Servo
    // --------------------------------------------------------

    barrierServo.setPeriodHertz(50);

    int servoAttachResult =
        barrierServo.attach(
            SERVO_PIN,
            500,
            2400
        );

    if (servoAttachResult == 0)
    {
        Serial.println(
            "WARNING: Servo attach failed."
        );
    }
    else
    {
        Serial.println(
            "Servo initialized successfully."
        );
    }

    closeBarrier();


    // --------------------------------------------------------
    // Initial display
    // --------------------------------------------------------

    updateParkingState();
    updateSlotLEDs();
    updateOLED();


    Serial.println();
    Serial.println("======================================");
    Serial.println("SMART PARKING READY");
    Serial.println("======================================");
    Serial.println();
    Serial.println("ENTRY button:");
    Serial.println("  -> first available slot becomes FULL");
    Serial.println();
    Serial.println("EXIT button:");
    Serial.println("  -> first occupied slot becomes FREE");
    Serial.println();
    Serial.println("HC-SR04:");
    Serial.println("  -> distance <= 15cm = FULL");
    Serial.println("  -> distance > 15cm = FREE");
    Serial.println();
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop()
{
    unsigned long now = millis();


    // --------------------------------------------------------
    // BUTTONS
    // --------------------------------------------------------

    handleButtons();


    // --------------------------------------------------------
    // SENSOR UPDATE
    // --------------------------------------------------------

    if (now - lastSensorRead >= SENSOR_INTERVAL)
    {
        lastSensorRead = now;

        updateSensors();

        updateParkingState();

        updateSlotLEDs();
    }


    // --------------------------------------------------------
    // OLED
    // --------------------------------------------------------

    if (now - lastDisplayUpdate >= DISPLAY_INTERVAL)
    {
        lastDisplayUpdate = now;

        updateOLED();
    }


    // --------------------------------------------------------
    // SERIAL
    // --------------------------------------------------------

    if (now - lastSerialUpdate >= SERIAL_INTERVAL)
    {
        lastSerialUpdate = now;

        printSerialStatus();
    }

    delay(5);
}


// ============================================================
// READ HC-SR04
// ============================================================

float readDistanceCM(
    uint8_t trigPin,
    uint8_t echoPin
)
{
    // Make sure trigger starts LOW
    digitalWrite(
        trigPin,
        LOW
    );

    delayMicroseconds(3);


    // Trigger pulse
    digitalWrite(
        trigPin,
        HIGH
    );

    delayMicroseconds(10);

    digitalWrite(
        trigPin,
        LOW
    );


    // Read echo
    unsigned long duration =
        pulseIn(
            echoPin,
            HIGH,
            25000UL
        );


    // No echo
    if (duration == 0)
    {
        return 400.0;
    }


    // HC-SR04:
    // distance(cm) = duration(us) / 58
    float distance =
        duration / 58.0;


    if (distance < 2.0)
    {
        distance = 2.0;
    }

    if (distance > 400.0)
    {
        distance = 400.0;
    }


    return distance;
}


// ============================================================
// UPDATE SENSOR VALUES
// ============================================================

void updateSensors()
{
    slotDistance[0] =
        readDistanceCM(
            SLOT1_TRIG,
            SLOT1_ECHO
        );

    delay(2);

    slotDistance[1] =
        readDistanceCM(
            SLOT2_TRIG,
            SLOT2_ECHO
        );

    delay(2);

    slotDistance[2] =
        readDistanceCM(
            SLOT3_TRIG,
            SLOT3_ECHO
        );

    delay(2);

    slotDistance[3] =
        readDistanceCM(
            SLOT4_TRIG,
            SLOT4_ECHO
        );
}


// ============================================================
// COMBINE SENSOR + MANUAL STATES
// ============================================================

void updateParkingState()
{
    for (int i = 0; i < 4; i++)
    {
        bool sensorOccupied =
            slotDistance[i] <=
            OCCUPIED_DISTANCE_CM;

        slotOccupied[i] =
            manualOccupied[i] ||
            sensorOccupied;
    }
}


// ============================================================
// BUTTON HANDLING
// ============================================================

void handleButtons()
{
    if (
        buttonPressed(
            ENTRY_BUTTON_PIN,
            lastEntryReading,
            stableEntryState,
            entryDebounceTime
        )
    )
    {
        processEntry();
    }


    if (
        buttonPressed(
            EXIT_BUTTON_PIN,
            lastExitReading,
            stableExitState,
            exitDebounceTime
        )
    )
    {
        processExit();
    }
}


// ============================================================
// DEBOUNCED BUTTON
// ============================================================

bool buttonPressed(
    uint8_t pin,
    bool &lastReading,
    bool &stableState,
    unsigned long &lastDebounce
)
{
    bool reading =
        digitalRead(pin);


    if (reading != lastReading)
    {
        lastDebounce =
            millis();

        lastReading =
            reading;
    }


    if (
        millis() - lastDebounce
        > DEBOUNCE_DELAY
    )
    {
        if (reading != stableState)
        {
            stableState =
                reading;

            // Active LOW
            if (stableState == LOW)
            {
                return true;
            }
        }
    }


    return false;
}


// ============================================================
// ENTRY
// ============================================================

void processEntry()
{
    Serial.println();
    Serial.println("ENTRY REQUEST");


    // Update before making decision
    updateSensors();
    updateParkingState();


    int slot =
        findFirstFreeSlot();


    // --------------------------------------------------------
    // PARKING FULL
    // --------------------------------------------------------

    if (slot == -1)
    {
        Serial.println(
            "ENTRY DENIED: PARKING FULL"
        );

        beepError();

        updateOLED();

        return;
    }


    // --------------------------------------------------------
    // Assign vehicle
    // --------------------------------------------------------

    manualOccupied[slot] =
        true;

    updateParkingState();

    updateSlotLEDs();

    updateOLED();


    Serial.print(
        "Vehicle assigned to SLOT "
    );

    Serial.println(
        slot + 1
    );


    // --------------------------------------------------------
    // Open barrier
    // --------------------------------------------------------

    beepSuccess();

    openBarrier();

    delay(1200);

    closeBarrier();


    updateOLED();
}


// ============================================================
// EXIT
// ============================================================

void processExit()
{
    Serial.println();
    Serial.println("EXIT REQUEST");


    updateSensors();
    updateParkingState();


    int slot =
        findFirstOccupiedSlot();


    if (slot == -1)
    {
        Serial.println(
            "EXIT: NO OCCUPIED SLOT"
        );

        beepError();

        updateOLED();

        return;
    }


    // Clear manual occupancy
    manualOccupied[slot] =
        false;


    updateParkingState();

    updateSlotLEDs();

    updateOLED();


    Serial.print(
        "Vehicle removed from SLOT "
    );

    Serial.println(
        slot + 1
    );


    beepSuccess();

    openBarrier();

    delay(1200);

    closeBarrier();

    updateOLED();
}


// ============================================================
// OPEN BARRIER
// ============================================================

void openBarrier()
{
    Serial.println(
        "Barrier: OPEN"
    );

    barrierServo.write(
        BARRIER_OPEN_ANGLE
    );

    delay(250);
}


// ============================================================
// CLOSE BARRIER
// ============================================================

void closeBarrier()
{
    Serial.println(
        "Barrier: CLOSED"
    );

    barrierServo.write(
        BARRIER_CLOSED_ANGLE
    );

    delay(250);
}


// ============================================================
// SHORT BEEP
// ============================================================

void beepShort()
{
    digitalWrite(
        BUZZER_PIN,
        HIGH
    );

    delay(80);

    digitalWrite(
        BUZZER_PIN,
        LOW
    );
}


// ============================================================
// SUCCESS BEEP
// ============================================================

void beepSuccess()
{
    beepShort();

    delay(60);

    beepShort();
}


// ============================================================
// ERROR BEEP
// ============================================================

void beepError()
{
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(
            BUZZER_PIN,
            HIGH
        );

        delay(120);

        digitalWrite(
            BUZZER_PIN,
            LOW
        );

        delay(80);
    }
}


// ============================================================
// UPDATE SLOT LEDs
// ============================================================
//
// GREEN LED ON  = AVAILABLE
// GREEN LED OFF = FULL
//
// Your current diagram uses green LEDs.
// You can later add red LEDs if you want a two-color system.
// ============================================================

void updateSlotLEDs()
{
    digitalWrite(
        SLOT1_LED,
        slotOccupied[0]
            ? LOW
            : HIGH
    );

    digitalWrite(
        SLOT2_LED,
        slotOccupied[1]
            ? LOW
            : HIGH
    );

    digitalWrite(
        SLOT3_LED,
        slotOccupied[2]
            ? LOW
            : HIGH
    );

    digitalWrite(
        SLOT4_LED,
        slotOccupied[3]
            ? LOW
            : HIGH
    );
}


// ============================================================
// COUNT FREE SLOTS
// ============================================================

int countFreeSlots()
{
    int count = 0;

    for (int i = 0; i < 4; i++)
    {
        if (!slotOccupied[i])
        {
            count++;
        }
    }

    return count;
}


// ============================================================
// COUNT OCCUPIED SLOTS
// ============================================================

int countOccupiedSlots()
{
    int count = 0;

    for (int i = 0; i < 4; i++)
    {
        if (slotOccupied[i])
        {
            count++;
        }
    }

    return count;
}


// ============================================================
// FIND FIRST FREE SLOT
// ============================================================

int findFirstFreeSlot()
{
    for (int i = 0; i < 4; i++)
    {
        if (!slotOccupied[i])
        {
            return i;
        }
    }

    return -1;
}


// ============================================================
// FIND FIRST OCCUPIED SLOT
// ============================================================

int findFirstOccupiedSlot()
{
    for (int i = 0; i < 4; i++)
    {
        if (slotOccupied[i])
        {
            return i;
        }
    }

    return -1;
}


// ============================================================
// OLED DISPLAY
// ============================================================

void updateOLED()
{
    int freeSlots =
        countFreeSlots();

    int occupiedSlots =
        countOccupiedSlots();


    display.clearDisplay();

    display.setTextColor(
        SSD1306_WHITE
    );


    // --------------------------------------------------------
    // HEADER
    // --------------------------------------------------------

    display.setTextSize(1);

    display.setCursor(0, 0);

    display.println(
        "SMART PARKING"
    );


    // Separator
    display.drawLine(
        0,
        9,
        127,
        9,
        SSD1306_WHITE
    );


    // --------------------------------------------------------
    // SLOT 1 + SLOT 2
    // --------------------------------------------------------

    display.setCursor(
        0,
        13
    );

    display.print("S1:");

    display.print(
        slotOccupied[0]
            ? "FULL"
            : "FREE"
    );


    display.setCursor(
        65,
        13
    );

    display.print("S2:");

    display.print(
        slotOccupied[1]
            ? "FULL"
            : "FREE"
    );


    // --------------------------------------------------------
    // SLOT 3 + SLOT 4
    // --------------------------------------------------------

    display.setCursor(
        0,
        23
    );

    display.print("S3:");

    display.print(
        slotOccupied[2]
            ? "FULL"
            : "FREE"
    );


    display.setCursor(
        65,
        23
    );

    display.print("S4:");

    display.print(
        slotOccupied[3]
            ? "FULL"
            : "FREE"
    );


    // --------------------------------------------------------
    // Separator
    // --------------------------------------------------------

    display.drawLine(
        0,
        33,
        127,
        33,
        SSD1306_WHITE
    );


    // --------------------------------------------------------
    // CAPACITY
    // --------------------------------------------------------

    display.setCursor(
        0,
        37
    );

    display.print(
        "FREE: "
    );

    display.print(
        freeSlots
    );

    display.print(
        "/4"
    );


    display.setCursor(
        65,
        37
    );

    display.print(
        "FULL: "
    );

    display.print(
        occupiedSlots
    );


    // --------------------------------------------------------
    // SYSTEM STATUS
    // --------------------------------------------------------

    display.setCursor(
        0,
        49
    );


    if (freeSlots == 0)
    {
        display.print(
            "STATUS: PARKING FULL"
        );
    }
    else
    {
        display.print(
            "STATUS: SPACE AVAILABLE"
        );
    }


    display.display();
}


// ============================================================
// SERIAL MONITOR
// ============================================================

void printSerialStatus()
{
    Serial.println();
    Serial.println(
        "------------- PARKING STATUS -------------"
    );


    for (int i = 0; i < 4; i++)
    {
        Serial.print(
            "SLOT "
        );

        Serial.print(
            i + 1
        );

        Serial.print(
            " | Distance: "
        );

        Serial.print(
            slotDistance[i],
            1
        );

        Serial.print(
            " cm | "
        );

        Serial.println(
            slotOccupied[i]
                ? "FULL"
                : "FREE"
        );
    }


    Serial.print(
        "FREE SLOTS: "
    );

    Serial.println(
        countFreeSlots()
    );


    Serial.print(
        "FULL SLOTS: "
    );

    Serial.println(
        countOccupiedSlots()
    );


    Serial.println(
        "-------------------------------------------"
    );
}