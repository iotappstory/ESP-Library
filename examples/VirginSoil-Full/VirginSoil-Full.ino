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

  virginSoilFull V2.2.2
*/

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                                        // Button pin on the esp for selecting modes. D3 for the Wemos!


#include <IOTAppStory.h>                                    // IotAppStory.com library
IOTAppStory IAS(COMPDATE, MODEBUTTON);                      // Initialize IotAppStory



// ================================================ EXAMPLE VARS =========================================
// used in this example to print variables every 10 seconds
unsigned long printEntry;
String deviceName = "virginsoil";
String chipId;

// We want to be able to edit these example variables below from the wifi config manager
// Currently only char arrays are supported.
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)

char* lbl         = "Light Show";
char* exampleURL  = "http://someapi.com/getdata.php?userid=1234&key=7890abc";
char* nrOf        = "6";

char* doSomething = "1";
char* chosen      = "0";

char* updInt      = "60";
char* ledPin      = "2";
char* timeZone    = "Europe/Amsterdam";



// ================================================ SETUP ================================================
void setup() {
  
  // create a unique deviceName for classroom situations (deviceName-123)
  chipId      = String(ESP_GETCHIPID);
  chipId      = "-"+chipId.substring(chipId.length()-3);
  deviceName += chipId;
	
  /* TIP! delete lines below when not used */
  IAS.preSetDeviceName(deviceName);                       	// preset deviceName this is also your MDNS responder: http://virginsoil-123.local
  //IAS.preSetAutoUpdate(false);                            // automaticUpdate (true, false)
  //IAS.preSetAutoConfig(false);                            // automaticConfig (true, false)
  //IAS.preSetWifi("ssid","password");                      // preset Wifi
  /* TIP! Delete Wifi cred. when you publish your App. */
	

  IAS.addField(lbl, "textLine", 16);                        // These fields are added to the "App Settings" page in config mode and saved to eeprom. Updated values are returned to the original variable.
  IAS.addField(exampleURL, "Textarea", 80, 'T');            // reference to org variable | field label value | max char return | Optional "special field" char
  IAS.addField(nrOf, "Number", 8, 'N');                     // Find out more about the optional "special fields" at https://iotappstory.com/wiki
  
  IAS.addField(doSomething, "Checkbox:Check me", 1, 'C');
  IAS.addField(chosen, "Selectbox:Red,Green,Blue", 1, 'S');

  IAS.addField(updInt, "Interval", 8, 'I');
  IAS.addField(ledPin, "ledPin", 2, 'P');
  IAS.addField(timeZone, "Timezone", 48, 'Z');
  


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
    Serial.println(F(" If mode button is released, I won't do anything unless you program me to."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    /* TIP! You can use this callback to put your app on it's own configuration mode */
  });

  IAS.onFirmwareUpdateProgress([](int written, int total){
      Serial.print(".");

      /*
      if(written%5==0){
        Serial.print(F("\n Written "));
        Serial.print(written);
        Serial.print(F(" of "));
        Serial.print(total);
      }
      */
  });
  
  /* 
  IAS.onModeButtonNoPress([]() {
    Serial.println(F(" Mode Button is not pressed."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });
  
  IAS.onFirstBoot([]() {                              
    Serial.println(F(" Run or display something on the first time this app boots"));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onFirmwareUpdateCheck([]() {
    Serial.println(F(" Checking if there is a firmware update available."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onFirmwareUpdateDownload([]() {
    Serial.println(F(" Downloading and Installing firmware update."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onFirmwareUpdateError([](String statusMessage) {
    Serial.println(F(" Update failed...Check your logs"));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onConfigMode([]() {
    Serial.println(F(" Starting configuration mode. Search for my WiFi and connect to 192.168.4.1."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });
  
  IAS.onFirstBoot([]() {
    IAS.eraseEEPROM('P');                   // Optional! What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase
  });
  */

	/* TIP! delete the lines above when not used */
 
  IAS.begin();                                            // Run IOTAppStory
  IAS.setCallHomeInterval(atoi(updInt));                  // Call home interval in seconds(disabled by default), 0 = off, use 60s only for development. Please change it to at least 2 hours in production


  //-------- Your Setup starts from here ---------------

}



// ================================================ LOOP =================================================
void loop() {
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)


  //-------- Your Sketch starts from here ---------------


  if (millis() - printEntry > 10000) {          // Serial.print the example variables every 10 seconds

    Serial.println(F(" LABEL\t\t| VAR\t\t| VALUE"));

    Serial.print(F(" textLine\t| lbl\t\t| "));
    Serial.println(lbl);
    Serial.print(F(" Textarea\t| exampleURL\t| "));
    Serial.println(exampleURL);
    Serial.print(F(" Number\t\t| nrOf\t\t| "));
    Serial.println(nrOf);

    Serial.print(F(" Checkbox\t| doSomething\t| "));
    Serial.println(doSomething);
    Serial.print(F(" Selectbox\t| chosenColor\t| "));
    Serial.println(chosen);

    Serial.print(F(" Interval\t| updInt\t| "));
    Serial.println(updInt);
    Serial.print(F(" Led pin\t| ledPin\t| "));
    Serial.println(atoi(ledPin));  
    Serial.print(F(" Timezone\t| timeZone\t| "));
    Serial.println(atof(timeZone));

    Serial.println(F("*-------------------------------------------------------------------------*"));
    printEntry = millis();
  }
}
