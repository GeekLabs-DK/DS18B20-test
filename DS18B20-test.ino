// Test sketch for DS18B20/DS18S20 temperature sensors thrown together in GeekLabs on 2015-11-19.
//
// Reads temperature of sensors present on an Arduino GPIO as a OneWire bus and prints the temperature on the Arduino serial console.
// Based on https://tushev.org/articles/arduino/10/how-it-works-ds18b20-and-arduino
//
// DS18B20 is the "Programmable Resolution 1-Wire Digital Thermometer" from Maxim former Dallas.
// Product homepage: https://www.maximintegrated.com/en/products/analog/sensors-and-sensor-interface/DS18B20.html
// PDF datasheet: https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf

#include <OneWire.h>

#define PIN 3

#define DS18S20_ID 0x10
#define DS18B20_ID 0x28

OneWire ds(PIN);
float temp;

void setup() {
  Serial.begin(9600);
  Serial.print("Reading DS18B20\n");
}

void loop()
{
  getTemperature();
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print("\n");
  delay(1000);
}


boolean getTemperature() {
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  //find a device
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
  ds.reset();
  ds.select(addr);
  // Start conversion
  ds.write(0x44, 1);
  // Wait some time...
  delay(850);
  present = ds.reset();
  ds.select(addr);
  // Issue Read scratchpad command
  ds.write(0xBE);
  // Receive 9 bytes
  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  // Calculate temperature value
  // TODO: handle sign properly
  temp = ( ((data[1] & 0x0e) << 8) + data[0] ) * 0.0625;
  return true;
}
