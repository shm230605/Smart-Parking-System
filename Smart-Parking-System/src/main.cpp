#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"

// ============================================================
// Smart Parking System - ESP32
// PlatformIO + Arduino framework
// ============================================================

Adafruit_SSD1306 display(128, 64, &Wire, -1);
WebServer server(WEB_PORT);

struct SlotState {
  bool occupied = false;
  bool pending = false;
  bool pendingDecision = false;
  uint32_t pendingSince = 0;
  float distanceCm = 999.0f;
};

SlotState slots[SLOT_COUNT];

bool displayReady = false;
bool wifiStationConnected = false;
bool apMode = false;

bool gateOpen = false;
uint32_t gateOpenedAt = 0;

uint32_t lastSensorScan = 0;
uint32_t lastOledUpdate = 0;
uint32_t lastSerialUpdate = 0;
uint32_t lastFullAlert = 0;

// -----------------------------
// Helpers
// -----------------------------

String slotLabel(uint8_t index) {
  return "S" + String(index + 1);
}

uint8_t availableSlots() {
  uint8_t freeCount = 0;
  for (uint8_t i = 0; i < SLOT_COUNT; ++i) {
    if (!slots[i].occupied) {
      ++freeCount;
    }
  }
  return freeCount;
}

// -----------------------------
// Ultrasonic sensor
// -----------------------------

float readDistanceOnce(uint8_t trigPin, uint8_t echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long pulseUs = pulseIn(echoPin, HIGH, ECHO_TIMEOUT_US);

  if (pulseUs == 0) {
    return NAN;
  }

  float cm = static_cast<float>(pulseUs) / 58.0f;

  if (cm < 2.0f || cm > 400.0f) {
    return NAN;
  }

  return cm;
}

float averageValid(float values[], uint8_t count) {
  // Simple insertion sort.
  for (uint8_t i = 1; i < count; ++i) {
    float key = values[i];
    int8_t j = static_cast<int8_t>(i) - 1;

    while (j >= 0 && values[j] > key) {
      values[j + 1] = values[j];
      --j;
    }
    values[j + 1] = key;
  }

  // Trim one low and one high value when enough valid samples exist.
  uint8_t start = (count >= 5) ? 1 : 0;
  uint8_t end = (count >= 5) ? count - 1 : count;

  float sum = 0.0f;
  uint8_t used = 0;

  for (uint8_t i = start; i < end; ++i) {
    sum += values[i];
    ++used;
  }

  return used ? sum / used : NAN;
}

float readFilteredDistance(uint8_t index) {
  float values[SENSOR_SAMPLES];
  uint8_t validCount = 0;

  for (uint8_t i = 0; i < SENSOR_SAMPLES; ++i) {
    float cm = readDistanceOnce(TRIG_PINS[index], ECHO_PINS[index]);

    if (!isnan(cm)) {
      values[validCount++] = cm;
    }

    delay(4);
  }

  if (validCount < 2) {
    return 999.0f;
  }

  float result = averageValid(values, validCount);
  return isnan(result) ? 999.0f : result;
}

// -----------------------------
// LEDs and slot state
// -----------------------------

void updateSlotLeds(uint8_t index) {
  if (slots[index].occupied) {
    digitalWrite(GREEN_LED_PINS[index], LOW);
    digitalWrite(RED_LED_PINS[index], HIGH);
  } else {
    digitalWrite(GREEN_LED_PINS[index], HIGH);
    digitalWrite(RED_LED_PINS[index], LOW);
  }
}

void updateSlotState(uint8_t index, float distanceCm) {
  slots[index].distanceCm = distanceCm;

  // Invalid/no-echo readings are treated as FREE for safety.
  bool decision = distanceCm < OCCUPIED_THRESHOLD_CM[index];

  if (decision == slots[index].occupied) {
    slots[index].pending = false;
    return;
  }

  if (!slots[index].pending || slots[index].pendingDecision != decision) {
    slots[index].pending = true;
    slots[index].pendingDecision = decision;
    slots[index].pendingSince = millis();
    return;
  }

  if (millis() - slots[index].pendingSince >= STATE_DEBOUNCE_MS) {
    slots[index].occupied = decision;
    slots[index].pending = false;
    updateSlotLeds(index);
  }
}

