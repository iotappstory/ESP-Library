
/* This is an initial sketch to be used as a "blueprint" to create apps which can be used with IOTappstory.com infrastructure
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
//#define LEDS_INVERSE          // LEDS on = GND

// ================================================ PIN DEFINITIONS ======================================
#ifdef ARDUINO_ESP8266_ESP01  // Generic ESP's 
  #define MODEBUTTON 0
  #define LEDgreen 13
  //#define LEDred 12
#else
  #define MODEBUTTON D3
  #define LEDgreen D7
  //#define LEDred D6
#endif
  
#include <IOTAppStory.h>
#include <IAS_Xtra_Func.h>
IOTAppStory IAS(SKETCH,VERSION,MODEBUTTON);

// ================================================ EXAMPLE VARS =========================================
char* lbl1 = "Light Show";                                                                                    // default value | this gets stored to eeprom and gets updated if changed from wifi config
char* lbl2 = "Living Room";
char* lbl3 = "Bath Room";
char* lbl4 = "Test Room";
char* lbl5 = "Another Room";

// ================================================ SETUP ================================================
void setup() {
  IAS.serialdebug(true);                                                                                      // 1st parameter: true or false for serial debugging. Default: false
  //IAS.serialdebug(true,115200);                                                                             // 1st parameter: true or false for serial debugging. Default: false | 2nd parameter: serial speed. Default: 115200

  //IAS.preSetConfig("yourBoardName");                                                                        // preset Boardname
  //IAS.preSetConfig("ssid","password");                                                                      // preset Wifi
  //IAS.preSetConfig("ssid","password",true);                                                                 // preset Wifi & automaticUpdate
  //IAS.preSetConfig("ssid","password","testboard");                                                          // preset Wifi & boardName
  //IAS.preSetConfig("ssid","password","testboard",true);                                                     // preset Wifi, boardName & automaticUpdate
  //IAS.preSetConfig("ssid","password","testboard","iotappstory.com","/ota/esp8266-v1.php");                  // preset Wifi, boardName, IOTappStory1 & IOTappStoryPHP1
  //IAS.preSetConfig("ssid","password","testboard","iotappstory.com","/ota/esp8266-v1.php",true);             // preset Wifi, boardName, IOTappStory1, IOTappStoryPHP1 & automaticUpdate

  IAS.addField(lbl1, "label1", "Label 1", 16);                                                                // reference to org variable | field name | field label value | max char return
  IAS.addField(lbl2, "label2", "Label 2", 16);
  IAS.addField(lbl3, "label3", "Label 3", 16);
  IAS.addField(lbl4, "label4", "Label 4", 16);
  IAS.addField(lbl5, "label5", "Label 5", 16);
  /* TIP! delete the above lines when not used */

  IAS.begin(p,true,LEDgreen);                                                                                  // 1st parameter: true or false to view BOOT STATISTICS | 2nd parameter: green feedback led integer | 3rd argument attach interrupt for the mode selection button


  //-------- Your Setup starts from here ---------------
  
}


// ================================================ LOOP =================================================
void loop() {
  yield();
  IAS.routine(org_buttonEntry,org_buttonTime,org_buttonChanged);                                               // this routine handles the reaction of the Flash button. If short press: update of skethc, long press: Configuration


  //-------- Your Sketch starts from here ---------------

}
