/* This sketch connects to the iopappstory and loads the assigned firmware down. The assignment is done on the server based on the MAC address of the board

    On the server, you need PHP script "iotappstory.php" and the bin files are in the .\bin folder

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

#define VERSION "v1.0.0"
#define FIRMWARE "IOTappStoryLoader "VERSION

#define SERIALDEBUG       // Serial is used to present debugging messages 
// #define REMOTEDEBUGGING   // telnet is used to present
#define LEDSONBOARD
#define BOOTSTATISTICS    // send bootstatistics to Sparkfun



#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>          //https://github.com/kentaylor/WiFiManager
#include <Ticker.h>

extern "C" {
#include "user_interface.h" // this is for the RTC memory read/write functions
}

//---------- CODE DEFINITIONS ----------
#define MAX_WIFI_RETRIES 50
#define STRUCT_CHAR_ARRAY_SIZE 50  // length of config variables
#define SERVICENAME "LOADER"  // name of the MDNS service used in this group of ESPs

#define RTCMEMBEGIN 68
#define MAGICBYTE 85


// -------- PIN DEFINITIONS ------------------
#ifdef ARDUINO_ESP8266_ESP01           // Generic ESP's 
#define GPIO0 0
#define LEDgreen 13
#define LEDred 12
#define PIRpin 14
#else
#define GPIO0 D3
#define LEDgreen D7
#define LEDred D6
#define PIRpin D5
#endif


//-------- SERVICES --------------

Ticker blink;

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
  char magicBytes[4];
} strConfig;

strConfig config = {
  "",
  "",
  "INITloader",
  "iotappstory.com",
  "/ota/esp8266-v1.php",
  "iotappstory.com",
  "/ota/esp8266-v1.php",
  "CFG"  // Magic Bytes
};

typedef struct {
  byte markerFlag;
  int bootTimes;
} rtcMemDef __attribute__((aligned(4)));
rtcMemDef rtcMem;


//---------- VARIABLES ----------
unsigned long entry;
String  boardName, IOTappStory1, IOTappStoryPHP1, IOTappStory2, IOTappStoryPHP2;  // add NEW CONSTANTS according boardname example

bool initialConfig = true;

volatile unsigned long buttonEntry, buttonTime;
volatile bool buttonChanged = false;
volatile int greenTimesOff = 0;
volatile int redTimesOff = 0;
volatile int greenTimes = 0;
volatile int redTimes = 0;


//---------- FUNCTIONS ----------
void loopWiFiManager(void);
void eraseFlash(void);


//---------- OTHER .H FILES ----------
#include "ESP_Helpers.h"


//-------------------------- SETUP -----------------------------------------
void setup() {

  Serial.begin(115200);

  for (int i = 0; i < 5; i++) Serial.println("");
  Serial.println("Start "FIRMWARE);
#ifdef REMOTEDEBUGGING
  Debug.println("Start "FIRMWARE);
#endif
  pinMode(GPIO0, INPUT_PULLUP);  // GPIO0 as input for Config mode selection

  eraseFlash();
  writeConfig();  // configuration in EEPROM

  initWiFiManager();

  if (WiFi.status() == WL_CONNECTED) IOTappStory();

  Serial.println("setup done");
}

// ----------------------- LOOP --------------------------------------------

void loop() {
  //-------- Standard Block ---------------
#ifdef REMOTEDEBUGGING
  Debug.handle();
#endif
  yield();
  if (initialConfig == true) loopWiFiManager();
  else LEDswitch(Green);
  //-------- Standard Block ---------------
}

//------------------------ END LOOP ------------------------------------------------

void loopWiFiManager() {  // new
  // is configuration portal requested?
  if ((digitalRead(GPIO0) == LOW) || (initialConfig)) {
    DEBUG_PRINTLN("Configuration portal requested");
#ifdef LEDSONBOARD
    LEDswitch(Red);  // turn the LED on by making the voltage LOW to tell us we are in configuration mode.
#endif

    //add all parameters here


    // Standard
    WiFiManagerParameter p_boardName("boardName", "boardName", config.boardName, STRUCT_CHAR_ARRAY_SIZE);
    WiFiManagerParameter p_IOTappStory1("IOTappStory1", "IOTappStory1", config.IOTappStory1, STRUCT_CHAR_ARRAY_SIZE);
    WiFiManagerParameter p_IOTappStoryPHP1("IOTappStoryPHP1", "IOTappStoryPHP1", config.IOTappStoryPHP1, STRUCT_CHAR_ARRAY_SIZE);
    WiFiManagerParameter p_IOTappStory2("IOTappStory2", "IOTappStory2", config.IOTappStory2, STRUCT_CHAR_ARRAY_SIZE);
    WiFiManagerParameter p_IOTappStoryPHP2("IOTappStoryPHP2", "IOTappStoryPHP2", config.IOTappStoryPHP2, STRUCT_CHAR_ARRAY_SIZE);

    // Just a quick hint
    WiFiManagerParameter p_hint("<small>*Hint: if you want to reuse the currently active WiFi credentials, leave SSID and Password fields empty</small>");

    // Initialize WiFIManager
    WiFiManager wifiManager;
    wifiManager.addParameter(&p_hint);

    //add all parameters here


    // Standard
    wifiManager.addParameter(&p_boardName);
    wifiManager.addParameter(&p_IOTappStory1);
    wifiManager.addParameter(&p_IOTappStoryPHP1);
    wifiManager.addParameter(&p_IOTappStory2);
    wifiManager.addParameter(&p_IOTappStoryPHP2);

    // Sets timeout in seconds until configuration portal gets turned off.
    // If not specified device will remain in configuration mode until
    // switched off via webserver or device is restarted.
    // wifiManager.setConfigPortalTimeout(600);

    // It starts an access point
    // and goes into a blocking loop awaiting configuration.
    // Once the user leaves the portal with the exit button
    // processing will continue
    if (!wifiManager.startConfigPortal(config.boardName)) {
      DEBUG_PRINTLN("Not connected to WiFi but continuing anyway.");
    } else {
      // If you get here you have connected to the WiFi
      DEBUG_PRINTLN("Connected... :-)");
    }
    // Getting posted form values and overriding local variables parameters

    //add all parameters here

    // Standard
    strcpy(config.boardName, p_boardName.getValue());
    strcpy(config.IOTappStory1, p_IOTappStory1.getValue());
    strcpy(config.IOTappStoryPHP1, p_IOTappStoryPHP1.getValue());
    strcpy(config.IOTappStory2, p_IOTappStory2.getValue());
    strcpy(config.IOTappStoryPHP2, p_IOTappStoryPHP2.getValue());
    writeConfig();

#ifdef LEDSONBOARD
    LEDswitch(None); // Turn LED off as we are not in configuration mode.
#endif

    ESP.reset();
  }
}

void eraseFlash() {
  Serial.println("Ereasing Flash...");
  EEPROM.begin(EEPROM_SIZE);
  for (unsigned int t = 0; t < EEPROM_SIZE; t++) EEPROM.write(t, 0);
  EEPROM.end();
}
