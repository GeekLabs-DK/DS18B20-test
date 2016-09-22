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
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#define DS18B20_PIN 13
#define DS18S20_ID 0x10
#define DS18B20_ID 0x28

#define LED_PIN 10
#define LDR_PIN A0

OneWire ds(DS18B20_PIN);
float temp;
int readCount=0;
bool useMonitor=false;

void setup() {
  Serial.begin(9600);
  Serial.print("Reading DS18B20\n");
  
  lcd.begin(16, 2);
  pinMode(LED_PIN, OUTPUT);
}

int state=0;
#define MAKER   0
#define GEEK    1
#define TEMP    2
#define LIGHT   3
#define STATES  4
#define WSERIAL 42
int displaytime;
void loop()
{
  switch (state%STATES)
  {
    case TEMP:
      lcd.clear();
      lcd.print("Temperatursensor");
      for (int i=0; i<10;i++) {
        if (getTemperature()) {
          if (useMonitor) {
            Serial.print("Read #");
            Serial.print(++readCount);
            Serial.print(": temp: ");
         }
         Serial.println(temp);

         lcd.setCursor(0,1);
         lcd.print("    ");
         lcd.print(temp);
         //lcd.write(248);
         lcd.print(" C");
       }
       else
       {
         Serial.println("Read failed!");
         lcd.setCursor(0,1);
         lcd.print("    FEJL");
       }
      }
      displaytime=2000;
      break;

    case LIGHT:{
      int threshold=600;
      lcd.clear();
      lcd.print("   Lyssensor");
      for (int i=0; i<50;i++) {
        int light = analogRead(A0);
        lcd.setCursor(0,1);
        lcd.print("    ");
        lcd.print(light);
        lcd.print(light>threshold?" (off)  ":" (on)  ");
        digitalWrite(LED_PIN, light>threshold?true:false);
         //lcd.write(248);
        delay(500);
        digitalWrite(LED_PIN, true);
      }
      displaytime=0;
      }
      break;

    case WSERIAL:
      lcd.clear();
      while (Serial.available() > 0)
        lcd.print(Serial.read());
      displaytime=0;
      break;

    case MAKER:
      lcd.clear();
      lcd.print("  Maker Event");
      lcd.setCursor(0,1);
      lcd.print("EsbBib 1/10-2016");
      displaytime=5000;
      break;

    case GEEK:
      lcd.clear();
      lcd.print("   GeekLabs.dk");
      lcd.setCursor(0,1);
      lcd.print("    er med !");
      displaytime=3000;
      break;
  }
  delay(displaytime);
  state++;
  //int oldstate=state;
  //while ((state=rand()) == oldstate)
  ;
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