// -----------------------------
// Servo gate - ESP32 Arduino core 3.x LEDC API
// -----------------------------

uint32_t servoDutyFromAngle(uint8_t angle) {
  // 0.5 ms to 2.5 ms pulse width over a 20 ms period.
  const float pulseUs = 500.0f + (static_cast<float>(angle) / 180.0f) * 2000.0f;
  const float maxDuty = static_cast<float>((1UL << SERVO_RESOLUTION_BITS) - 1UL);
  return static_cast<uint32_t>((pulseUs / 20000.0f) * maxDuty);
}

void setGateAngle(uint8_t angle) {
  angle = constrain(angle, 0, 180);
  ledcWrite(SERVO_PIN, servoDutyFromAngle(angle));
  gateOpen = (angle > 45);
}

void openGate() {
  if (availableSlots() == 0) {
    return;
  }

  setGateAngle(GATE_OPEN_ANGLE);
  gateOpenedAt = millis();
}

void closeGate() {
  setGateAngle(GATE_CLOSED_ANGLE);
}

void serviceGate() {
  if (gateOpen && millis() - gateOpenedAt >= GATE_OPEN_TIME_MS) {
    closeGate();
  }
}

// -----------------------------
// Buzzer
// -----------------------------

void beepFullAlert() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(120);
  digitalWrite(BUZZER_PIN, LOW);
  delay(120);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(120);
  digitalWrite(BUZZER_PIN, LOW);
}

// -----------------------------
// OLED
// -----------------------------

void drawOLED() {
  if (!displayReady) {
    return;
  }

  uint8_t freeCount = availableSlots();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SMART PARKING SYSTEM");

  display.setTextSize(2);
  display.setCursor(0, 12);
  display.print("FREE ");
  display.print(freeCount);
  display.print("/");
  display.println(SLOT_COUNT);

  display.setTextSize(1);
  display.setCursor(0, 34);

  for (uint8_t i = 0; i < SLOT_COUNT; ++i) {
    display.print(slotLabel(i));
    display.print(":");
    display.print(slots[i].occupied ? "OCC " : "FREE ");
  }

  display.setCursor(0, 48);

  if (freeCount == 0) {
    display.println("STATUS: PARKING FULL");
  } else if (gateOpen) {
    display.println("STATUS: GATE OPEN");
  } else {
    display.println("STATUS: AVAILABLE");
  }

  display.display();
}

// -----------------------------
// Serial output
// -----------------------------

void printStatus() {
  Serial.println();
  Serial.println("================================");
  Serial.println(" SMART PARKING SYSTEM");
  Serial.println("================================");

  for (uint8_t i = 0; i < SLOT_COUNT; ++i) {
    Serial.print(slotLabel(i));
    Serial.print(" | Distance: ");
    Serial.print(slots[i].distanceCm, 1);
    Serial.print(" cm | ");
    Serial.println(slots[i].occupied ? "OCCUPIED" : "FREE");
  }

  Serial.print("Available slots: ");
  Serial.print(availableSlots());
  Serial.print("/");
  Serial.println(SLOT_COUNT);

  Serial.print("Gate: ");
  Serial.println(gateOpen ? "OPEN" : "CLOSED");

  if (apMode) {
    Serial.print("AP address: http://");
    Serial.println(WiFi.softAPIP());
  } else if (wifiStationConnected) {
    Serial.print("Web address: http://");
    Serial.println(WiFi.localIP());
  }

  Serial.println("================================");
}

// -----------------------------
// Wi-Fi
// -----------------------------

void startWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");

  uint32_t started = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - started < 10000) {
    delay(250);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    wifiStationConnected = true;
    apMode = false;

    Serial.print("Wi-Fi connected. IP: ");
    Serial.println(WiFi.localIP());
    return;
  }

  Serial.println("Station connection failed.");
  Serial.println("Starting fallback access point...");

  WiFi.disconnect(true);
  delay(100);

  WiFi.mode(WIFI_AP);
  apMode = WiFi.softAP(AP_SSID, AP_PASSWORD);

  if (apMode) {
    Serial.print("AP started. IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("ERROR: Could not start AP.");
  }
}

// -----------------------------
// Web UI
// -----------------------------

String makeSlotCards() {
  String html;

  for (uint8_t i = 0; i < SLOT_COUNT; ++i) {
    bool occupied = slots[i].occupied;

    html += "<div class='card ";
    html += occupied ? "occ" : "free";
    html += "'>";

    html += "<div class='slot-title'>";
    html += slotLabel(i);
    html += "</div>";

    html += "<div class='state'>";
    html += occupied ? "OCCUPIED" : "FREE";
    html += "</div>";

    html += "<div class='distance'>";
    html += String(slots[i].distanceCm, 1);
    html += " cm</div>";

    html += "</div>";
  }

  return html;
}

