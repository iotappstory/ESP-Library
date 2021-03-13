/*
  This is an initial sketch to get your device registered at IOTappstory.com
  You will need an account at IOTAppStory.com

  You will need either the button & OLED shields! or the LOLIN oled v2.1.0 shield!

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
                                                          // This button is not present on the v2.1.0 oled shield, buttons A/B are used instead

#include <IOTAppStory.h>                                  // IotAppStory.com library
#include <SSD1306.h>                                      // OLED library by Daniel Eichhorn https://github.com/ThingPulse/esp8266-oled-ssd1306
#include "logo.h"                                         // IOTAppStory.com logo in XBM format
#include <LOLIN_I2C_BUTTON.h>                             // Wemos button library for wemos lolin oled shield v2.1.0 https://github.com/wemos/LOLIN_OLED_I2C_Button_Library
I2C_BUTTON button(0x31);                                  // I2C address 0x31

IOTAppStory IAS(COMPDATE, MODEBUTTON);                    // Initialize IotAppStory
SSD1306  display(0x3c, D2, D1, GEOMETRY_128_64);          // Initialize OLED



// ================================================ VARS =================================================
unsigned long printEntry;
String deviceName = "wemosloader";
String chipId;
uint8_t callHomeIntervall = 60;


// ================================================ SETUP ================================================
void setup() {
  display.init();                                         // setup OLED and show "Wait"
  display.flipScreenVertically();
  display.clear();
  display.drawXbm(46, 14, IAS_Logo_width, IAS_Logo_height, IAS_Logo_36_bits);
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 48, "IOTAppStory");
  display.display();
  delay(5000);

  // creat a unique deviceName for classroom situations (deviceName-123)
  chipId      = String(ESP_GETCHIPID);
  chipId      = "-"+chipId.substring(chipId.length()-3);
  deviceName += chipId;

  IAS.preSetDeviceName(deviceName);                       // preset deviceName this is also your MDNS responder: http://deviceName-123.local
  IAS.preSetAppName(F("Wemos"));                          // preset appName | The appName & appVersion get updated when you receive OTA updates. As this is your first app we will set it manually.
  IAS.preSetAppVersion(F("1.3.1"));                       // preset appVersion

  // default mode button not connected on LOLIN oled v2.1.0 shield
  IAS.onModeButtonShortPress([]() {
    Serial.println(F(" If mode button is released, I will enter in firmware update mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    dispTemplate_threeLineV2(F("Release"), F("for"), F("Updates"));
  });

  // default mode button not connected on LOLIN oled v2.1.0 shield
  IAS.onModeButtonLongPress([]() {
    Serial.println(F(" If mode button is released, I will enter in configuration mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    dispTemplate_threeLineV2(F("Release"), F("for"), F("Config"));
  });


  IAS.onFirstBoot([]() {
    dispTemplate_threeLineV1(F("Welcome"), F("To"), F("IOTAppStory"));
    IAS.eraseEEPROM('P');                                 // Optional! What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase
    delay(3000);
  });

  IAS.onConfigMode([]() {
    //  if wifi clientmode display ip addr
    if(WiFi.isConnected()){
      dispTemplate_threeLineV1(F("Connect to"), String(WiFi.SSID()) , WiFi.localIP().toString() );
      }else{
      dispTemplate_threeLineV2(F("Connect to"), F("Wi-Fi"), "xxxxx-" + chipId);
      }
  });

  IAS.onFirmwareUpdateCheck([]() {
    dispTemplate_threeLineV2(F("Checking"), F("for"), F("Updates"));
  });

  IAS.onFirmwareUpdateDownload([]() {
    dispTemplate_threeLineV2(F("Download"), F("&"), F("Install App"));
    delay(1000);
  });

  IAS.onFirmwareUpdateProgress([](int written, int total){
    String perc = String(written / (total / 100)) + "%";
    dispTemplate_progressBarV1(F("Installing"), perc, written, total );
    Serial.print(".");
  });

  IAS.onFirmwareUpdateError([](String statusMessage) {
    dispTemplate_threeLineV1(F("Update"), F("Error"), F("Check logs"));
  });

  IAS.begin();                                            // Run IOTAppStory
  IAS.setCallHomeInterval(callHomeIntervall);             // Call home interval in seconds(disabled by default), 0 = off, use 60s only for development. Please change it to at least 2 hours in production
}


// ================================================ LOOP =================================================
void loop() {
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)

  buttonLoop(); // check wemos lolin oled shield v2.1.0 buttons for press event for wemos oled display shield

  if (millis() - printEntry > (callHomeIntervall * 500) && digitalRead(D3) == HIGH) {
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
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 32, str2);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(96, 51, str3);
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

void dispTemplate_progressBarV1(String str1, String str2, int written , int total) {
  int progress = (written / (total / 100));
  display.clear();

  if(progress < 100){
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(32, 13, str1);
    display.setFont(ArialMT_Plain_24);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 24, str2);
    display.drawProgressBar(32, 56, 63, 6, progress);

    }else if(progress == 100){
      display.setFont(ArialMT_Plain_10);
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.drawString(64, 48, "rebooting");
      display.drawXbm(46, 14, IAS_Logo_width, IAS_Logo_height, IAS_Logo_36_bits);
    }
  display.display();
}

   // 0 None
   // 1 Short Press
   // 2 Long Press
   // 3 Double Press
   // 4 Hold
   // check wemos lolin oled shield v2.1.0 buttons for press event for wemos oled display shield
void buttonLoop() {
  if (button.get() == 0) {
                                                  //  if button A has been pressed once
    if (button.BUTTON_A == 1) {
      dispTemplate_threeLineV1(F("Press twice"), F("for"), F("call home"));
    }
    //  if button A has been double pressed
    if (button.BUTTON_A == 3) {
      IAS.callHome();                             //  check the IAS server for updates
    }
                                                  //  if button B has been pressed once
    if (button.BUTTON_B == 1) {
      dispTemplate_threeLineV1(F("Press twice"), F("for"), F("config mode"));
    }
                                                  //  if button B has been double pressed
    if (button.BUTTON_B == 3) {
      IAS.espRestart('C');                        //  restart in config mode
    }
  }
}
