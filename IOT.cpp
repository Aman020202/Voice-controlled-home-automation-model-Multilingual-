#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// ==================== WiFi Configuration ====================
const char* ssid = "Prajapati-Ji";
const char* password = "12341234";

// ==================== Server & Relays ====================
ESP8266WebServer server(80);
const int relayPins[] = {5, 4, 0, 2}; // D1, D2, D3, D4
bool relayStates[] = {false, false, false, false};

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n============================================");
  Serial.println("Starting ESP8266 Voice Control System...");
  Serial.println("============================================");

  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);  // OFF by default
    Serial.println("Relay " + String(i + 1) + " initialized on GPIO " + String(relayPins[i]) + " (OFF)");
  }

  connectToWiFi();
  setupServerRoutes();

  server.begin();
  Serial.println("HTTP Server Started Successfully!");
  Serial.println("Access Control Panel at: http://" + WiFi.localIP().toString());
  Serial.println("Ready for Python or Browser control.");
  Serial.println("============================================\n");
}

// ==================== Loop ====================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    reconnectWiFi();
  }
  server.handleClient();
}

// ==================== WiFi Connection ====================
void connectToWiFi() {
  Serial.println("\nConnecting to WiFi...");
  Serial.println("SSID: " + String(ssid));

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    printWiFiDetails();
  } else {
    Serial.println("WiFi not connected after 20 attempts!");
    Serial.println("Check SSID/Password or Signal strength.");
  }
}

// ==================== Auto Reconnect ====================
void reconnectWiFi() {
  static unsigned long lastAttemptTime = 0;
  unsigned long now = millis();
  if (now - lastAttemptTime > 10000) { // every 10 seconds
    lastAttemptTime = now;
    Serial.println("WiFi Disconnected! Attempting reconnection...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
  }
}

// ==================== WiFi Info Printer ====================
void printWiFiDetails() {
  Serial.println("WiFi Connected Successfully!");
  Serial.println("--------------------------------------------");
  Serial.println("SSID: " + String(WiFi.SSID()));
  Serial.println("IP Address: " + WiFi.localIP().toString());
  Serial.println("Gateway: " + WiFi.gatewayIP().toString());
  Serial.println("Subnet: " + WiFi.subnetMask().toString());
  Serial.println("DNS: " + WiFi.dnsIP().toString());
  Serial.println("Signal Strength (RSSI): " + String(WiFi.RSSI()) + " dBm");
  Serial.println("MAC Address: " + WiFi.macAddress());
  Serial.println("--------------------------------------------");
}

// ==================== Server Routes ====================
void setupServerRoutes() {
  // Enable CORS
  server.onNotFound([]() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(404, "application/json", "{\"error\":\"Endpoint not found\"}");
  });

  // Root endpoint
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    String html = "<h1>ESP8266 Voice Control API</h1>";
    html += "<p>Use Python or Browser to control relays.</p>";
    html += "<p>Endpoints: /control, /status, /bulk</p>";
    server.send(200, "text/html", html);
  });

  // Control single relay (GET)
  server.on("/control", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    String relayStr = server.arg("relay");
    String action = server.arg("action");

    if (relayStr == "" || action == "") {
      server.send(400, "application/json", "{\"error\":\"Missing parameters\"}");
      return;
    }

    int relayIndex = relayStr.toInt() - 1;
    if (relayIndex < 0 || relayIndex > 3) {
      server.send(400, "application/json", "{\"error\":\"Relay must be 1-4\"}");
      return;
    }

    if (action == "on") {
      digitalWrite(relayPins[relayIndex], LOW);
      relayStates[relayIndex] = true;
      Serial.println("Light " + String(relayIndex + 1) + " turned ON via HTTP");
      server.send(200, "application/json",
                  "{\"success\":true,\"relay\":" + String(relayIndex + 1) + ",\"state\":\"on\"}");
    } else if (action == "off") {
      digitalWrite(relayPins[relayIndex], HIGH);
      relayStates[relayIndex] = false;
      Serial.println("Light " + String(relayIndex + 1) + " turned OFF via HTTP");
      server.send(200, "application/json",
                  "{\"success\":true,\"relay\":" + String(relayIndex + 1) + ",\"state\":\"off\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"Action must be on/off\"}");
    }
  });

  // Control single relay (POST JSON)
  server.on("/control", HTTP_POST, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"error\":\"No body found\"}");
      return;
    }

    String body = server.arg("plain");
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }

    int relay = doc["relay"];
    String action = doc["action"];
    int relayIndex = relay - 1;

    if (relayIndex >= 0 && relayIndex <= 3) {
      if (action == "on") {
        digitalWrite(relayPins[relayIndex], LOW);
        relayStates[relayIndex] = true;
        Serial.println("Light " + String(relay) + " turned ON via JSON");
        server.send(200, "application/json", "{\"success\":true,\"relay\":" + String(relay) + ",\"state\":\"on\"}");
      } else if (action == "off") {
        digitalWrite(relayPins[relayIndex], HIGH);
        relayStates[relayIndex] = false;
        Serial.println("Light " + String(relay) + " turned OFF via JSON");
        server.send(200, "application/json", "{\"success\":true,\"relay\":" + String(relay) + ",\"state\":\"off\"}");
      } else {
        server.send(400, "application/json", "{\"error\":\"Action must be on/off\"}");
      }
    } else {
      server.send(400, "application/json", "{\"error\":\"Invalid relay number\"}");
    }
  });

  // Bulk control (POST JSON array)
  server.on("/bulk", HTTP_POST, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");

    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"error\":\"No body found\"}");
      return;
    }

    String body = server.arg("plain");
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }

    JsonArray actions = doc.as<JsonArray>();
    String response = "{\"results\":[";
    for (JsonObject action : actions) {
      int relay = action["relay"];
      String act = action["action"];
      int relayIndex = relay - 1;

      if (relayIndex >= 0 && relayIndex <= 3) {
        if (act == "on") {
          digitalWrite(relayPins[relayIndex], LOW);
          relayStates[relayIndex] = true;
          Serial.println("Light " + String(relay) + " turned ON via BULK");
          response += "{\"relay\":" + String(relay) + ",\"state\":\"on\"},";
        } else if (act == "off") {
          digitalWrite(relayPins[relayIndex], HIGH);
          relayStates[relayIndex] = false;
          Serial.println("Light " + String(relay) + " turned OFF via BULK");
          response += "{\"relay\":" + String(relay) + ",\"state\":\"off\"},";
        }
      }
    }

    if (response.endsWith(",")) response.remove(response.length() - 1);
    response += "]}";
    server.send(200, "application/json", response);
  });

  // Status endpoint
  server.on("/status", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    DynamicJsonDocument doc(512);
    for (int i = 0; i < 4; i++) {
      doc["light" + String(i + 1)] = relayStates[i];
    }
    doc["ip"] = WiFi.localIP().toString();
    doc["wifi"] = WiFi.SSID();
    doc["signal"] = WiFi.RSSI();
    doc["mac"] = WiFi.macAddress();

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
}
