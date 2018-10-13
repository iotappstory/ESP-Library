/*
	This is an initial sketch to get your device registered at IOTappstory.com
	You will need an account at IOTAppStory.com 

  You will need the button & OLED shields!

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

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON D3                                     // Button pin on the esp for selecting modes. 0 for Generic devices!


#include <IOTAppStory.h>                                  // IotAppStory.com library
#include <SSD1306.h>                                      // OLED library by Daniel Eichhorn


IOTAppStory IAS(COMPDATE, MODEBUTTON);                    // Initialize IotAppStory
SSD1306  display(0x3c, D2, D1);                           // Initialize OLED


// ================================================ VARS =================================================
unsigned long printEntry;

String deviceName = "wemosloader";


// ================================================ SETUP ================================================
void setup() {
  display.init();                                         // setup OLED and show "Wait"
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(48, 35, F("Wait"));
  display.display();


  #if defined  ESP8266
    String chipId  = String(ESP.getChipId());   // creat a unique deviceName for classroom situations (deviceName-123)
    chipId         = "-"+chipId.substring(chipId.length()-3);
    deviceName    += chipId;
  #endif
  
  IAS.preSetDeviceName(deviceName);                       // preset deviceName this is also your MDNS responder: http://deviceName.local
  IAS.preSetAppName(F("WemosLoader"));                    // preset appName
  IAS.preSetAppVersion(F("1.1.0"));                       // preset appVersion
  IAS.preSetAutoUpdate(false);                            // automaticUpdate (true, false)


  IAS.onModeButtonShortPress([]() {
    Serial.println(F(" If mode button is released, I will enter in firmware update mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    dispTemplate_threeLineV2(F("Release"), F("for"), F("Updates"));
  });

  IAS.onModeButtonLongPress([]() {
    Serial.println(F(" If mode button is released, I will enter in configuration mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    dispTemplate_threeLineV2(F("Release"), F("for"), F("Config"));
  });
  
  IAS.onFirstBoot([]() {
    Serial.println(F(" Manual reset necessary after serial upload!"));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    dispTemplate_threeLineV1(F("Press"), F("Reset"), F("Button"));
    ESP.reset();
  });

  IAS.onConfigMode([]() {
    dispTemplate_threeLineV2(F("Connect to"), F("Wi-Fi"), "xxxxx-" + chipId);
  });

  IAS.onFirmwareUpdateCheck([]() {
    dispTemplate_threeLineV2(F("Checking"), F("for"), F("Updates"));
  });

  IAS.onFirmwareUpdateDownload([]() {
    dispTemplate_threeLineV2(F("Download"), F("&"), F("Install App"));
  });

  IAS.onFirmwareUpdateError([]() {
    dispTemplate_threeLineV1(F("Update"), F("Error"), F("Check logs"));
  });


  IAS.begin('F');                                           // Optional parameter: What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase(default) | 'L' Leave intact

  IAS.setCallHome(true);                                    // Set to true to enable calling home frequently (disabled by default)
  IAS.setCallHomeInterval(60);                              // Call home interval in seconds, use 60s only for development. Please change it to at least 2 hours in production
  
  //-------- Your Setup starts from here ---------------


  IAS.callHome(true);
  
}



// ================================================ LOOP =================================================
void loop() {
  IAS.loop();                                         // this routine handles the reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration


  if (millis() - printEntry > 5000 && digitalRead(D3) == HIGH) {
    // if the sketch reaches this point, you failed to activate your device at IotAppStory.com, did not create a project or did not add an app to your project
    printEntry = millis();
    dispTemplate_fourLineV1(F("Error"), F("Not registred"), F("No project"), F("No app"));
  }
}



// ================================================ Extra functions ======================================
void dispTemplate_threeLineV1(String str1, String str2, String str3) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, str1);
  display.drawString(32, 30, str2);
  display.drawString(32, 45, str3);
  display.display();
}

void dispTemplate_threeLineV2(String str1, String str2, String str3) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 13, str1);
  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 24, str2);
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(96, 51, str3);
  display.display();
}

void dispTemplate_fourLineV1(String str1, String str2, String str3, String str4) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, str1);
  display.drawString(32, 27, str2);
  display.drawString(32, 39, str3);
  display.drawString(32, 51, str4);
  display.display();
}
