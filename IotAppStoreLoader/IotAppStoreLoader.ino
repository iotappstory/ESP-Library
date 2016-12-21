/* This sketch connects to the iopappstore and loads the assigned firmware down. The assignment is done on the server based on the MAC address of the board

    On the server, you need PHP script "iotappstore.php" and the bin files are in the .\bin folder

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

#define VERSION "v4.1"
#define FIRMWARE "IOTappstoryLoader "VERSION

#define SERIALDEBUG       // Serial is used to present debugging messages 
#define REMOTEDEBUGGING   // telnet is used to present
#define LEDSONBOARD
#define BOOTSTATISTICS    // send bootstatistics to Sparkfun



#include <credentials.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug
#include <WiFiManager.h>          //https://github.com/kentaylor/WiFiManager
ESP8266WebServer server(80);                  // The Webserver

extern "C" {
#include "user_interface.h" // this is for the RTC memory read/write functions
}

//---------- CODE DEFINITIONS ----------
#define MAXDEVICES 5
#define STRUCT_CHAR_ARRAY_SIZE 50  // length of config variables
#define SERVICENAME "LOADER"  // name of the MDNS service used in this group of ESPs


// -------- PIN DEFINITIONS ------------------
#ifdef ARDUINO_ESP8266_ESP01           // Generic ESP's 
#define LEDgreen 13
#define LEDred 12
#define PIRpin 14
#else
#define LEDgreen D7
#define LEDred D6
#define PIRpin D5
#endif


//-------- SERVICES --------------
ESP8266WebServer webServer(80);
WiFiClient client;
HTTPClient http;

// remoteDebug
#ifdef REMOTEDEBUGGING
RemoteDebug Debug;
#endif


//--------- ENUMS AND STRUCTURES  -------------------

typedef struct {
  char boardName[50];
  char IOTappStore1[40];
  char IOTappStorePHP1[40];
  char IOTappStore2[40];
  char IOTappStorePHP2[40];
  // insert NEW CONSTANTS according boardname example HERE!
  char magicBytes[4];
} strConfig;

strConfig config = {
  "INITloader",
  "192.168.0.200",
  "/iotappstore/iotappstorev20.php",
  "iotappstory.com",
  "/ota/esp8266-v1.php",
  "CFG"  // Magic Bytes
};

//---------- VARIABLES ----------
unsigned long entry;
String  boardName, IOTappStore1, IOTappStorePHP1, IOTappStore2, IOTappStorePHP2;  // add NEW CONSTANTS according boardname example

bool initialConfig = true;


//---------- FUNCTIONS ----------
void loopWiFiManager(void);
boolean readConfig(void);
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

 if (WiFi.status() == WL_CONNECTED) iotAppstory();

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
    WiFiManagerParameter p_IOTappStore1("IOTappStore1", "IOTappStore1", config.IOTappStore1, STRUCT_CHAR_ARRAY_SIZE);
    WiFiManagerParameter p_IOTappStorePHP1("IOTappStorePHP1", "IOTappStorePHP1", config.IOTappStorePHP1, STRUCT_CHAR_ARRAY_SIZE);
    WiFiManagerParameter p_IOTappStore2("IOTappStore2", "IOTappStore2", config.IOTappStore2, STRUCT_CHAR_ARRAY_SIZE);
    WiFiManagerParameter p_IOTappStorePHP2("IOTappStorePHP2", "IOTappStorePHP2", config.IOTappStorePHP2, STRUCT_CHAR_ARRAY_SIZE);

    // Just a quick hint
    WiFiManagerParameter p_hint("<small>*Hint: if you want to reuse the currently active WiFi credentials, leave SSID and Password fields empty</small>");

    // Initialize WiFIManager
    WiFiManager wifiManager;
    wifiManager.addParameter(&p_hint);

    //add all parameters here


    // Standard
    wifiManager.addParameter(&p_boardName);
    wifiManager.addParameter(&p_IOTappStore1);
    wifiManager.addParameter(&p_IOTappStorePHP1);
    wifiManager.addParameter(&p_IOTappStore2);
    wifiManager.addParameter(&p_IOTappStorePHP2);

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
    strcpy(config.IOTappStore1, p_IOTappStore1.getValue());
    strcpy(config.IOTappStorePHP1, p_IOTappStorePHP1.getValue());
    strcpy(config.IOTappStore2, p_IOTappStore2.getValue());
    strcpy(config.IOTappStorePHP2, p_IOTappStorePHP2.getValue());
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
