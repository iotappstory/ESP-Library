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
#define VERSION "V1.1"
#define FIRMWARE SKETCH VERSION

#define SERIALDEBUG         // Serial is used to present debugging messages 
#define REMOTEDEBUGGING     // UDP is used to transfer debug messages

#define LEDS_INVERSE   // LEDS on = GND

#include <credentials.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>        //https://github.com/kentaylor/WiFiManager
#include <Ticker.h>
#include <EEPROM.h>

#ifdef REMOTEDEBUGGING
#include <WiFiUDP.h>
#endif

extern "C" {
#include "user_interface.h" // this is for the RTC memory read/write functions
}

//--------  Sketch Specific -------



// -------- PIN DEFINITIONS ------------------
#ifdef ARDUINO_ESP8266_ESP01           // Generic ESP's 
#define MODEBUTTON 0
#define LEDgreen 13
//#define LEDred 12
#else
#define MODEBUTTON D3
#define LEDgreen D7
//#define LEDred D6
#endif

// --- Sketch Specific -----



//---------- DEFINES for SKETCH ----------
#define STRUCT_CHAR_ARRAY_SIZE 50  // length of config variables
#define MAX_WIFI_RETRIES 50
#define RTCMEMBEGIN 68
#define MAGICBYTE 85

// --- Sketch Specific -----
// #define SERVICENAME "VIRGIN"  // name of the MDNS service used in this group of ESPs


//-------- SERVICES --------------


// --- Sketch Specific -----



//--------- ENUMS AND STRUCTURES  -------------------

typedef struct {
  char ssid[STRUCT_CHAR_ARRAY_SIZE];
  char password[STRUCT_CHAR_ARRAY_SIZE];
  char boardName[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStory1[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStoryPHP1[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStory2[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStoryPHP2[STRUCT_CHAR_ARRAY_SIZE];
  char automaticUpdate[2];   // right after boot
  // insert NEW CONSTANTS according boardname example HERE!

  char magicBytes[4];

} strConfig;

strConfig config = {
  "",
  "",
  "yourFirstApp",
  "iotappstory.com",
  "/ota/esp8266-v1.php",
  "iotappstory.com",
  "/ota/esp8266-v1.php",
  "0",
  "CFG"  // Magic Bytes
};

// --- Sketch Specific -----



//---------- VARIABLES ----------

unsigned long debugEntry;

#ifdef REMOTEDEBUGGING
// UDP variables
char debugBuffer[255];
IPAddress broadcastIp(255, 255, 255, 255);
#endif
long counter = 0;
char boardMode = 'N';  // Normal operation or Configuration mode?
String sysMessage;

// --- Sketch Specific -----
// String xx; // add NEW CONSTANTS for WiFiManager according the variable "boardname"




//---------- FUNCTIONS ----------
// to help the compiler, sometimes, functions have  to be declared here
void loopWiFiManager(void);
void readFullConfiguration(void);
bool readRTCmem(void);
void printRTCmem(void);
void initialize(void);


//---------- OTHER .H FILES ----------
#include <ESP_Helpers.h>           // General helpers for all IOTappStory sketches
#include "IOTappStoryHelpers.h"    // Sketch specific helpers for all IOTappStory sketches




// ================================== SETUP ================================================

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 5; i++) DEBUG_PRINTLN("");
  DEBUG_PRINTLN("Start "FIRMWARE);


  // ----------- PINS ----------------
  pinMode(MODEBUTTON, INPUT_PULLUP);  // MODEBUTTON as input for Config mode selection

#ifdef LEDgreen
  pinMode(LEDgreen, OUTPUT);
  digitalWrite(LEDgreen, LEDOFF);
#endif
#ifdef LEDred
  pinMode(LEDred, OUTPUT);
  digitalWrite(LEDred, LEDOFF);
#endif

  // --- Sketch Specific -----



  // ------------- INTERRUPTS ----------------------------
  attachInterrupt(MODEBUTTON, ISRbuttonStateChanged, CHANGE);
  blink.detach();


  //------------- LED and DISPLAYS ------------------------
  LEDswitch(GreenBlink);


  // --------- BOOT STATISTICS ------------------------
  // read and increase boot statistics (optional)
  readRTCmem();
  rtcMem.bootTimes++;
  writeRTCmem();
  printRTCmem();


  //---------- SELECT BOARD MODE -----------------------------

  system_rtc_mem_read(RTCMEMBEGIN + 100, &boardMode, 1);   // Read the "boardMode" flag RTC memory to decide, if to go to config
  if (boardMode == 'C') configESP();

  readFullConfiguration();

  // --------- START WIFI --------------------------

  connectNetwork();

  sendSysLogMessage(2, 1, config.boardName, FIRMWARE, 10, counter++, "------------- Normal Mode -------------------");

  if (atoi(config.automaticUpdate) == 1) IOTappStory();



  // ----------- SPECIFIC SETUP CODE ----------------------------

  // add a DNS service
  // MDNS.addService(SERVICENAME, "tcp", 8080);  // just as an example

  // ----------- END SPECIFIC SETUP CODE ----------------------------

  LEDswitch(None);
  pinMode(MODEBUTTON, INPUT_PULLUP);  // MODEBUTTON as input for Config mode selection

 sendSysLogMessage(7, 1, config.boardName, FIRMWARE, 10, counter++, "Setup done");
}





//======================= LOOP =========================
void loop() {
  //-------- IOTappStory Block ---------------
  yield();
  handleModeButton();   // this routine handles the reaction of the Flash button. If short press: update of skethc, long press: Configuration

  // Normal blind (1 sec): Connecting to network
  // fast blink: Configuration mode. Please connect to ESP network
  // Slow Blink: IOTappStore Update in progress

  if (millis() - debugEntry > 5000) { // Non-Blocking second counter
    debugEntry = millis();
    sendDebugMessage();
  }


  //-------- Your Sketch ---------------



}
//------------------------- END LOOP --------------------------------------------

void sendDebugMessage() {
  // ------- Syslog Message --------

  /* severity: 2 critical, 6 info, 7 debug
    facility: 1 user level
    String hostName: Board Name
    app: FIRMWARE
    procID: unddefined
    msgID: counter
    message: Your message
  */

  sysMessage = "";
  long h1 = ESP.getFreeHeap();
  sysMessage += " Heap ";
  sysMessage += h1;
  sendSysLogMessage(6, 1, config.boardName, FIRMWARE, 10, counter++, sysMessage);
}




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


