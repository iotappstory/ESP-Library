/*
  This is an initial sketch to be used as a "blueprint" to create apps which can be used with IOTappstory.com infrastructure
  Your code can be filled wherever it is marked.

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

#define APPNAME "WemosLoader"
#define VERSION "V1.1.0"
#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0


#include <ESP8266WiFi.h>
#include <IOTAppStory.h>
#include <SSD1306.h>

IOTAppStory IAS(APPNAME, VERSION, COMPDATE, MODEBUTTON);
SSD1306  display(0x3c, D2, D1);

unsigned long printEntry;


void displayConfigMode() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, F("Connect to"));
  display.setFont(ArialMT_Plain_16);
  display.drawString(42, 30, F("Wi-Fi"));
  display.setFont(ArialMT_Plain_10);
  display.drawString(32, 50, "x:x:" + WiFi.macAddress().substring(9, 99));
  display.display();
}

void displayNoApp() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, F("Error"));
  display.drawString(32, 27, F("Not registred"));
  display.drawString(32, 39, F("No project"));
  display.drawString(32, 51, F("No app"));
  display.display();
}

void displayUpdate() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 13, F("Update"));
  display.drawString(52, 31, F("of"));
  display.drawString(32, 49, F("Sketch"));
  display.display();
}

void displayStartup() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, F("Press"));
  display.drawString(32, 30, F("Reset"));
  display.drawString(32, 45, F("Button"));
  display.display();
}

// ================================================ SETUP ================================================
void setup() {
  IAS.serialdebug(true);                  // 1st parameter: true or false for serial debugging. Default: false
  //IAS.serialdebug(true,115200);         // 1st parameter: true or false for serial debugging. Default: false | 2nd parameter: serial speed. Default: 115200

  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(48, 35, F("Wait"));
  display.display();

  String boardName = APPNAME"_" + WiFi.macAddress();
  IAS.preSetBoardname(boardName);         // preset Boardname
  IAS.preSetAutoUpdate(false);            // automaticUpdate (true, false)



  IAS.setCallHome(true);                  // Set to true to enable calling home frequently (disabled by default)
  IAS.setCallHomeInterval(60);            // Call home interval in seconds, use 60s only for development. Please change it to at least 2 hours in production



  IAS.onFirstBoot([]() {
    Serial.println(F(" Hardware reset necessary after Serial upload. Reset to continu!"));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    displayStartup();
    ESP.reset();
  });

  IAS.onConfigMode([]() {
    //Serial.println(F(" Starting configuration mode. Search for my WiFi and connect to 192.168.4.1."));
    //Serial.println(F("*-------------------------------------------------------------------------*"));
    displayConfigMode();
  });

  IAS.onFirmwareUpdate([]() {
    //Serial.println(F(" Checking if there is a firmware update available."));
    //Serial.println(F("*-------------------------------------------------------------------------*"));
    displayUpdate();
  });


  IAS.begin(true, 'F');                   // 1st parameter: true or false to view BOOT STATISTICS
  // 2nd parameter: Wat to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase(default) | 'L' Leave intact


  IAS.callHome(true);
}



// ================================================ LOOP =================================================
void loop() {
  IAS.buttonLoop();                       // this routine handles the reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration

  if (millis() - printEntry > 5000) {
    // if the scetch reaches this spot, no project was defined. Otherwise, it would load the defined sketch already before...
    Serial.println(F("\n\n                               ----------------   N O   A P P   L O A D E D   ----------------"));
    Serial.println(F("\n----------------  P L E A S E  C R E A T E   P R O J E C T   O N   I O T A P P S T O R Y . C O M   ----------------\n"));
    printEntry = millis();
    displayNoApp();
  }
}
