/*
 * =============================================================================
 * CPR (Crowd Pressure Radar) — Hardware Test 09: Wi-Fi Cloud Polling Test
 * Board: ESP32 Dev Module
 * =============================================================================
 * 
 * WHAT IT DOES:
 * - Connects to your Wi-Fi hotspot.
 * - Polls your LIVE Render cloud backend endpoint:
 *   https://oaic-crowd.onrender.com/api/esp32/status
 * - Decodes the live crowd count, traffic LED state, and servo gate angle from the cloud!
 * 
 * REQUIRED LIBRARY:
 * - "ArduinoJson" by Benoit Blanchon (Version 6 or 7)
 * =============================================================================
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Enter your Wi-Fi hotspot details (Mobile hotspot or home Wi-Fi)
const char* WIFI_SSID = "Your_Hotspot_Name";
const char* WIFI_PASS = "Your_Hotspot_Password";

// Live Cloud URL on Render (Or replace with http://<your_laptop_ip>:3000/api/esp32/status)
const char* CLOUD_STATUS_URL = "https://oaic-crowd.onrender.com/api/esp32/status";

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n========================================================");
  Serial.println("  CPR ESP32 Test 09: Wi-Fi Cloud Polling Client");
  Serial.println("  Target: https://oaic-crowd.onrender.com/api/esp32/status");
  Serial.println("========================================================\n");

  Serial.print("[WIFI] Connecting to: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }

  Serial.println("\n[WIFI] Connected successfully!");
  Serial.print("[WIFI] ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(CLOUD_STATUS_URL);
    http.setTimeout(2500);

    int httpCode = http.GET();
    if (httpCode == 200) {
      String payload = http.getString();
      
      StaticJsonDocument<512> doc;
      DeserializationError err = deserializeJson(doc, payload);

      if (!err) {
        String mode = doc["mode"] | "UNKNOWN";
        int count = doc["count"] | 0;
        String led = doc["led"] | "OFF";
        int angle = doc["servoAngle"] | 90;
        String servoStatus = doc["servoStatus"] | "OPEN";

        Serial.println("--------------------------------------------------------");
        Serial.print("[CLOUD STATUS] Mode: "); Serial.println(mode);
        Serial.print("               Crowd Count: "); Serial.print(count); Serial.println(" PPL");
        Serial.print("               Traffic LED: "); Serial.println(led);
        Serial.print("               Servo Gate:  "); Serial.print(angle); Serial.print(" deg ("); Serial.print(servoStatus); Serial.println(")");
        
        if (doc.containsKey("oled")) {
          JsonObject oled = doc["oled"];
          Serial.print("               OLED Line 1: "); Serial.println(oled["line1"].as<String>());
          Serial.print("               OLED Line 2: "); Serial.println(oled["line2"].as<String>());
        }
      } else {
        Serial.println("[JSON ERROR] Failed to parse backend payload.");
      }
    } else {
      Serial.print("[HTTP ERROR] Status code: ");
      Serial.println(httpCode);
    }
    http.end();
  } else {
    Serial.println("[WIFI] Disconnected. Reconnecting...");
    WiFi.reconnect();
  }

  delay(1000); // Poll once per second
}
