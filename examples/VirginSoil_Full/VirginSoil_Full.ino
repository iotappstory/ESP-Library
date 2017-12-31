
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

#define APPNAME "virginSoilFull"
#define VERSION "V2.2.1"
#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0


#include <IOTAppStory.h>
IOTAppStory IAS(APPNAME, VERSION, COMPDATE, MODEBUTTON);



// ================================================ EXAMPLE VARS =========================================
// used in this example to print variables every 10 seconds
unsigned long printEntry;

// We want to be able to edit these example variables below from the wifi config manager
// Currently only char arrays are supported.
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)

char* lbl1        = "Light Show";
char* lbl2        = "Living Room";
char* exampleURL  = "http://someapi.com/getdata.php?userid=1234&key=7890abc";
char* timeZone    = "0.0";
char* ledPin      = "2";



// ================================================ SETUP ================================================
void setup() {
  /* TIP! delete lines below when not used */
  IAS.serialdebug(true);                              // 1st parameter: true or false for serial debugging. Default: false
  //IAS.serialdebug(true,115200);                     // 1st parameter: true or false for serial debugging. Default: false | 2nd parameter: serial speed. Default: 115200


  IAS.preSetBoardname("virginSoil-full");             // preset Boardname this is also your MDNS responder: http://virginSoil-full.local
  //IAS.preSetAutoUpdate(false);                      // automaticUpdate (true, false)
  //IAS.preSetAutoConfig(false);                      // automaticConfig (true, false)
  //IAS.preSetWifi("ssid","password");                // preset Wifi
  /* TIP! Delete Wifi cred. when you publish your App. */


  IAS.addField(lbl1, "label1", "Label 1", 16);        // These fields are added to the config wifimanager and saved to eeprom. Updated values are returned to the original variable.
  IAS.addField(lbl2, "label2", "Label 2", 16);        // reference to org variable | field name | field label value | max char return
  IAS.addField(exampleURL, "url", "Example url", 80);
  IAS.addField(timeZone, "timezone", "Timezone", 4);
  IAS.addField(ledPin, "ledpin", "ledPin", 2);
  /* TIP! delete the lines above when not used */

  
	//IAS.begin();
	//IAS.begin(true);                                  // 1st parameter: true or false to view BOOT STATISTICS
  IAS.begin(true,'P');                                // 2nd parameter: Wat to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase(default) | 'L' Leave intact


  IAS.setCallHome(true);											        // Set to true to enable calling home frequently (disabled by default)
  IAS.setCallHomeInterval(60);										    // Call home interval in seconds, use 60s only for development. Please change it to at least 2 hours in production


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

  IAS.onModeButtonVeryLongPress([]() {
    Serial.println(F(" If mode button is released, I won't do anything."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    /* TIP! You can use this callback to put your app on it's own configuration mode */
  });
  
  /*
  IAS.onModeButtonNoPress([]() {
    Serial.println(F(" Mode Button is not pressed.");
    Serial.println(F("*-------------------------------------------------------------------------*");
  });
  
  IAS.onModeButtonFirmwareUpdate([]() {
    Serial.println(F(" Checking if there is a firmware update available."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onModeButtonConfigMode([]() {
    Serial.println(F(" Starting configuration mode. Search for my WiFi and connect to 192.168.4.1."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });
  */


  //-------- Your Setup starts from here ---------------

}



// ================================================ LOOP =================================================
void loop() {
  IAS.buttonLoop();                                   // this routine handles the calling home functionality and reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration


  //-------- Your Sketch starts from here ---------------

  if (millis() - printEntry > 10000) {                // Serial.print the example variables every 10 seconds

    Serial.println(F(" LABEL\t\t| VAR\t\t| VALUE"));

    Serial.print(F(" Label 1\t\t| lbl1\t\t| "));
    Serial.println(lbl1);

    Serial.print(F(" Label 2\t\t| lbl2\t\t| "));
    Serial.println(lbl2);

    Serial.print(F(" Example url\t| exampleURL\t| "));
    Serial.println(exampleURL);

    Serial.print(F(" Timezone\t| timeZone\t| "));
    Serial.println(atof(timeZone));

    Serial.print(F(" Led pin\t\t| ledPin\t| "));
    Serial.println(atoi(ledPin));
    
    Serial.println(F("*-------------------------------------------------------------------------*"));
    printEntry = millis();
  }

}
