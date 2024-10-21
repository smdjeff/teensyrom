//
//  TEENSY ROM
//
//  Author: Jeff Mathews
//
//  Based on : Ted Fried, MicroCore Labs ROM_Emulator
//
//------------------------------------------------------------------------
//
// Copyright (c) 2021 Ted Fried
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//------------------------------------------------------------------------

#include <stdint.h>
#include <SD.h>
#include <SPI.h>



// all inputs are optimized for reading software speed and a single GPIO bank
#define PIN_ADDR0          19   // 1.16
#define PIN_ADDR1          18   // 1.17
#define PIN_ADDR2          14   // 1.18
#define PIN_ADDR3          15   // 1.19
#define PIN_ADDR4          40   // 1.20
#define PIN_ADDR5          41   // 1.21
#define PIN_ADDR6          17   // 1.22
#define PIN_ADDR7          16   // 1.23
#define PIN_ADDR8          22   // 1.24
#define PIN_ADDR9          23   // 1.25
#define PIN_ADDR10         20   // 1.26

// address pins beyond the 2716 footprint requiring jumper hookup
#define PIN_ADDR11         21 // 1.27  wire to Sega G80 U30.1 (A11)
#define PIN_ADDR12         38 // 1.28  wire to Sega G80 U30.2 (A12)
#define PIN_ADDR13         39 // 1.29  wire to Sega G80 U30.3 (A13)
#define PIN_ADDR14         26 // 1.30  wire to Sega G80 U30.4 (A14)
#define PIN_ADDR15         27 // 1.31  wire to Sega G80 U30.5 (A15)
#define PIN_CS             25 // 1.13  wire to Sega G80 U25.1 (nBuffer)

// all outputs are optimized for writing software speed and single GPIO bank
#define PIN_DATA0          6   // 2.10
#define PIN_DATA1          9   // 2.11
#define PIN_DATA2          32  // 2.12
#define PIN_DATA3          8   // 2.16
#define PIN_DATA4          7   // 2.17
#define PIN_DATA5          36  // 2.18
#define PIN_DATA6          37  // 2.19
#define PIN_DATA7          35  // 2.28


#define PIN_LED            13 // onboard Teensy


uint8_t memory[65536] = {0,};


void dataPins(uint8_t mode) {
  pinMode(PIN_DATA0,   mode);
  pinMode(PIN_DATA1,   mode);
  pinMode(PIN_DATA2,   mode);
  pinMode(PIN_DATA3,   mode);
  pinMode(PIN_DATA4,   mode);
  pinMode(PIN_DATA5,   mode);
  pinMode(PIN_DATA6,   mode);
  pinMode(PIN_DATA7,   mode);
}

// Setup Teensy 4.0 IO's
//
void setup() {
    
  // pinMode(PIN_LED,   OUTPUT);

  pinMode(PIN_ADDR0,   INPUT);
  pinMode(PIN_ADDR1,   INPUT);
  pinMode(PIN_ADDR2,   INPUT);
  pinMode(PIN_ADDR3,   INPUT);
  pinMode(PIN_ADDR4,   INPUT);
  pinMode(PIN_ADDR5,   INPUT);
  pinMode(PIN_ADDR6,   INPUT);
  pinMode(PIN_ADDR7,   INPUT);
  pinMode(PIN_ADDR8,   INPUT);
  pinMode(PIN_ADDR9,   INPUT);
  pinMode(PIN_ADDR10,  INPUT);
  pinMode(PIN_ADDR11,  INPUT);
  pinMode(PIN_ADDR12,  INPUT);
  pinMode(PIN_ADDR13,  INPUT);
  pinMode(PIN_ADDR14,  INPUT);  
  pinMode(PIN_ADDR15,  INPUT);

  pinMode(PIN_CS,  INPUT);

  if (SD.begin(BUILTIN_SDCARD)) {
    File fp = SD.open("rom.bin", FILE_READ);
    if (fp) {
      int i = 0;
      while (fp.available()) {
         memory[ i ] = fp.read();
         i++;
      }
    }

  }
}



// -------------------------------------------------
//
// Main loop
//
// -------------------------------------------------


uint16_t readAddress(void) {
  uint32_t address = GPIO1_DR >> 16; // A0-A15 maps contiguously to 1.16-1.31
  return (0x7FFF & address);  // 32KB ROM (27C256)
}


void writeData( uint8_t data ) {
  uint32_t g1 = (data & 0x07) << 10;        // D0-D2 maps to 2.10-2.12
  uint32_t g2 = (data & 0x78) >> 3 << 16;   // D3-D6 maps to 2.16-2.19
  uint32_t g3 = (data & 0x80) >> 7 << 28;   // D7    maps to 2.28 
  GPIO2_DR = g1 | g2 | g3;
}

void loop() {

  dataPins( INPUT );
  while  ( digitalRead( PIN_CS ) != 0 ) {
    // wait for assert chip select 
  };

  uint16_t address = readAddress();
  writeData( memory[ address] );

  // digitalWrite(PIN_LED, 1);

  dataPins( OUTPUT );
  while  ( digitalRead( PIN_CS ) == 0 ) {
    // wait for deassert chip select 
  };

  // digitalWrite(PIN_LED, 1);

 }
