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

  INITLoader V1.2.0
*/

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                      // Button pin on the esp for selecting modes. D3 for the Wemos!



#include <IOTAppStory.h>                  // IOTAppStory.com library

IOTAppStory IAS(COMPDATE, MODEBUTTON);    // Initialize IOTAppStory


// ================================================ VARS =================================================
String deviceName = "initloader";



// ================================================ SETUP ================================================
void setup() {
  
  #if defined  ESP8266
    String chipId  = String(ESP.getChipId());   // creat a unique deviceName for classroom situations (deviceName-123)
    chipId         = "-"+chipId.substring(chipId.length()-3);
    deviceName    += chipId;
  #endif
  
  IAS.preSetDeviceName(deviceName);	      // preset deviceName this is also your MDNS responder: http://deviceName.local
  IAS.preSetAppName(F("INITLoader"));     // preset appName
  IAS.preSetAppVersion(F("1.2.0"));       // preset appVersion
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
  IAS.loop();                             // this routine handles the calling home on the configured itnerval as well as reaction of the Flash button. If short press: update of skethc, long press: Configuration
}
