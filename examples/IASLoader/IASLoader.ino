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

#define APPNAME "INITLoader"
#define VERSION "V1.1.0"
#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0


#include <ESP8266WiFi.h>
#include <IOTAppStory.h>
IOTAppStory IAS(APPNAME, VERSION, COMPDATE, MODEBUTTON);

unsigned long printEntry;
String boardName;



// ================================================ SETUP ================================================
void setup() {
  IAS.serialdebug(true);                  // 1st parameter: true or false for serial debugging. Default: false
  //IAS.serialdebug(true,115200);         // 1st parameter: true or false for serial debugging. Default: false | 2nd parameter: serial speed. Default: 115200
 
  
  boardName = APPNAME"_" + WiFi.macAddress();
  IAS.preSetBoardname(boardName);	        // preset Boardname
  IAS.preSetAutoUpdate(false);            // automaticUpdate (true, false)
  

  IAS.begin(true,'F');                    // 1st parameter: true or false to view BOOT STATISTICS
                                          // 2nd parameter: Wat to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase(default) | 'L' Leave intact

  IAS.setCallHome(true);                  // Set to true to enable calling home frequently (disabled by default)
  IAS.setCallHomeInterval(60);            // Call home interval in seconds, use 60s only for development. Please change it to at least 2 hours in production


  // You can configure callback functions that can give feedback to the app user about the current state of the application.
  // In this example we use serial print to demonstrate the call backs. But you could use leds etc.
  IAS.onModeButtonShortPress([]() {
    Serial.println(F(" If mode button is released, I will enter in firmware update mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onModeButtonLongPress([]() {
    Serial.println(F(" If mode button is released, I will enter in configuration mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });
  

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
  }
}
