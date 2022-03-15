#include "EmonLib.h"             // Include Emon Library
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

EnergyMonitor ct1, ct2, ct3, ct4, ct5;
boolean CT1, CT2, CT3, CT4, CT5;
boolean LEDToggle = false;

const float Ical = 45.98; //37.19
const float Vcal = 103.2;
const float Pshift = 1.7;
const int numHalfWavelengths = 300;
const int sampleTimeout = 2000;
const int delayBetweenReadings = 10000;


typedef struct {
  int i1, i2, i3, i4, i5, Vrms;
} PayloadTX;

PayloadTX payload;


// Software Serial for BT
SoftwareSerial Bluetooth(2, 3); // RX, TX

// JSON
StaticJsonDocument<200> jsonDoc;

void setup()
{
  Serial.begin(9600);
  Bluetooth.begin(9600);
  pinMode(13, OUTPUT); // Setup onboard LED

  // Setup current reading
  ct1.current(A1, Ical);
  ct2.current(A2, Ical);
  ct3.current(A3, Ical);
  ct4.current(A4, Ical);
  ct5.current(A5, Ical);

  // Setup Voltage
  ct1.voltage(A0, Vcal, Pshift);
  ct2.voltage(A0, Vcal, Pshift);
  ct3.voltage(A0, Vcal, Pshift);
  ct4.voltage(A0, Vcal, Pshift);
  ct5.voltage(A0, Vcal, Pshift);

}

void loop()
{

  // Flash the LED on every other loop
  LEDToggle = !LEDToggle;
  digitalWrite(13, LEDToggle);

  // Measure each CT
  ct1.calcVI(numHalfWavelengths, sampleTimeout);
  payload.i1 = ct1.realPower;
  payload.Vrms = ct1.Vrms;
  // Use this to get AMPS instead of WATTS
  //payload.i1 = ct1.calcIrms(numHalfWavelengths);

  ct2.calcVI(numHalfWavelengths, sampleTimeout);
  payload.i2 = ct2.realPower;

  ct3.calcVI(numHalfWavelengths, sampleTimeout);
  payload.i3 = ct3.realPower;

  ct4.calcVI(numHalfWavelengths, sampleTimeout);
  payload.i4 = ct4.realPower;

  ct5.calcVI(numHalfWavelengths, sampleTimeout);
  payload.i5 = ct5.realPower;

  // Output to console
  Serial.print("ct1:"); Serial.print(payload.i1);
  Serial.print(",ct2:"); Serial.print(payload.i2);
  Serial.print(",ct3:"); Serial.print(payload.i3);
  Serial.print(",ct4:"); Serial.print(payload.i4);
  Serial.print(",ct5:"); Serial.print(payload.i5);
  Serial.print(",vrms:"); Serial.print(payload.Vrms);
  Serial.println();

  // Format the data for Bluetooth output
  jsonDoc["i1"] = payload.i1;
  jsonDoc["i2"] = payload.i2;
  jsonDoc["i3"] = payload.i3;
  jsonDoc["i4"] = payload.i4;
  jsonDoc["i5"] = payload.i5;
  jsonDoc["Vrms"] = payload.Vrms;

  // Send the data
  serializeJson(jsonDoc, Bluetooth);
  Bluetooth.println();
  
  // Sleep until next reading
  delay(delayBetweenReadings);
  
}
