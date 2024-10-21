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



// Teensy is mounted with two rows hanging over the socket
// That sets up GPIO 1 as pin 1 in the 2716 socket 
#define PIN_ADDR7          1   // 1.02
#define PIN_ADDR6          2   // 4.04
#define PIN_ADDR5          3   // 4.05
#define PIN_ADDR4          4   // 4.06
#define PIN_ADDR3          5   // 4.08
#define PIN_ADDR2          6   // 2.10
#define PIN_ADDR1          7   // 2.17
#define PIN_ADDR0          8   // 2.16
#define PIN_DATA0          9   // 2.11
#define PIN_DATA1          10  // 2.00
#define PIN_DATA2          11  // 2.02
#define PIN_VSS            12  // 2.01

#define PIN_VCC            - // 3.3V (bend pin outward, do not connect)
#define PIN_ADDR8          23  // 1.25
#define PIN_ADDR9          22  // 1.24
#define PIN_VPP            21  // floating
#define PIN_G              20  // input (ignore since it's just for one 2kb chunk)
#define PIN_ADDR10         19  // 1.16
#define PIN_EP             18  // 1.17
#define PIN_DATA7          17  // 1.22
#define PIN_DATA6          16  // 1.23
#define PIN_DATA5          15  // 1.19
#define PIN_DATA4          14  // 1.18
#define PIN_DATA3          13  // 2.03

// address pins beyond the 2716 footprint
#define PIN_ADDR11         25 // 1.13  wire to U30.1 (A11)
#define PIN_ADDR12         26 // 1.30  wire to U30.2 (A12)
#define PIN_ADDR13         27 // 1.31  wire to U30.3 (A13)
#define PIN_ADDR14         38 // 1.28  wire to U30.4 (A14)
#define PIN_ADDR15         39 // 1.29  wire to U30.5 (A15)
#define PIN_CS             24 // 1.12  wire to U25.1 (nBuffer)

#define PIN_LED            13 // conflict! with PIN_DATA3


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

  uint32_t g1 = GPIO1_DR;
  uint32_t g2 = GPIO2_DR;
  uint32_t g4 = GPIO4_DR;

  uint32_t address = 
          ((g1 & (1<<29)) >> 29 ) | // A15
          ((g1 & (1<<28)) >> 28 ) | // A14
          ((g1 & (1<<31)) >> 31 ) | // A13
          ((g1 & (1<<30)) >> 30 ) | // A12
          ((g1 & (1<<13)) >> 13 ) | // A11
          ((g1 & (1<<16)) >> 16 ) | // A10
          ((g1 & (1<<24)) >> 24 ) | // A9
          ((g1 & (1<<25)) >> 25 ) | // A8
          ((g1 & (1<<2)) >> 2 ) | // A7

          ((g4 & (1<<4)) >> 4 ) | // A6
          ((g4 & (1<<5)) >> 5 ) | // A5
          ((g4 & (1<<6)) >> 6 ) | // A4
          ((g4 & (1<<8)) >> 8 ) | // A3

          ((g2 & (1<<10)) >> 10 ) | // A2
          ((g2 & (1<<17)) >> 17 ) | // A1
          ((g2 & (1<<16)) >> 16 ); // A0

  return (0x7FFF & address);  // 32KB ROM (27C256)
}


