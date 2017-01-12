/* This sketch connects to IOTAppStory and downloads the assigned firmware. The firmware assignment is done on the server, and is based on the MAC address of the board

    On the server, you need to upload the PHP script "iotappstory.php", and put your .bin files in the .\bin folder

    This sketch is based on the ESPhttpUpdate examples

   To add new constants in WiFiManager search for "NEW CONSTANTS" and insert them according the "boardName" example

  Copyright (c) [2016] [Andreas Spiess]

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#define SKETCH "IOTappStoryLoader "
#define VERSION "V1.0"
#define FIRMWARE SKETCH VERSION

#define SERIALDEBUG       // Serial is used to present debugging messages 

#define LEDS_INVERSE   // LEDS on = GND

#include <credentials.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>        //https://github.com/kentaylor/WiFiManager
#include <Ticker.h>
#include <EEPROM.h>

extern "C" {
#include "user_interface.h" // this is for the RTC memory read/write functions
}

// -------- PIN DEFINITIONS ------------------
#ifdef ARDUINO_ESP8266_ESP01           // Generic ESP's 
#define MODEBUTTON 0
#define LEDgreen 13
//#define LEDred 12
#else
#define MODEBUTTON D3
#define LEDgreen D7
//#define LEDred D6
#endif

//---------- DEFINITIONS for SKETCH ----------
#define STRUCT_CHAR_ARRAY_SIZE 50  // length of config variables
#define MAX_WIFI_RETRIES 50
#define RTCMEMBEGIN 68
#define MAGICBYTE 85

// --- Optional -----
#define SERVICENAME "LOADER"  // name of the MDNS service used in this group of ESPs


//-------- SERVICES --------------



//--------- ENUMS AND STRUCTURES  -------------------

typedef struct {
  char ssid[STRUCT_CHAR_ARRAY_SIZE];
  char password[STRUCT_CHAR_ARRAY_SIZE];
  char boardName[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStory1[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStoryPHP1[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStory2[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStoryPHP2[STRUCT_CHAR_ARRAY_SIZE];
  // insert NEW CONSTANTS according boardname example HERE!
  char magicBytes[4];
} strConfig;

strConfig config = {
  "",
  "",
  "INITloader",
  "iotappstory.com",
  "/ota/esp8266-v1.php",
  "iotappstory.com",
  "/ota/esp8266-v1.php",
  "CFG"  // Magic Bytes
};



//---------- VARIABLES ----------



//---------- FUNCTIONS ----------
// to help the compiler, sometimes, functions have  to be declared here
void initialize(void);
void connectNetwork(void);
void loopWiFiManager(void);
void eraseFlash(void);


//---------- OTHER .H FILES ----------
#include "ESP_Helpers.h"           // General helpers for all IOTappStory sketches. Is also placed inside this directory for ease of use, but should be the same as the library file
#include "IOTappStoryHelpers.h"    // Sketch specific helpers for all IOTappStory sketches


//-------------------------- SETUP -----------------------------------------
void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 5; i++) DEBUG_PRINTLN("");
  DEBUG_PRINTLN("Start "FIRMWARE);

  // ----------- PINS ----------------
  pinMode(MODEBUTTON, INPUT_PULLUP);  // MODEBUTTON as input for Config mode selection

#ifdef LEDgreen
  pinMode(LEDgreen, OUTPUT);
  digitalWrite(LEDgreen, LEDOFF);
#endif
#ifdef LEDred
  pinMode(LEDred, OUTPUT);
  digitalWrite(LEDred, LEDOFF);
#endif


  // ------------- INTERRUPTS ----------------------------
  blink.detach();

  //------------- LED and DISPLAYS ------------------------
  LEDswitch(GreenBlink);

  for (int ii = 0; ii < 3; ii++) {
    for (int i = 0; i < 2; i++) Serial.println("");
    Serial.println("!!!!!!!!!!!!!!!  Please press reset button. ONLY FIRST TIME AFTER SERIAL LOAD!   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    for (int i = 0; i < 2; i++) Serial.println("");
    delay(5000);
  }

  eraseFlash();
  writeConfig();  // configuration in EEPROM
  initWiFiManager();
  Serial.println("setup done");
}

// ----------------------- LOOP --------------------------------------------

void loop() {
  yield();
  loopWiFiManager();
  //-------- Standard Block ---------------
}

//------------------------ END LOOP ------------------------------------------------

