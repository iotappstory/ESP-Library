

#include <credentials.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug
#include <WiFiManager.h>        //https://github.com/kentaylor/WiFiManager
#include <Ticker.h>

extern "C" {
#include "user_interface.h" // this is for the RTC memory read/write functions
}


// -------- PIN DEFINITIONS ------------------


//---------- CODE DEFINITIONS ----------



//-------- SERVICES --------------



//--------- ENUMS AND STRUCTURES  -------------------


//---------- VARIABLES ----------



//---------- FUNCTIONS ----------


//---------- OTHER .H FILES ----------



//--------------- START ----------------------------------
void configESP() {
  Serial.begin(115200);
  for (int i = 0; i < 5; i++) DEBUG_PRINTLN("");
  DEBUG_PRINTLN("Start "FIRMWARE);
  DEBUG_PRINTLN("------------- Configuration Mode -------------------");
  for (int i = 0; i < 3; i++) DEBUG_PRINTLN("");

  pinMode(GPIO0, INPUT_PULLUP);  // GPIO0 as input for Config mode selection

#ifdef LEDgreen
  pinMode(LEDgreen, OUTPUT);
#endif
#ifdef LEDred
  pinMode(LEDred, OUTPUT);
#endif

  LEDswitch(GreenFastBlink);

  readFullConfiguration();  // configuration in EEPROM

  initWiFiManager();


  //--------------- LOOP ----------------------------------

  while (1) {
    if (buttonChanged && buttonTime > 4000) espRestart('N', "Back to normal mode");  // long button press > 4sec
    yield();
    loopWiFiManager();
  }
}


void loopWiFiManager() {

  //add all parameters here


  // Standard
  WiFiManagerParameter p_boardName("boardName", "boardName", config.boardName, STRUCT_CHAR_ARRAY_SIZE);
  WiFiManagerParameter p_IOTappStory1("IOTappStory1", "IOTappStory1", config.IOTappStory1, STRUCT_CHAR_ARRAY_SIZE);
  WiFiManagerParameter p_IOTappStoryPHP1("IOTappStoryPHP1", "IOTappStoryPHP1", config.IOTappStoryPHP1, STRUCT_CHAR_ARRAY_SIZE);
  WiFiManagerParameter p_IOTappStory2("IOTappStory2", "IOTappStory2", config.IOTappStory2, STRUCT_CHAR_ARRAY_SIZE);
  WiFiManagerParameter p_IOTappStoryPHP2("IOTappStoryPHP2", "IOTappStoryPHP2", config.IOTappStoryPHP2, STRUCT_CHAR_ARRAY_SIZE);

  //add all parameters here
  WiFiManagerParameter p_blinkPin("blinkPin", "blinkPin", config.blinkPin, STRUCT_CHAR_ARRAY_SIZE);

  // Just a quick hint
  WiFiManagerParameter p_hint("<small>*Hint: if you want to reuse the currently active WiFi credentials, leave SSID and Password fields empty</small>");

  // Initialize WiFIManager
  WiFiManager wifiManager;
  wifiManager.addParameter(&p_hint);

  //add all parameters here

  // Standard
  wifiManager.addParameter(&p_boardName);

  //add all parameters here
  wifiManager.addParameter(&p_blinkPin);

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
  // Config file is written

  //add all parameters here


  // Standard
  strcpy(config.boardName, p_boardName.getValue());
  strcpy(config.IOTappStory1, p_IOTappStory1.getValue());
  strcpy(config.IOTappStoryPHP1, p_IOTappStoryPHP1.getValue());
  strcpy(config.IOTappStory2, p_IOTappStory2.getValue());
  strcpy(config.IOTappStoryPHP2, p_IOTappStoryPHP2.getValue());

  //add all parameters here
  strcpy(config.blinkPin, p_blinkPin.getValue());
  writeConfig();

  LEDswitch(None); // Turn LED off as we are not in configuration mode.

  espRestart('N', "Configuration finished"); //Normal Operation

}
