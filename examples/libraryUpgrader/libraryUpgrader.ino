/*
  This sketch is an example of how you can upgrade from the previous library(2.0.X) 
  to the new 2.1.0 library EEPROM layout. If successful all the previous settings
  like Wifi, IAS device registration and added fields get "rewritten" to the new 
  EEPROM layout. 

  You can use this sketch as an intermediate update or add this code to your next 
  firmware update. It's wise to test this locally first! Make sure to include the 
  upgrade helper file in your sketch folder. And do NOT use presets!

  Copyright (c) [2019] [Onno Dirkzwager]

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

  libraryUpgrader V2.1.0
*/

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                              // Button pin on the esp for selecting modes. D3 for the Wemos!


#include <IOTAppStory.h>                          // IotAppStory.com library
#include <EEPROM.h>                               // Default EEPROM library
IOTAppStory IAS(COMPDATE, MODEBUTTON);            // Initialize IOTAppStory



// ================================================ CHANGE THESE VALUES =========================================
// Study the added fields in your current app you want to upgrade:
// IAS.addField(LEDpin, "ledpin", 2, 'P');
// IAS.addField(blinkTime, "Blinktime(mS)", 5, 'N');
// reference to org variable | field label value | max char return | Field type

#define WRITECHANGES                true          // you might want to do a practice run and turn this to false
#define NROFADDEDFIELDS             8             // How many added fields should we search for?
#define LARGESTLENGTH               5             // What is de largest "max char return"

int fieldMaxLength[NROFADDEDFIELDS] = {2  ,5};    // Max returns as used in the added fields.
char fieldType[NROFADDEDFIELDS]     = {'P','N'};  // Field types as used in the added fields. (L if none was entered!)



// ================================================ OLD struct & defines =========================================
#define OLD_STRUCT_CHAR_ARRAY_SIZE  50
#define OLD_STRUCT_PASSWORD_SIZE    64
#define OLD_STRUCT_BNAME_SIZE       30
#define OLD_STRUCT_COMPDATE_SIZE    20

typedef struct oldConfig{
  char actCode[7];                              // saved IotAppStory activation code
  char appName[33];
  char appVersion[12];
  char ssid[3][OLD_STRUCT_CHAR_ARRAY_SIZE];     // 3x SSID
  char password[3][OLD_STRUCT_PASSWORD_SIZE];   // 3x PASS
  char deviceName[OLD_STRUCT_BNAME_SIZE];
  char compDate[OLD_STRUCT_COMPDATE_SIZE];      // saved compile date time
  #if defined  ESP8266                          // the esp32 in lib 2.0.X uses a certificate stored in PROGMEM
    char sha1[60];
  #endif
  #if CFG_AUTHENTICATE == true
    char cfg_pass[17];
  #endif
  const char magicBytes[4] = MAGICBYTES;
};



// ================================================ SETUP ================================================
void setup() {
  
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

  IAS.onFirmwareUpdateProgress([](int written, int total){
      Serial.print(".");
  });

  IAS.onFirstBoot([]() {
    runUpgrade();                           // Run the library upgrader
  });
  
  
  
  IAS.begin();                              // Run IOTAppStory
  IAS.setCallHomeInterval(60);              // Call home interval in seconds(disabled by default), 0 = off, use 60s only for development. Please change it to at least 2 hours in production

  
  //-------- Your Setup starts from here ---------------


}



// ================================================ LOOP =================================================
void loop() {
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)


  //-------- Your Sketch starts from here ---------------


}