String webPage() {
  uint8_t freeCount = availableSlots();

  String html = R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta http-equiv="refresh" content="3">
<title>Smart Parking</title>
<style>
*{box-sizing:border-box}
body{margin:0;font-family:Arial,sans-serif;background:#0b1020;color:#f4f7fb}
.wrap{max-width:1000px;margin:auto;padding:28px}
.header{background:linear-gradient(135deg,#18213d,#10172c);padding:26px;border-radius:18px;border:1px solid #2d395c}
h1{margin:0 0 8px;font-size:30px}
.sub{color:#aab6d4}
.summary{display:flex;gap:16px;flex-wrap:wrap;margin:20px 0}
.metric{flex:1;min-width:200px;background:#121a31;border:1px solid #2d395c;border-radius:16px;padding:20px}
.metric b{display:block;font-size:38px;margin-top:8px}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(190px,1fr));gap:16px}
.card{padding:22px;border-radius:16px;border:2px solid}
.free{border-color:#22c55e;background:#0d2518}
.occ{border-color:#ef4444;background:#2b1015}
.slot-title{font-size:20px;font-weight:bold}
.state{font-size:24px;font-weight:bold;margin:12px 0}
.distance{color:#b7c2da}
.actions{margin-top:20px;display:flex;gap:12px;flex-wrap:wrap}
button{border:0;border-radius:12px;padding:13px 18px;font-weight:bold;cursor:pointer}
.open{background:#22c55e;color:#041108}
.close{background:#ef4444;color:white}
.footer{margin-top:22px;color:#7f8baa;font-size:13px}
</style>
</head>
<body>
<div class="wrap">
<div class="header">
<h1>SMART PARKING SYSTEM</h1>
<div class="sub">ESP32 real-time parking monitoring dashboard</div>
</div>

<div class="summary">
<div class="metric">Available slots<b>)HTML";

  html += String(freeCount);

  html += R"HTML(</b></div>
<div class="metric">Total slots<b>)HTML";

  html += String(SLOT_COUNT);

  html += R"HTML(</b></div>
<div class="metric">Gate<b>)HTML";

  html += gateOpen ? "OPEN" : "CLOSED";

  html += R"HTML(</b></div>
</div>

<div class="grid">)HTML";

  html += makeSlotCards();

  html += R"HTML(</div>

<div class="actions">
<button class="open" onclick="fetch('/api/gate/open').then(()=>location.reload())">OPEN GATE</button>
<button class="close" onclick="fetch('/api/gate/close').then(()=>location.reload())">CLOSE GATE</button>
</div>

<div class="footer">Auto refresh: 3 seconds</div>
</div>
</body>
</html>
)HTML";

  return html;
}

void handleRoot() {
  server.send(200, "text/html", webPage());
}

void handleApiStatus() {
  String json = "{";
  json += "\"available\":" + String(availableSlots()) + ",";
  json += "\"total\":" + String(SLOT_COUNT) + ",";
  json += "\"gateOpen\":" + String(gateOpen ? "true" : "false") + ",";
  json += "\"slots\":[";

  for (uint8_t i = 0; i < SLOT_COUNT; ++i) {
    if (i > 0) {
      json += ",";
    }

    json += "{";
    json += "\"id\":" + String(i + 1) + ",";
    json += "\"occupied\":" + String(slots[i].occupied ? "true" : "false") + ",";
    json += "\"distanceCm\":" + String(slots[i].distanceCm, 1);
    json += "}";
  }

  json += "]}";

  server.send(200, "application/json", json);
}

void handleGateOpen() {
  if (availableSlots() == 0) {
    server.send(409, "text/plain", "PARKING FULL - gate remains closed");
    return;
  }

  openGate();
  server.send(200, "text/plain", "Gate opened");
}

void handleGateClose() {
  closeGate();
  server.send(200, "text/plain", "Gate closed");
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleApiStatus);
  server.on("/api/gate/open", HTTP_GET, handleGateOpen);
  server.on("/api/gate/close", HTTP_GET, handleGateClose);
  server.begin();

  Serial.println("Web server started.");
}

// -----------------------------
// Setup
// -----------------------------

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println();
  Serial.println("================================");
  Serial.println(" SMART PARKING SYSTEM BOOT");
  Serial.println(" ESP32 + PlatformIO");
  Serial.println("================================");

  // GPIO setup
  for (uint8_t i = 0; i < SLOT_COUNT; ++i) {
    pinMode(TRIG_PINS[i], OUTPUT);
    pinMode(ECHO_PINS[i], INPUT);

    pinMode(GREEN_LED_PINS[i], OUTPUT);
    pinMode(RED_LED_PINS[i], OUTPUT);

    digitalWrite(TRIG_PINS[i], LOW);
    digitalWrite(GREEN_LED_PINS[i], LOW);
    digitalWrite(RED_LED_PINS[i], LOW);

    slots[i] = SlotState{};
    slots[i].distanceCm = 999.0f;

    updateSlotLeds(i);
  }

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);

  displayReady = display.begin(
    SSD1306_SWITCHCAPVCC,
    OLED_ADDRESS
  );

  if (displayReady) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("SMART PARKING");
    display.println();
    display.println("Initializing...");
    display.display();
  } else {
    Serial.println("WARNING: OLED not detected.");
  }

  // Servo: Arduino-ESP32 3.x LEDC API.
  // The old ledcSetup()/ledcAttachPin() API is intentionally not used.
  if (!ledcAttachChannel(
        SERVO_PIN,
        SERVO_FREQ_HZ,
        SERVO_RESOLUTION_BITS,
        SERVO_CHANNEL)) {
    Serial.println("ERROR: Servo LEDC attach failed.");
  }

  closeGate();

  startWiFi();
  setupWebServer();

  drawOLED();
  printStatus();

  Serial.println("SYSTEM READY");
}

void loop() {
  server.handleClient();
  serviceGate();

  // Scan sensors at a controlled interval.
  if (millis() - lastSensorScan >= SENSOR_SCAN_INTERVAL_MS) {
    lastSensorScan = millis();

    // Read one sensor per scan. This avoids blocking all four sensors at once.
    static uint8_t sensorIndex = 0;

    float distance = readFilteredDistance(sensorIndex);
    updateSlotState(sensorIndex, distance);

    ++sensorIndex;
    if (sensorIndex >= SLOT_COUNT) {
      sensorIndex = 0;
    }
  }

  // OLED refresh.
  if (millis() - lastOledUpdate >= OLED_UPDATE_MS) {
    lastOledUpdate = millis();
    drawOLED();
  }

  // Serial status.
  if (millis() - lastSerialUpdate >= SERIAL_UPDATE_MS) {
    lastSerialUpdate = millis();
    printStatus();
  }

  // Parking-full alert.
  if (availableSlots() == 0 &&
      !gateOpen &&
      millis() - lastFullAlert >= FULL_ALERT_INTERVAL_MS) {
    lastFullAlert = millis();
    beepFullAlert();
  }
}
