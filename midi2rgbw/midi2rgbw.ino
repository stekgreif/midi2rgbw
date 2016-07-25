#include "MIDIUSB.h"
#include "Adafruit_NeoPixel.h"
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN            6
#define NUMPIXELS      8

#define _RED 1
#define _GRN 2
#define _BLU 3
#define _WHT 0

uint8_t color = 0;
uint8_t id = 0;
uint8_t brightness = 0;

uint8_t prevColor[4] = {};

int16_t potVal = 0;
int16_t lastPotVal = 0;


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRBW + NEO_KHZ800);

void setup()
{
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  pixels.begin();
  pixels.show();
}


// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) 
{
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}




void loop()
{
  midiEventPacket_t rx;

  potVal = analogRead(A0) >> 5;
  potVal = potVal << 2;
  
  if( potVal != lastPotVal ) 
  {
    Serial.println(potVal);
    controlChange(0, 0, potVal);
    MidiUSB.flush();
    lastPotVal = potVal;
  }

  do
  {
    rx = MidiUSB.read();

    if (rx.header == 0xB)
    {
      color = rx.byte1 & 0b00001111;
      id = rx.byte2;
      brightness = rx.byte3 << 1;

      uint32_t prevCol = pixels.getPixelColor(id);
      prevColor[_BLU] = (uint8_t)  prevCol;
      prevColor[_GRN] = (uint8_t) (prevCol >> 8);
      prevColor[_RED] = (uint8_t) (prevCol >> 16);
      prevColor[_WHT] = (uint8_t) (prevCol >> 24);

      Serial.print("LED: color: ");

      switch (color)
      {

        case 0: //R
          {
            pixels.setPixelColor(id, pixels.Color(brightness, prevColor[_GRN], prevColor[_BLU], prevColor[_WHT]));
            Serial.print("red");
            break;
          }
        case 1: //G
          {
            pixels.setPixelColor(id, pixels.Color(prevColor[_RED], brightness, prevColor[_BLU], prevColor[_WHT]));
            Serial.print("green");
            break;
          }
        case 2: //B
          {
            pixels.setPixelColor(id, pixels.Color(prevColor[_RED], prevColor[_GRN], brightness, prevColor[_WHT]));
            Serial.print("blue");
            break;
          }
        case 3: //W
          {
            pixels.setPixelColor(id, pixels.Color(prevColor[_RED], prevColor[_GRN], prevColor[_BLU], brightness));
            Serial.print("white");
            break;
          }
        case 4: //all colors
          {
            pixels.setPixelColor(id, pixels.Color(brightness, brightness, brightness, brightness));
            Serial.print("all colors");
            break;
          }
      }

      Serial.print("  ||  ");
      Serial.print("id: ");
      Serial.print(id);
      Serial.print(" | ");
      Serial.print("brightness: ");
      Serial.print(brightness);
      Serial.print("  ||  ");

      Serial.print("Last colors: ");
      Serial.print(prevColor[0]);
      Serial.print(" | ");
      Serial.print(prevColor[1]);
      Serial.print(" | ");
      Serial.print(prevColor[2]);
      Serial.print(" | ");
      Serial.print(prevColor[3]);
      Serial.println(" | ");
      pixels.show();

    }
  } while (rx.header != 0);
}
