
// this function is called by IOTappstory() before return. Here, you put a safe startup configuration
void initialize() {

}

void configESP() {
  Serial.begin(115200);
  for (int i = 0; i < 5; i++) DEBUG_PRINTLN("");
  DEBUG_PRINTLN("Start "FIRMWARE);

  // ----------- PINS ----------------
#ifdef LEDgreen
  pinMode(LEDgreen, OUTPUT);
  digitalWrite(LEDgreen, LEDOFF);
#endif
#ifdef LEDred
  pinMode(LEDred, OUTPUT);
  digitalWrite(LEDred, LEDOFF);
#endif


  // ------------- INTERRUPTS ----------------------------
  blink.detach();


  //------------- LED and DISPLAYS ------------------------
  LEDswitch(GreenBlink);

  connectNetwork();

  DEBUG_PRINTLN("------------- Configuration Mode -------------------");

  initWiFiManager();


  //--------------- LOOP ----------------------------------

  while (1) {
    yield();
    loopWiFiManager();
  }
}

void loopWiFiManager() {
  DEBUG_PRINTLN("Configuration portal requested");

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
  // If not specified, the device will remain in configuration mode until
  // switched off via webserver, or device is restarted.
  // wifiManager.setConfigPortalTimeout(600);

  // Starts an access point
  // and goes into a blocking loop awaiting configuration.
  // Once the user leaves the portal with the exit button
  // processing will continue
  if (!wifiManager.startConfigPortal(config.boardName)) {
    DEBUG_PRINTLN("Not connected to WiFi but continuing anyway.");
  } else {
    // If you get here you have connected to the WiFi
    DEBUG_PRINTLN("Connected to WiFi... :-)");
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
Serial.println("Going to IOTappStory.com");
  IOTappStory();
}
