#include "HX711.h"
#include <HCSR04.h>
#include <ESP8266WiFi.h>;
#include <WiFiClient.h>;
#include <ThingSpeak.h>;
#include <Adafruit_Sensor.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 14  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int fsrAnalogPin = A0; // D14 FSR is connected to analog 0
int LEDpin = 0;       //S2
int SWpin = 2;         //D4

int DTpin =  5;        //D1 dataPin
int SCKpin = 4;        //D2 clockPin

uint32_t start, stop;
volatile float f;


int fsrReading;       // the analog reading from the FSR resistor divider
int LEDbrightness;
int SWValue = 0;

UltraSonicDistanceSensor distanceSensor(13, 12);  // Initialize sensor that uses digital pins D6 and D7.

HX711 scale;

void setup() {
  Serial.begin(115200);   // We'll send debugging information via the Serial monitor
  pinMode(LEDpin, OUTPUT);
  pinMode(SWpin, INPUT_PULLUP);
  
  scale.begin(DTpin, SCKpin);
  // TODO find a nice solution for this calibration..
  // loadcell factor 20 KG
  // scale.set_scale(127.15);
  // loadcell factor 5 KG
  scale.set_scale(420.0983);
  // reset the scale to zero = 0
  scale.tare();
  
  sensors.begin();
 
}
 
void loop(void) {
  fsrReading = analogRead(fsrAnalogPin);
  Serial.print("Force Sensetive Resistor = ");
  Serial.println(fsrReading);
  // we'll need to change the range from the analog reading (0-1023) down to the range
  // used by analogWrite (0-255) with map!
  LEDbrightness = map(fsrReading, 0, 1023, 0, 255);
  // LED gets brighter the harder you press
  analogWrite(LEDpin, LEDbrightness);
  SWValue = digitalRead(SWpin);  
  Serial.print("SW = ");
  Serial.println(SWValue);
  
  Serial.print("measureDistanceCm = ");
  Serial.println(distanceSensor.measureDistanceCm());


  // continuous scale 4x per second
  f = scale.get_units(), 10;

  
  Serial.print("measureWeight = ");
  Serial.println(f);

  
  sensors.requestTemperatures();
  Serial.print("Temperature for Device 1 is: ");
  Serial.println(sensors.getTempCByIndex(0));
  
  delay(2000);
}