void writeData( uint8_t data ) {

     const uint32_t lut_2[] = {
      0x00000000, // 0000 0000 0000 0000 0000 0000 0000 0000
      0x00000800, // 0000 0000 0001 0000 0000 0000 0000 0000
      0x00000001, // 1000 0000 0000 0000 0000 0000 0000 0000
      0x00000801, // 1000 0000 0001 0000 0000 0000 0000 0000
      0x00000004, // 0010 0000 0000 0000 0000 0000 0000 0000
      0x00000804, // 0010 0000 0001 0000 0000 0000 0000 0000
      0x00000005, // 1010 0000 0000 0000 0000 0000 0000 0000
      0x00000805, // 1010 0000 0001 0000 0000 0000 0000 0000
      0x00000008, // 0001 0000 0000 0000 0000 0000 0000 0000
      0x00000808, // 0001 0000 0001 0000 0000 0000 0000 0000
      0x00000009, // 1001 0000 0000 0000 0000 0000 0000 0000
      0x00000809, // 1001 0000 0001 0000 0000 0000 0000 0000
      0x0000000c, // 0011 0000 0000 0000 0000 0000 0000 0000
      0x0000080c, // 0011 0000 0001 0000 0000 0000 0000 0000
      0x0000000d, // 1011 0000 0000 0000 0000 0000 0000 0000
      0x0000080d, // 1011 0000 0001 0000 0000 0000 0000 0000
    };
    const uint32_t lut_1[] = {
      0x00000000, // 0000 0000 0000 0000 0000 0000 0000 0000
      0x00040000, // 0000 0000 0000 0000 0010 0000 0000 0000
      0x00080000, // 0000 0000 0000 0000 0001 0000 0000 0000
      0x000c0000, // 0000 0000 0000 0000 0011 0000 0000 0000
      0x00800000, // 0000 0000 0000 0000 0000 0001 0000 0000
      0x00840000, // 0000 0000 0000 0000 0010 0001 0000 0000
      0x00880000, // 0000 0000 0000 0000 0001 0001 0000 0000
      0x008c0000, // 0000 0000 0000 0000 0011 0001 0000 0000
      0x00400000, // 0000 0000 0000 0000 0000 0010 0000 0000
      0x00440000, // 0000 0000 0000 0000 0010 0010 0000 0000
      0x00480000, // 0000 0000 0000 0000 0001 0010 0000 0000
      0x004c0000, // 0000 0000 0000 0000 0011 0010 0000 0000
      0x00c00000, // 0000 0000 0000 0000 0000 0011 0000 0000
      0x00c40000, // 0000 0000 0000 0000 0010 0011 0000 0000
      0x00c80000, // 0000 0000 0000 0000 0001 0011 0000 0000
      0x00cc0000, // 0000 0000 0000 0000 0011 0011 0000 0000
    };

    GPIO2_DR = lut_2[ data & 0x0F ];
    GPIO1_DR = lut_1[ (data & 0xF0) >> 4 ];
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


 void printb(uint32_t n) {
    for (int i=0; i<32; i++) {
        if ( i % 4 == 0 ) {
            printf(" ");
        }
        if (n & 1) {
            printf("1");
        } else {
            printf("0");
        }
        n >>= 1;
    }
}

 void make_data_lut(void) {
    printf("const uint32_t lut_2[] = {\n");
    for (uint32_t i=0; i<1<<4; i++) {
        uint32_t d0 = (i>>0) & 0x01;
        uint32_t d1 = (i>>1) & 0x01;
        uint32_t d2 = (i>>2) & 0x01;
        uint32_t d3 = (i>>3) & 0x01;
        uint32_t d = 0;
        d |= d0 << 11;
        d |= d1 << 0;
        d |= d2 << 2;
        d |= d3 << 3;
        printf("\t0x%08lx, //",d);
        printb( d );
        printf("\n");
    }
    printf("};\n");

    printf("const uint32_t lut_1[] = {\n");
    for (uint32_t i=0; i<1<<4; i++) {
        uint32_t d0 = (i>>0) & 0x01;
        uint32_t d1 = (i>>1) & 0x01;
        uint32_t d2 = (i>>2) & 0x01;
        uint32_t d3 = (i>>3) & 0x01;
        uint32_t d = 0;
        d |= d0 << 18;
        d |= d1 << 19;
        d |= d2 << 23;
        d |= d3 << 22;
        printf("\t0x%08lx, //",d);
        printb( d );
        printf("\n");
    }
    printf("};\n");
 }



 /*

Pin            GPIO
1              1.02
0              1.03
24/A10        1.12
25/A11        1.13
19/A5          1.16
18/A4          1.17
14/A0          1.18
15/A1          1.19
40/A16         1.20
41/A17         1.21
17/A3          1.22
16/A2          1.23
22/A8          1.24
23/A9          1.25
20/A6          1.26
21/A7          1.27
38/A14         1.28
39/A5           1.29
26/A12        1.30
27/A13        1.31

10            2.00
12            2.01
11            2.02
13            2.03
6              2.10
9               2.11
32            2.12
8              2.16
7              2.17
36            2.18
37            2.19
35            2.28
34            2.29

45            3.12
44            3.13
43            3.14
42            3.15
47            3.16
46            3.17
28            3.18
31            3.22
30            3.23

2              4.04
3             4.05
4              4.06
33            4.07
5              4.08
51            4.22
48            4.24
53            4.25
52            4.26
49            4.27
50            4.28
54            4.29
29            4.31

 */
