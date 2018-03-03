/*
	This is an initial sketch to get your device registered at IOTappstory.com
	You will need an account at IOTAppStory.com

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
#define VERSION "V1.2.0"
#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                      // Button pin on the esp for selecting modes. D3 for the Wemos!



#if defined  ESP8266
  #include <ESP8266WiFi.h>                // esp8266 core wifi library
#elif defined ESP32
  #include <WiFi.h>                       // esp32 core wifi library
#endif
#include <IOTAppStory.h>                  // IotAppStory.com library

IOTAppStory IAS(APPNAME, VERSION, COMPDATE, MODEBUTTON);



// ================================================ SETUP ================================================
void setup() {
  String boardName = APPNAME"_" + WiFi.macAddress();
  IAS.preSetDeviceName(boardName);	      // preset Boardname
  IAS.preSetAutoUpdate(false);            // automaticUpdate (true, false)


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
  
  #ifdef  ESP8266
    IAS.onFirstBoot([]() {
      Serial.println(F(" Manual reset necessary after serial upload!"));
      Serial.println(F("*-------------------------------------------------------------------------*"));
      ESP.restart();
    });
  #endif  

  IAS.begin('F');                         // Optional parameter: What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase(default) | 'L' Leave intact

  IAS.setCallHome(true);                  // Set to true to enable calling home frequently (disabled by default)
  IAS.setCallHomeInterval(60);            // Call home interval in seconds, use 60s only for development. Please change it to at least 2 hours in production
  
  IAS.callHome(true);                     // call home immediately
}



// ================================================ LOOP =================================================
void loop() {
  IAS.buttonLoop();                       // this routine handles the reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
}
