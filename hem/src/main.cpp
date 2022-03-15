#include <Arduino.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <BluetoothSerial.h>
#include <ArduinoJson.h>
#include <WebServer.h>

extern "C" {
    #include "Esp.h"
}


// Global Parameters
WiFiManager wifiManager;
BluetoothSerial serialBT;
WebServer server(7070);

String createMetric(const char* type, const char* circuit, int value){
  char strBuf[50];
  sprintf(strBuf, "%s{circuit=\"%s\"} %d\n", type, circuit, value);
  return String(strBuf);
}

int i1, i2, i3, i4, i5, Vrms;
void handleMetrics(){

  String payload = createMetric("power", "grid", i1);
  payload += createMetric("power", "solar", i2);
  payload += createMetric("power", "waterimmersion", i3);
  payload += createMetric("power", "ashp", i4);
  payload += createMetric("power", "bufferimmersion", i5);
  payload += createMetric("voltage", "grid", Vrms);
  
  server.send(200, "text/plain", payload); 
}


// Setup
void setup(){
  // Serial
  Serial.begin(115200);  

  // // Setup WiFi
  WiFi.mode(WIFI_STA);
  wifiManager.setClass("invert"); // dark theme
  
  wifiManager.setHostname("HEM-ESP32");
  wifiManager.setConfigPortalTimeout(180);

  bool res = wifiManager.autoConnect(); // password protected ap
  if(!res) {
    Serial.println("Failed to connect or hit timeout");
    ESP.restart();
  } 

  // Web Server
  server.on("/metrics", handleMetrics);
  server.begin();

  Serial.println("SetupBT");
  serialBT.begin("HEM", true);

  uint8_t address[6]  = {0x98, 0xD3, 0x31, 0xF5, 0xC0, 0xFA};

  serialBT.setPin("1234");
  serialBT.enableSSP();
  bool connected = serialBT.connect(address);

  if(connected) {
      Serial.println("Connected Succesfully!");
    } else {
      while(!serialBT.connected(10000)) {
        Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app."); 
      }
    }
  serialBT.connect();
}

void loop(){

  if(serialBT.available()){
    String input = serialBT.readString();
    Serial.print(input);
    Serial.print("--\n");

    // Decode the JSON
    DynamicJsonDocument doc(1024);    
    auto error = deserializeJson(doc, input);
    if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        return;
    }

    // Put into the handler
    i1 = doc["i1"];
    i2 = doc["i2"];
    i3 = doc["i3"];
    i4 = doc["i4"];
    i5 = doc["i5"];
    Vrms = doc["Vrms"];
  }

  // Handle /metrics requests
  server.handleClient();
}