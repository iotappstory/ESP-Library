
// This is an example sketch from the ezTime arduino library adapted to use IOTAppStory.
// Added multi WiFi management and over the air update functionality.

// 2019  C.W.L Broeders


// MIT License

// Copyright (c) 2018 R. Gonggrijp

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.



#include <ezTime.h>                                         // https://github.com/ropg/ezTime
#include <IOTAppStory.h>                                    // IotAppStory.com library

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                                        // Button pin on the esp for selecting modes. D3 for the Wemos!


IOTAppStory IAS(COMPDATE, MODEBUTTON);                      // Initialize IOTAppStory

// ================================================ EXAMPLE VARS =========================================
// We want to be able to edit the example variable below from the wifi config manager
// Currently only char arrays are supported.
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)

char* hostName = "timeformats";



// ================================================ SETUP ================================================
void setup() {
  IAS.preSetDeviceName(hostName);                           // preset deviceName this is also your MDNS responder: http://timeformats.local
  IAS.addField(hostName, "textLine", 16);                   // These fields are added to the "App Settings" page in config mode and saved to eeprom. Updated values are returned to the original variable.



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


  IAS.begin('P');                                           // Optional parameter: What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase(default) | 'L' Leave intact
	waitForSync();

	Serial.println();
	Serial.println("Time in various internet standard formats ...");
	Serial.println();
	Serial.println("ATOM:        " + dateTime(ATOM));
	Serial.println("COOKIE:      " + dateTime(COOKIE));
	Serial.println("IS8601:      " + dateTime(ISO8601));
	Serial.println("RFC822:      " + dateTime(RFC822));
	Serial.println("RFC850:      " + dateTime(RFC850));
	Serial.println("RFC1036:     " + dateTime(RFC1036));
	Serial.println("RFC1123:     " + dateTime(RFC1123));
	Serial.println("RFC2822:     " + dateTime(RFC2822));
	Serial.println("RFC3339:     " + dateTime(RFC3339));
	Serial.println("RFC3339_EXT: " + dateTime(RFC3339_EXT));
	Serial.println("RSS:         " + dateTime(RSS));
	Serial.println("W3C:         " + dateTime(W3C));
	Serial.println();
	Serial.println(" ... and any other format, like \"" + dateTime("l ~t~h~e jS ~o~f F Y, g:i A") + "\"");

}

// ================================================ LOOP =================================================
void loop() {
    IAS.loop();
	events();
}
