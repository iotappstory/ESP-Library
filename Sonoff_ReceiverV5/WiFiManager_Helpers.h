extern "C" {
#include "user_interface.h" // this is for the RTC memory read/write functions
}


// -------- PIN DEFINITIONS ------------------


//---------- CODE DEFINITIONS ----------
#define MAXDEVICES 5
#define STRUCT_CHAR_ARRAY_SIZE 50  // length of config variables
#define SERVICENAME "SONOFF"  // name of the MDNS service used in this group of ESPs



//-------- SERVICES --------------



//--------- ENUMS AND STRUCTURES  -------------------


//---------- VARIABLES ----------



//---------- FUNCTIONS ----------


//---------- OTHER .H FILES ----------



//--------------- START ----------------------------------
void configESP() {
  Serial.begin(115200);
  for (int i = 0; i < 5; i++) DEBUG_PRINTLN("");
  DEBUG_PRINTLN("Start " FIRMWARE);
  DEBUG_PRINTLN("------------- Configuration Mode -------------------");
  for (int i = 0; i < 3; i++) DEBUG_PRINTLN("");

  pinMode(GPIO0, INPUT_PULLUP);  // GPIO0 as input for Config mode selection

#ifdef LEDgreen
  pinMode(LEDgreen, OUTPUT);
  LEDswitch(GreenFastBlink);
#endif
#ifdef LEDred
  pinMode(LEDred, OUTPUT);
#endif


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
  // Config file is written

  //add all parameters here


  // Standard
  strcpy(config.boardName, p_boardName.getValue());
  strcpy(config.IOTappStore1, p_IOTappStore1.getValue());
  strcpy(config.IOTappStorePHP1, p_IOTappStorePHP1.getValue());
  strcpy(config.IOTappStore2, p_IOTappStore2.getValue());
  strcpy(config.IOTappStorePHP2, p_IOTappStorePHP2.getValue());
  writeConfig();

  LEDswitch(None); // Turn LED off as we are not in configuration mode.

  espRestart('N', "Configuration finished"); //Normal Operation

}
