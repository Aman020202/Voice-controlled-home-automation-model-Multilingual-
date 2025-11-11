#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// WiFi Configuration - CHANGE THESE!
const char* ssid = "Redmi Note 10S";
const char* password = "diksha*123";

ESP8266WebServer server(80);

// Relay Pins: D1=5, D2=4, D3=0, D4=2
const int relayPins[] = {5, 4, 0, 2};
bool relayStates[] = {false, false, false, false};

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n🚀 Starting ESP8266 Voice Control System...");
  
  // Initialize relays
  for(int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH); // Start OFF
    Serial.println("✅ Relay " + String(i+1) + " initialized on GPIO " + String(relayPins[i]) + " (OFF)");
  }
  
  // Connect to WiFi
  connectToWiFi();
  
  // Setup server routes
  setupServerRoutes();
  
  server.begin();
  Serial.println("✅ HTTP Server Started");
  Serial.println("🌐 Ready for Python connection or browser control");
}

void loop() {
  server.handleClient();
}

void connectToWiFi() {
  Serial.println("📡 Connecting to WiFi network: " + String(ssid));
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(1000);
    Serial.print(".");
    attempts++;
    if (attempts % 5 == 0) Serial.println(" Still trying to connect...");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected!");
    Serial.println("🌐 IP Address: " + WiFi.localIP().toString());
    Serial.println("🔗 Subnet Mask: " + WiFi.subnetMask().toString());
    Serial.println("🌍 Gateway: " + WiFi.gatewayIP().toString());
    Serial.println("📶 Signal Strength: " + String(WiFi.RSSI()) + " dBm");
    Serial.println("💡 Ready for HTTP server connections");
  } else {
    Serial.println("\n❌ WiFi Connection Failed!");
    Serial.println("🔁 Restarting ESP...");
    delay(2000);
    ESP.restart();
  }
}

void setupServerRoutes() {
  // Enable CORS for Python requests
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
    html += "<p>Use Python server for voice control</p>";
    html += "<p>Endpoints: /control, /status, /bulk</p>";
    server.send(200, "text/html", html);
  });

  // Control individual relay - GET
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
      Serial.println("💡 Relay " + String(relayIndex+1) + " turned ON (GET)");
      server.send(200, "application/json", 
        "{\"success\":true,\"message\":\"Light " + String(relayIndex+1) + " ON\",\"relay\":" + String(relayIndex+1) + ",\"state\":\"on\"}");
    } 
    else if (action == "off") {
      digitalWrite(relayPins[relayIndex], HIGH);
      relayStates[relayIndex] = false;
      Serial.println("💡 Relay " + String(relayIndex+1) + " turned OFF (GET)");
      server.send(200, "application/json", 
        "{\"success\":true,\"message\":\"Light " + String(relayIndex+1) + " OFF\",\"relay\":" + String(relayIndex+1) + ",\"state\":\"off\"}");
    } 
    else {
      server.send(400, "application/json", "{\"error\":\"Action must be on/off\"}");
    }
  });

  // Control individual relay - POST
  server.on("/control", HTTP_POST, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    
    if (server.hasArg("plain")) {
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
          Serial.println("💡 Relay " + String(relay) + " turned ON (POST)");
          server.send(200, "application/json", "{\"success\":true,\"relay\":" + String(relay) + ",\"state\":\"on\"}");
        } else if (action == "off") {
          digitalWrite(relayPins[relayIndex], HIGH);
          relayStates[relayIndex] = false;
          Serial.println("💡 Relay " + String(relay) + " turned OFF (POST)");
          server.send(200, "application/json", "{\"success\":true,\"relay\":" + String(relay) + ",\"state\":\"off\"}");
        }
      }
    }
  });

  // Bulk control - POST
  server.on("/bulk", HTTP_POST, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    
    if (server.hasArg("plain")) {
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
        String actionStr = action["action"];
        int relayIndex = relay - 1;
        
        if (relayIndex >= 0 && relayIndex <= 3) {
          if (actionStr == "on") {
            digitalWrite(relayPins[relayIndex], LOW);
            relayStates[relayIndex] = true;
            Serial.println("💡 Relay " + String(relay) + " turned ON (BULK)");
            response += "{\"relay\":" + String(relay) + ",\"state\":\"on\",\"success\":true},";
          } else if (actionStr == "off") {
            digitalWrite(relayPins[relayIndex], HIGH);
            relayStates[relayIndex] = false;
            Serial.println("💡 Relay " + String(relay) + " turned OFF (BULK)");
            response += "{\"relay\":" + String(relay) + ",\"state\":\"off\",\"success\":true},";
          }
        }
      }
      
      if (response.endsWith(",")) response.remove(response.length()-1);
      response += "]}";
      
      server.send(200, "application/json", response);
    }
  });

  // Status endpoint
  server.on("/status", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    
    DynamicJsonDocument doc(512);
    for(int i = 0; i < 4; i++) {
      doc["light" + String(i+1)] = relayStates[i];
    }
    doc["ip"] = WiFi.localIP().toString();
    doc["wifi"] = WiFi.SSID();
    doc["signal"] = WiFi.RSSI();
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
}
