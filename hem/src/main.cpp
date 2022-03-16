#include <Arduino.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <BluetoothSerial.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>


extern "C" {
    #include "Esp.h"
}

#define ONE_WIRE_BUS 14

// Global Parameters
WiFiManager wifiManager;
BluetoothSerial serialBT;
AsyncWebServer server(7070);

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int i1, i2, i3, i4, i5, Vrms;
bool connectedStatus = false;
uint loopCounter = 0;


String createPwrMetric(const char* type, const char* circuit, int value){
  char strBuf[50];
  sprintf(strBuf, "%s{circuit=\"%s\"} %d\n", type, circuit, value);
  return String(strBuf);
}

String createTempMetric(const char* sensorId, float value){
  char strBuf[50];
  sprintf(strBuf, "temperature{sensorId=\"%s\"} %.2f\n", sensorId, value);
  return String(strBuf);
}

String createStatusMetrics(){
  char strBuf[60];
  sprintf(strBuf, "btConnected{} %d\niterations{} %d\n", connectedStatus, loopCounter);
  return String(strBuf);
}

String formatSensorId(DeviceAddress deviceAddress){
  String adm;
  for (uint8_t i = 0; i < 8; i++){  
    adm = adm + String(deviceAddress[i], HEX);
  }
  return adm;
}

#define MAX_SENSORS 10

uint8_t address[6]  = {0x98, 0xD3, 0x31, 0xF5, 0xC0, 0xFA};

struct DeviceInfo {
  DeviceAddress address;
  String addressStr;
  float lastReading;
};

DeviceInfo oneWireDevices[MAX_SENSORS] = {};
int actualSensorCount = 0;

// Setup
void setup(){
  // Serial
  Serial.begin(115200);  

  // Discover the one-wire devices
  // Bug Workaround: https://github.com/PaulStoffregen/OneWire/issues/57
  delay(500);
  sensors.begin();
  delay(1000);
  sensors.begin();
  
  Serial.printf("Dallas count: %d\n", sensors.getDeviceCount());

  for(int i =0; i<MAX_SENSORS; i++){
    if(!sensors.getAddress(oneWireDevices[i].address, i)){
      break;
    }
    oneWireDevices[i].addressStr = formatSensorId(oneWireDevices[i].address);
    actualSensorCount++;
  }

  Serial.printf("Found %d sensors!\n", actualSensorCount);

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
  server.on("/metrics", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String payload = createStatusMetrics();
    if(connectedStatus){  // Only send BT data if it's currently active
      payload += createPwrMetric("power", "grid", i1);
      payload += createPwrMetric("power", "solar", i2);
      payload += createPwrMetric("power", "waterImmersion", i3);
      payload += createPwrMetric("power", "ashp", i4);
      payload += createPwrMetric("power", "bufferImmersion", i5);
      payload += createPwrMetric("voltage", "grid", Vrms);
    }

    // Add the Temp Sensors
    for(int i=0; i<actualSensorCount; i++){
      float reading = oneWireDevices[i].lastReading;
      // Sanity check so we don't emit error codes
      // This will also highlight any BUS issues
      if(reading > 5){  
        payload += createTempMetric(oneWireDevices[i].addressStr.c_str(), reading);
      }else{
        Serial.println(reading);
      }
    }

    request->send(200, "text/plain", payload);
  });
  server.begin();

  Serial.println("SetupBT");
  serialBT.begin("HEM", true);
  serialBT.setPin("1234");
  serialBT.enableSSP();
  bool connected = serialBT.connect(address);

  if(connected) {
      Serial.println("Connected Succesfully!");
    } else {
      if(!serialBT.connected(2000)) {
        Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app."); 
        ESP.restart();  // Reboot seems to be more reliable 
      }
    }
  serialBT.connect();
}

void loop(){

  connectedStatus = serialBT.connected(1000);
  loopCounter++;

  // Non-blocking Bluetooth processing
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

  // Request One Wire Temps
  sensors.requestTemperatures();
  for(int i=0; i<actualSensorCount; i++){
    oneWireDevices[i].lastReading = sensors.getTempC(oneWireDevices[i].address);
  }


  delay(5000);  
}