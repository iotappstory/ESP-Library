/* 
  This is an initial sketch to be used as a "blueprint" to create apps which can be used with IOTappstory.com infrastructure
  Your code can be filled wherever it is marked.
  
  Copyright (c) [2017] [Andreas Spiess]
  
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

#define SKETCH "Slow blinkz "
#define VERSION "V2.2.0"
#define COMPDATE __DATE__ __TIME__

// ================================================ PIN DEFINITIONS ======================================
#ifdef ARDUINO_ESP8266_ESP01    // Generic ESP's 0-16  WeMos, NodeMCU etc D0-D16
  #define MODEBUTTON 0
  #define LEDgreen 13
  //#define LEDred 12   
#else                           // WeMos, NodeMCU etc D0-D16
  #define MODEBUTTON D3
  #define LEDgreen D7
  //#define LEDred D6
#endif
  
#include <IOTAppStory.h>
IOTAppStory IAS(SKETCH,VERSION,COMPDATE,MODEBUTTON);



// ================================================ EXAMPLE VARS =========================================
unsigned long blinkEntry, debugEntry;
byte blinkState = 0;
byte prevBlinkState = 1;
long iLongDelay = 1000;

// We want to be able to edit these example variables from the wifi config manager
// Currently only char arrays are supported. (Keep in mind that html form fields always return Strings)
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)

char* LEDpin = "D6";             // The value given here is the default value and can be overwritten by values saved in configuration mode
char* longDelay = "200";

int statusLEDPin = D4;

// ================================================ SETUP ================================================
void setup() {
  IAS.serialdebug(true);               // 1st parameter: true or false for serial debugging. Default: false | When set to true or false serialdebug can be set from wifi config manager
  //IAS.serialdebug(true,115200);      // 1st parameter: true or false for serial debugging. Default: false | 2nd parameter: serial speed. Default: 115200
  /* TIP! delete the above lines when not used */

  IAS.preSetConfig("slowblink",true); // preset Boardname

  IAS.addField(LEDpin, "ledpin", "LED pin", 2);             // These fields are added to the config wifimanager and saved to eeprom. Updated values are returned to the original variable.
  IAS.addField(longDelay, "longDelay", "Long delay", 3);      // These fields are added to the config wifimanager and saved to eeprom. Updated values are returned to the original variable.
                                                              // reference to org variable | field name | field label value | max char return

  IAS.begin(true);                     // 1st parameter: true or false to view BOOT STATISTICS | 2nd parameter: green feedback led integer

  //-------- Your Setup starts from here ---------------
  if (atoi(longDelay) > 0)
    iLongDelay = atoi(longDelay);
    
  pinMode(IAS.dPinConv(LEDpin), OUTPUT);
  
  pinMode(statusLEDPin, OUTPUT);
}



// ================================================ LOOP =================================================
void loop() {
  yield();
  IAS.routine();            // this routine handles the reaction of the Flash button. If short press: update of skethc, long press: Configuration

  //-------- Your Sketch starts from here ---------------
  if (millis() - blinkEntry < 200)
    digitalWrite(IAS.dPinConv(LEDpin), HIGH);
  else if (millis() - blinkEntry < iLongDelay + 200)
    digitalWrite(IAS.dPinConv(LEDpin), LOW);
  else
    blinkEntry = millis();

  if (millis() - debugEntry > 1000)
  {
    debugEntry = millis();

    // check the status of the flash button
    switch(IAS.getModeButtonStatus())
    {
    case MODE_BUTTON_SHORT_PRESS:
      Serial.println("SHORT");
      digitalWrite(statusLEDPin, LOW);
      delay(200);
      digitalWrite(statusLEDPin, HIGH);
      break;
    
    case MODE_BUTTON_LONG_PRESS:
      Serial.println("LONG");
      digitalWrite(statusLEDPin, LOW);
      delay(50);
      digitalWrite(statusLEDPin, HIGH);
      delay(100);
      digitalWrite(statusLEDPin, LOW);
      delay(50);
      digitalWrite(statusLEDPin, HIGH);
      break;
      
    case MODE_BUTTON_NO_PRESS:
      break;
    }
  }
}
