// Test sketch for DS18B20/DS18S20 temperature sensors thrown together in GeekLabs on 2015-11-19.
//
// Reads temperature of sensors present on an Arduino GPIO as a OneWire bus and prints the temperature on the Arduino serial console.
// Based on https://tushev.org/articles/arduino/10/how-it-works-ds18b20-and-arduino
//
// DS18B20 is the "Programmable Resolution 1-Wire Digital Thermometer" from Maxim former Dallas.
// Product homepage: https://www.maximintegrated.com/en/products/analog/sensors-and-sensor-interface/DS18B20.html
// PDF datasheet: https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
//
// Dependencies
// Arduino OneWire library: https://github.com/PaulStoffregen/OneWire


#include <OneWire.h>

#define PIN 3

#define DS18S20_ID 0x10
#define DS18B20_ID 0x28

OneWire ds(PIN);
float temp;
int readCount=0;

void setup() {
  Serial.begin(9600);
  Serial.print("Reading DS18B20\n");
}

void loop()
{
  bool useMonitor=false;
  if (getTemperature()) {
    if (useMonitor) {
      Serial.print("Read #");
      Serial.print(++readCount);
      Serial.print(": temp: ");
    }
    Serial.println(temp);
  }
  else {
    Serial.println("Read failed!");
  }
  delay(1000);
}

// Read temperature using the global ds object.
boolean getTemperature() {
  byte i;
  byte present = 0;
  byte data[12];
  static byte addr[8];
  static bool addrValid=false;

  if(!addrValid) {
    // Find a device
    // TODO: check more than the first device on bus
    if (!ds.search(addr)) {
     ds.reset_search();
      return false;
    }
    if (OneWire::crc8( addr, 7) != addr[7]) {
      return false;
    }
    if (addr[0] != DS18S20_ID && addr[0] != DS18B20_ID) {
      return false;
    }
    else
      addrValid=true;
  }

  // Start conversion
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);
  // Wait some time...
  delay(850);
  present = ds.reset();
  ds.select(addr);

  // Issue Read scratchpad command
  ds.write(0xBE);
  // Receive 9 byte schratchpad
  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  // Validate data
  // TODO: also support 9-11 bit conv ( 0.5, 0.25, 0.125 increments) 
  if (   OneWire::crc8(data,8) == data[8] // crc ok?
      && (data[4] & 0x60) == 0x60) {      // was this 12 bit conv?
    // Calculate temperature value (
    temp = ( ((data[1] & 0x07) << 8) + data[0] ) * 0.0625;
    temp *= (data[1]&0x80?-1:1); // did we go sub-zero?
    return true;
  }
  return false;
}
