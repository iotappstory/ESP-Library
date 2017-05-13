
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

#define SKETCH "virginSoil "
#define VERSION "V2.1.1"
#define COMPDATE __DATE__ __TIME__
//#define LEDS_INVERSE          // LEDS on = GND



// ================================================ PIN DEFINITIONS ======================================
#ifdef ARDUINO_ESP8266_ESP01		// Generic ESP's 0-16  WeMos, NodeMCU etc D0-D16
  #define MODEBUTTON 0
  #define LEDgreen 13
  //#define LEDred 12
#else														// WeMos, NodeMCU etc D0-D16
  #define MODEBUTTON D3
  #define LEDgreen D7
  //#define LEDred D6
#endif
  
#include <IOTAppStory.h>
IOTAppStory IAS(SKETCH,VERSION,COMPDATE,MODEBUTTON);



// ================================================ EXAMPLE VARS =========================================
volatile unsigned long lastPrint;

// We want to be able to edit these example variables from the wifi config manager
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
  IAS.serialdebug(true);                                             // 1st parameter: true or false for serial debugging. Default: false
  //IAS.serialdebug(true,115200);                                    // 1st parameter: true or false for serial debugging. Default: false | 2nd parameter: serial speed. Default: 115200


  IAS.preSetConfig("VirginSoil-Full");                               // preset Boardname
  //IAS.preSetConfig("ssid","password");                             // preset Wifi
  //IAS.preSetConfig("ssid","password",true);                        // preset Wifi & automaticUpdate
  //IAS.preSetConfig("ssid","password","testboard");                 // preset Wifi & boardName
  //IAS.preSetConfig("ssid","password","testboard",true);            // preset Wifi, boardName & automaticUpdate
  /* TIP! Delete Wifi cred. when you publish your App. */


  IAS.addField(lbl1, "label1", "Label 1", 16);                       // These fields are added to the config wifimanager and saved to eeprom. Updated values are returned to the original variable.
  IAS.addField(lbl2, "label2", "Label 2", 16);                       // reference to org variable | field name | field label value | max char return
  IAS.addField(exampleURL, "url", "Example url", 80);
  IAS.addField(timeZone, "timezone", "Timezone", 4);
  IAS.addField(ledPin, "ledpin", "ledPin", 2);
  /* TIP! delete the lines above when not used */


  IAS.begin(true,LEDgreen);                                          // 1st parameter: true or false to view BOOT STATISTICS | 2nd parameter: green feedback led integer | 3rd argument attach interrupt for the mode selection button


  //-------- Your Setup starts from here ---------------
  
}



// ================================================ LOOP =================================================
void loop() {
  yield();
  IAS.routine();                                                     // this routine handles the reaction of the Flash button. If short press: update of skethc, long press: Configuration


  //-------- Your Sketch starts from here ---------------

  if (millis() - lastPrint > 10000) {                                // Serial.print the example variables every 5 seconds
    lastPrint = millis();

    Serial.print("Label 1: ");
    Serial.println(lbl1);

    Serial.print("Label 2: ");
    Serial.println(lbl2);

    Serial.print("Example url: ");
    Serial.println(exampleURL);

    Serial.print("Timezone: ");
    Serial.println(atof(timeZone));

    Serial.print("Led pin: ");
    Serial.println(atoi(ledPin));
    
    Serial.println("-------------------------------------------");
  }

}
