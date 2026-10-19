/*
  ESP32 DevKit V1 - Wi-Fi provisioning portal with long-press reset
  ----------------------------------------------------------------
  First boot:
    - Starts ESP32-Setup access point.
    - Captive portal lets the user select a scanned Wi-Fi network and enter password.
    - On successful connection, the setup AP is stopped.
  Later boots:
    - Reuses credentials saved by WiFiManager.
    - If saved Wi-Fi cannot connect, opens the setup portal again.
  Button:
    - Connect an external momentary push button between GPIO 27 and GND.
    - Hold for 3 seconds while running OR hold during startup to erase Wi-Fi
      credentials and reopen setup.

  Library: WiFiManager by tzapu
  Board: ESP32 Dev Module (ESP32 DevKit V1)
*/

#include <WiFi.h>
#include <WiFiManager.h>

static const char* AP_NAME = "ESP32-Setup";
static const char* AP_PASSWORD = "";  // Open setup AP; set 8+ chars to protect it.
static const uint8_t RESET_BUTTON_PIN = 27;
static const unsigned long LONG_PRESS_MS = 3000;
static const unsigned long DEBOUNCE_MS = 35;

WiFiManager wifiManager;

bool buttonWasDown = false;
bool longPressHandled = false;
unsigned long buttonDownAt = 0;
unsigned long lastDebounceAt = 0;
int lastRawButton = HIGH;
int stableButton = HIGH;

void startSetupPortal() {
  Serial.println();
  Serial.println("Starting Wi-Fi setup portal...");
  Serial.printf("Connect to Wi-Fi: %s\n", AP_NAME);
  Serial.println("If the setup page does not open automatically, browse to http://192.168.4.1");

  WiFi.mode(WIFI_STA);

  // Zero means the portal does not time out: it stays available until
  // credentials are submitted successfully or the device is restarted.
  wifiManager.setConfigPortalTimeout(0);
  wifiManager.setConnectTimeout(30);
  wifiManager.setCaptivePortalEnable(true);
  wifiManager.setConfigPortalBlocking(true);

  bool connected;
  if (strlen(AP_PASSWORD) >= 8) {
    connected = wifiManager.startConfigPortal(AP_NAME, AP_PASSWORD);
  } else {
    connected = wifiManager.startConfigPortal(AP_NAME);
  }

  if (connected && WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected successfully.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    Serial.println("Setup hotspot stopped.");
  } else {
    // With an unlimited portal timeout, this normally only happens if
    // the portal is interrupted by an unusual internal error.
    Serial.println("Wi-Fi setup did not complete. Restarting...");
    delay(1000);
    ESP.restart();
  }
}

void eraseWiFiAndRestart() {
  Serial.println("Long press detected. Erasing saved Wi-Fi credentials...");
  wifiManager.resetSettings();
  delay(300);
  ESP.restart();
}

bool wasButtonHeldAtBootForThreeSeconds() {
  // Allows the user to request Wi-Fi reset even if the ESP32 cannot connect
  // to its previously saved network. Hold the external button while powering
  // on/resetting and keep holding it for 3 seconds.
  if (digitalRead(RESET_BUTTON_PIN) != LOW) {
    return false;
  }

  unsigned long started = millis();
  while (millis() - started < LONG_PRESS_MS) {
    if (digitalRead(RESET_BUTTON_PIN) != LOW) {
      return false;
    }
    delay(10);
  }
  return true;
}

void connectOrProvisionWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  // If the button was held for 3 seconds during startup, clear credentials.
  if (wasButtonHeldAtBootForThreeSeconds()) {
    Serial.println("Startup long press detected.");
    wifiManager.resetSettings();
  }

  wifiManager.setConfigPortalTimeout(0);
  wifiManager.setConnectTimeout(25);
  wifiManager.setCaptivePortalEnable(true);

  Serial.println("Connecting to saved Wi-Fi or starting setup portal...");
  bool connected;

  if (strlen(AP_PASSWORD) >= 8) {
    connected = wifiManager.autoConnect(AP_NAME, AP_PASSWORD);
  } else {
    connected = wifiManager.autoConnect(AP_NAME);
  }

  if (!connected || WiFi.status() != WL_CONNECTED) {
    // autoConnect with portal timeout 0 should keep the portal running.
    // Restart as a last-resort fallback if it returns without a connection.
    Serial.println("Wi-Fi connection/setup failed. Restarting to retry...");
    delay(1500);
    ESP.restart();
  }

  Serial.println("Wi-Fi connected.");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Ensure the provisioning AP is no longer active after successful setup.
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
}

void serviceLongPressButton() {
  int raw = digitalRead(RESET_BUTTON_PIN);

  if (raw != lastRawButton) {
    lastDebounceAt = millis();
    lastRawButton = raw;
  }

  if (millis() - lastDebounceAt >= DEBOUNCE_MS) {
    if (raw != stableButton) {
      stableButton = raw;

      if (stableButton == LOW) {
        buttonWasDown = true;
        longPressHandled = false;
        buttonDownAt = millis();
      } else {
        buttonWasDown = false;
        longPressHandled = false;
      }
    }
  }

  if (buttonWasDown && !longPressHandled &&
      millis() - buttonDownAt >= LONG_PRESS_MS) {
    longPressHandled = true;
    eraseWiFiAndRestart();
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

  Serial.println();
  Serial.println("====================================");
  Serial.println(" ESP32 Wi-Fi Setup Portal");
  Serial.println("====================================");

  connectOrProvisionWiFi();
}

void loop() {
  serviceLongPressButton();

  // Your main application code can run here.
  // Keep long blocking operations out of loop() so the button remains responsive.

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Attempting reconnection...");
    WiFi.reconnect();
    delay(250);
  }

  delay(10);
}
