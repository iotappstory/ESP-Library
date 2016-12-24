/* This sketch connects to iopappstory.com and loads the assigned firmware down
    This work is based on the ESPhttpUpdate examples

    To add new constants in WiFiManager search for "NEW CONSTANTS" and insert them according the "boardName" example

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

#define VERSION "V1.0"
#define FIRMWARE "SLOWBLINK "VERSION

#define SERIALDEBUG         // Serial is used to present debugging messages 
#define REMOTEDEBUGGING     // telnet is used to present
//#define BOOTSTATISTICS    // send bootstatistics to Sparkfun


#include <credentials.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug
#include <WiFiManager.h>        //https://github.com/kentaylor/WiFiManager
#include <Ticker.h>

extern "C" {
#include "user_interface.h" // this is for the RTC memory read/write functions
}


// -------- PIN DEFINITIONS ------------------
#ifdef ARDUINO_ESP8266_ESP01           // Generic ESP's 
#define GPIO0 0
#else
#define GPIO0 D3
#endif

//---------- CODE DEFINITIONS ----------
#define MAXDEVICES 5
#define STRUCT_CHAR_ARRAY_SIZE 50  // length of config variables
#define SERVICENAME "FASTBLINK"  // name of the MDNS service used in this group of ESPs
#define MAX_WIFI_RETRIES 50

#define RTCMEMBEGIN 68
#define MAGICBYTE 85


//-------- SERVICES --------------

//WiFiServer server(80);
Ticker blink;
WiFiClientSecure client;

// remoteDebug
#ifdef REMOTEDEBUGGING
RemoteDebug Debug;
#endif


//--------- ENUMS AND STRUCTURES  -------------------

typedef struct {
  char ssid[STRUCT_CHAR_ARRAY_SIZE];
  char password[STRUCT_CHAR_ARRAY_SIZE];
  char boardName[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStory1[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStoryPHP1[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStory2[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStoryPHP2[STRUCT_CHAR_ARRAY_SIZE];
  // insert NEW CONSTANTS according boardname example HERE!
  char blinkPin[STRUCT_CHAR_ARRAY_SIZE];
  char magicBytes[4];
} strConfig;

strConfig config = {
  mySSID,
  myPASSWORD,
  "FASTBLINK",
  "iotappstory.org",
  "ota/esp8266-v1.php",
  "iotappstory.org",
  "ota/esp8266-v1.php",
  "",
  "CFG"  // Magic Bytes
};

typedef struct {
  byte markerFlag;
  int bootTimes;
} rtcMemDef __attribute__((aligned(4)));
rtcMemDef rtcMem;

//---------- VARIABLES ----------

String boardName, IOTappStory1, IOTappStoryPHP1, IOTappStory2, IOTappStoryPHP2, IFTTTchannel; // add NEW CONSTANTS according boardname example

long delayCount = -1;

char boardMode = 'N';  // Normal operation or Configuration mode?

volatile unsigned long buttonEntry, buttonTime;
volatile bool buttonChanged = false;
volatile int greenTimesOff = 0;
volatile int redTimesOff = 0;
volatile int greenTimes = 0;
volatile int redTimes = 0;

unsigned long infoEntry, blinkEntry;

byte LEDpin;


int delayCounter = 0; // counts every second



//---------- FUNCTIONS ----------
void loopWiFiManager(void);
void readFullConfiguration(void);
bool readRTCmem(void);
void printRTCmem(void);
void switchRelay(bool);

//---------- OTHER .H FILES ----------
#include <ESP_Helpers.h>
#include "WiFiManager_Helpers.h"


//-------------------------- SETUP -----------------------------------------

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 5; i++) DEBUG_PRINTLN("");
  DEBUG_PRINTLN("Start "FIRMWARE);


  // ----------- PINS ----------------
  pinMode(GPIO0, INPUT_PULLUP);  // GPIO0 as input for Config mode selection


  // ------------- INTERRUPTS ----------------------------
  attachInterrupt(GPIO0, ISRbuttonStateChanged, CHANGE);



  //------------- LED and DISPLAYS ------------------------


  // --------- BOOT STATISTICS ------------------------
  // read and increase boot statistics (optional)
  readRTCmem();
  rtcMem.bootTimes++;
  writeRTCmem();
  printRTCmem();


  //---------- BOARD MODE -----------------------------

  system_rtc_mem_read(RTCMEMBEGIN + 100, &boardMode, 1);   // Read the "boardMode" flag RTC memory to decide, if to go to config
  if (boardMode == 'C') configESP();


  DEBUG_PRINTLN("------------- Normal Mode -------------------");

  // --------- START WIFI --------------------------
  readFullConfiguration();
  WiFi.mode(WIFI_STA);
  WiFi.begin();

  int retries = MAX_WIFI_RETRIES;
  while (WiFi.status() != WL_CONNECTED && retries-- > 0 ) {
    delay(500);
    Serial.print(".");
  }
  if (retries >= MAX_WIFI_RETRIES || WiFi.psk() == "") {
    DEBUG_PRINTLN("NoConn");
    if ( WiFi.psk() == "") espRestart('C', "No Connection...");
    else espRestart('N', "No Connection...");
  } else {

    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("WiFi connected");
    getMACaddress();
    printMacAddress();
    DEBUG_PRINT("IP Address: ");
    DEBUG_PRINTLN(WiFi.localIP());

#ifdef REMOTEDEBUGGING
    remoteDebugSetup();
    REMOTEDEBUG_PRINTLN(config.boardName);
#endif

    registerDNS();
    // ----------- SPECIFIC SETUP CODE ----------------------------

    String blinkPin(config.blinkPin);
    if (blinkPin == "D0")  LEDpin = D0;
    else {
      if (blinkPin == "D1")  LEDpin = D1;
      else {
        if (blinkPin == "D2")  LEDpin = D2;
        else {
          if (blinkPin == "D4")  LEDpin = D4;
          else {
            if (blinkPin == "D5")  LEDpin = D5;
            else {
              if (blinkPin == "D6")  LEDpin = D6;
              else {
                if (blinkPin == "D7")  LEDpin = D7;
                else LEDpin = D8;
              }
            }
          }
        }
      }
    }


    pinMode(LEDpin, OUTPUT);

    Serial.print("LED Pin ");
    Serial.println(LEDpin);

    // ----------- END SPECIFIC SETUP CODE ----------------------------

  }  // End WiFi necessary


  DEBUG_PRINTLN("setup done");
}
//--------------- LOOP ----------------------------------



void loop() {
  //-------- Standard Block ---------------
  if (buttonChanged && buttonTime > 4000) espRestart('C', "Going into Configuration Mode");  // long button press > 4sec
  if (buttonChanged && buttonTime > 500 && buttonTime < 4000) IOTappStory(); // long button press > 1sec
  buttonChanged = false;
#ifdef REMOTEDEBUGGING
  Debug.handle();
  yield();
  //-------- End Standard Block ---------------

  // ------- Debug Message --------
  if (Debug.ative(Debug.INFO) && (millis() - infoEntry) > 2000) {
    Debug.printf("Firmware: %s", FIRMWARE);
    Debug.printf(" LED Pin: %d", LEDpin);
    Debug.printf(" BlinkEntry: %d", blinkEntry);
    Debug.println(" mSec:");

    Debug.print("Heap ");
    Debug.println(ESP.getFreeHeap());
    infoEntry = millis();
  }
#endif
  // ------------------------------------

  if (millis() - blinkEntry > 100) {
    digitalWrite(LEDpin, !digitalRead(LEDpin));
    blinkEntry = millis();
  }

}
//------------------------- END LOOP --------------------------------------------


void readFullConfiguration() {
  readConfig();  // configuration in EEPROM
  // insert NEW CONSTANTS according switchName1 example
}


bool readRTCmem() {
  bool ret = true;
  system_rtc_mem_read(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
  if (rtcMem.markerFlag != MAGICBYTE) {
    rtcMem.markerFlag = MAGICBYTE;
    rtcMem.bootTimes = 0;
    system_rtc_mem_write(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
    ret = false;
  }
  return ret;
}

void printRTCmem() {
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("rtcMem ");
  DEBUG_PRINT("markerFlag ");
  DEBUG_PRINTLN(rtcMem.markerFlag);
  DEBUG_PRINT("bootTimes ");
  DEBUG_PRINTLN(rtcMem.bootTimes);
}

