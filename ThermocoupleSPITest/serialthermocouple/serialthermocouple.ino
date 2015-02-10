/*************************************************** 
  This is an example for the Adafruit Thermocouple Sensor w/MAX31855K

  Designed specifically to work with the Adafruit Thermocouple Sensor
  ----> https://www.adafruit.com/products/269

  These displays use SPI to communicate, 3 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <SPI.h>
#include "Adafruit_MAX31855.h"

// Default connection is using software SPI, but comment and uncomment one of
// the two examples below to switch between software SPI and hardware SPI:

// Example creating a thermocouple instance with software SPI on any three
// digital IO pins.
#define SELA 4
#define SELB 5
#define SELC 6
#define CS 3

#define ENABLE_THERMOCOUPLE 7

Adafruit_MAX31855 thermocouple(ENABLE_THERMOCOUPLE);

// Example creating a thermocouple instance with hardware SPI (Uno/Mega only)
// on a given CS pin.
//#define CS   10
//Adafruit_MAX31855 thermocouple(CS);

double readThermcouple(int num);

void setup() {
  Serial.begin(9600);
  
  Serial.println("MAX31855 test");
  // wait for MAX chip to stabilize
  delay(500);
  
  pinMode(ENABLE_THERMOCOUPLE,OUTPUT);
  digitalWrite(ENABLE_THERMOCOUPLE,HIGH);
 
  pinMode(SELA,OUTPUT);
  pinMode(SELB,OUTPUT);
  pinMode(SELC,OUTPUT);
  
  digitalWrite(SELA,LOW);
  digitalWrite(SELB,LOW);
  digitalWrite(SELC,LOW);
}

void loop() {
   readThermocouple(0);
   delay(1000);
}

double readThermocouple(int num)
{
   digitalWrite(ENABLE_THERMOCOUPLE,LOW);
   // basic readout test, just print the current temp
   Serial.print("Internal Temp = ");
   Serial.println(thermocouple.readInternal());
   
   double c = thermocouple.readCelsius();
   Serial.println(c);
   if (isnan(c)) {
     Serial.println("Something wrong with thermocouple!");
   } else {
     Serial.print("C = "); 
     Serial.println(c);
   }
   digitalWrite(ENABLE_THERMOCOUPLE,HIGH);
}

