

#define MAGICBYTES "CFG"
#define EEPROM_SIZE 1024

#define ON true
#define OFF false
  
// macros for debugging
#ifdef SERIALDEBUG
#define         DEBUG_PRINT(x)    Serial.print(x)
#define         DEBUG_PRINTLN(x)  Serial.println(x)
#else
#define         DEBUG_PRINT(x)
#define         DEBUG_PRINTLN(x)
#endif


#ifdef REMOTEDEBUGGING
#define         REMOTEDEBUG_PRINT(x)    Debug.print(x)
#define         REMOTEDEBUG_PRINTLN(x)  Debug.println(x)
#else
#define         REMOTEDEBUG_PRINT(x)
#define         REMOTEDEBUG_PRINTLN(x)
#endif



#ifdef LEDS_INVERSE
#define LEDON  0
#define LEDOFF 1
#else
#define LEDON  1
#define LEDOFF 0
#endif


//---------- COMMON DEFINITIONS ------------
enum ledColorDef {
  None,
  Green,
  Red,
  Both,
  GreenSlowBlink,
  RedSlowBlink,
  RedBlink,
  GreenBlink,
  GreenFastBlink,
  RedFastBlink
};

/*
typedef struct {
  byte markerFlag;
  long lastSubscribers;
  int updateSpaces;
  int runSpaces;
  int bootTimes;
} rtcMemDef __attribute__((aligned(4)));
rtcMemDef rtcMem;*/



//---------- MISC FUNCTIONS ----------

/*
void ESPrestart(String message){
    DEBUG_PRINTLN(message);
    DEBUG_PRINTLN("R E S E T");
    #ifdef REMOTEDEBUGGING
    Debug.println(message);
    Debug.println("R E S E T");
    #endif
    ESP.restart();
}
*/

// Wait till networl is connected. Returns false if not connected after MAX_WIFI_RETRIES retries
bool isNetworkConnected() {
  int retries = MAX_WIFI_RETRIES;
  while (WiFi.status() != WL_CONNECTED && retries-- > 0 ) {
    delay(500);
    Serial.print(".");
  }
  if (retries <= 0) return false;
  else return true;
}

void ISRbuttonStateChanged() {
  if (digitalRead(GPIO0) == 0) buttonEntry = millis();
  else {
    buttonTime = millis() - buttonEntry;
    buttonChanged = true;
  }
}

void espRestart(char mmode, String message) {
   DEBUG_PRINTLN(message);
#ifdef REMOTEDEBUGGING
   Debug.println(message);
#endif
  while (digitalRead(GPIO0) == OFF) yield();    // wait till GPIOo released
  delay(500);
  system_rtc_mem_write(RTCMEMBEGIN + 100, &mmode, 1);
  ESP.restart();
}

String getMACaddress() {
  uint8_t mac[6];
  char macStr[18] = {0};
  WiFi.macAddress(mac);
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],  mac[1], mac[2], mac[3], mac[4], mac[5]);
  return  String(macStr);
}

void printMacAddress() {
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  for (int i = 0; i < 5; i++) {
    Serial.print(mac[i], HEX);
    Serial.print(":");
  }
  Serial.println(mac[5], HEX);
}


void registerDNS(){
    // Register host name in WiFi and mDNS
    String hostNameWifi = boardName;   // boardName is device name
    hostNameWifi.concat(".local");
    WiFi.hostname(hostNameWifi);
    if (MDNS.begin(config.boardName)) {
      DEBUG_PRINT("* MDNS responder started. http://");
      DEBUG_PRINTLN(hostNameWifi);
    }
}

//---------- LED FUNCTIONS ----------



void tickGreen() {
  #ifdef LEDgreen
  if (greenTimes%greenTimesOff==0) digitalWrite(LEDgreen,LEDON);
  else digitalWrite(LEDgreen,LEDOFF);
  greenTimes++;
  #endif
}

void tickRed() {
  #ifdef LEDred
  if (redTimes%redTimesOff==0) digitalWrite(LEDred,LEDON);
  else digitalWrite(LEDred,LEDOFF);
  redTimes++;
  #endif
}

void greenFlash(float takt, int timesOff){
  greenTimesOff = timesOff;
  blink.attach(takt, tickGreen);
}

void redFlash(float takt, int timesOff){
  redTimesOff = timesOff;
  blink.attach(takt, tickRed);
}


void LEDswitch(ledColorDef color) {

/*
none: 0.1 sec on, 3.6 sec off;
Green, red, and both: LEDs always on
..SlowBlink: 2 sec on, 2 sec off;
..Blink: 0.5 sec on, o.5 sec off
...FastBlink: 0.1 sec on, 0.1 sec off
*/

    blink.detach();
    
    switch (color) {
    case None:
       #ifdef LEDgreen
       greenFlash(0.1, 36);
       #endif

       break;
    case Green:
       #ifdef LEDgreen
       digitalWrite(LEDgreen, LEDON);
       #endif
       #ifdef LEDred
       digitalWrite(LEDred, LEDOFF);
       #endif
       break;
    case Red:
       #ifdef LEDgreen
       digitalWrite(LEDgreen, LEDOFF);
       #endif
       #ifdef LEDred
       digitalWrite(LEDred, LEDON);
       #endif
       break;
    case Both:
       #ifdef LEDgreen
       digitalWrite(LEDgreen, LEDON);
       #endif
       #ifdef LEDred
       digitalWrite(LEDred, LEDON);
       #endif
       break;
    case GreenSlowBlink:
       #ifdef LEDgreen
       greenFlash(2.0, 2);
       #endif
       #ifdef LEDred
       digitalWrite(LEDred, LEDOFF);
       #endif
       break;
    case RedSlowBlink:
       #ifdef LEDgreen
       digitalWrite(LEDgreen, LEDOFF);
       #endif
       #ifdef LEDred
       redFlash(2.0, 2);
       #endif
       break;
    case GreenBlink:
       greenFlash(0.5, 2);
       #ifdef LEDred
       digitalWrite(LEDred, LEDOFF);
       #endif
       break;
    case RedBlink:
       #ifdef LEDgreen
       digitalWrite(LEDgreen, LEDOFF);
       #endif
       redFlash(0.5, 2);
       break;
    case GreenFastBlink:
       #ifdef LEDgreen
       greenFlash(0.1, 2);
       #endif
       #ifdef LEDred
       digitalWrite(LEDred, LEDOFF);
       #endif
       break;
    case RedFastBlink:
       #ifdef LEDgreen
       digitalWrite(LEDgreen, LEDOFF);
       #endif
       #ifdef LEDred
       redFlash(0.1, 2);
       #endif
       break;
    default:
       break;
    }
}


//---------- RTC MEMORY FUNCTIONS ----------
/*bool readRTCmem() {
  bool ret = true;
  system_rtc_mem_read(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
  if (rtcMem.markerFlag != MAGICBYTE ) {
    rtcMem.markerFlag = MAGICBYTE;
    rtcMem.bootTimes = 0;
    system_rtc_mem_write(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
    ret = false;
  }
  return ret;
}*/

void writeRTCmem() {
  rtcMem.markerFlag = MAGICBYTE;
  system_rtc_mem_write(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
}

/*void printRTCmem() {
  DEBUG_PRINT("BootTimes ");
  DEBUG_PRINTLN(rtcMem.bootTimes);
}*/


//---------- IOTappStory FUNCTIONS ----------
bool iotUpdater(String server, String url, String firmware, bool immediately, bool debugWiFi) {
  bool retValue = true;
  
    delay(1000);
    DEBUG_PRINTLN("------------- IOT Appstory MODE -------------------");
    REMOTEDEBUG_PRINTLN("------------- IOT Appstory Mode -------------------");

  if (debugWiFi) {
    getMACaddress();
    printMacAddress();
    DEBUG_PRINT("IP = ");
    DEBUG_PRINTLN(WiFi.localIP());
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("Updating from...");
    DEBUG_PRINT("Update_server ");
    DEBUG_PRINTLN(server);
    DEBUG_PRINT("UPDATE_URL ");
    DEBUG_PRINTLN(url);
    DEBUG_PRINT("FIRMWARE_VERSION ");
    DEBUG_PRINTLN(firmware);
  }
  t_httpUpdate_return ret = ESPhttpUpdate.update(server, 80, url, firmware);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      retValue = false;
      if (debugWiFi) Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      DEBUG_PRINTLN();
      break;

    case HTTP_UPDATE_NO_UPDATES:
      if (debugWiFi) DEBUG_PRINTLN("---------- HTTP_UPDATE_NO_UPDATES ------------------");      
      break;

    case HTTP_UPDATE_OK:
      if (debugWiFi) DEBUG_PRINTLN("HTTP_UPDATE_OK");
      break;
  }
  return retValue;
}

void IOTappStory(){
 // update from IOTappStory.com
  if (iotUpdater(config.IOTappStory1, config.IOTappStoryPHP1, FIRMWARE, true, true) == false) {
   DEBUG_PRINTLN("False !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    if (iotUpdater(config.IOTappStory2, config.IOTappStoryPHP2, FIRMWARE, true, true) == false) {
      DEBUG_PRINTLN(" Update not succesful");
#ifdef REMOTEDEBUGGING
      Debug.println(" Update not succesful");
#endif
      //     WiFi.begin("No", "connection");
      //     ESPrestart("No Connection");
    }
  }
}


//---------- CONFIGURATION PARAMETERS ----------

void writeConfig() {
  DEBUG_PRINTLN("------------------ Writing Config --------------------------------");
  if (WiFi.psk()!="") {
  
    WiFi.SSID().toCharArray(config.ssid,STRUCT_CHAR_ARRAY_SIZE);
    WiFi.psk().toCharArray(config.password,STRUCT_CHAR_ARRAY_SIZE);
    DEBUG_PRINT("Stored ");
    DEBUG_PRINT(config.ssid);
    DEBUG_PRINTLN(" ");
 //   DEBUG_PRINTLN(config.password);
  }
  EEPROM.begin(EEPROM_SIZE);
  config.magicBytes[0] = MAGICBYTES[0];
  config.magicBytes[1] = MAGICBYTES[1];
  config.magicBytes[2] = MAGICBYTES[2];

  for (unsigned int t = 0; t < sizeof(config); t++) EEPROM.write(t, *((char*)&config + t));
  EEPROM.end();
}


void readConfig() {
  DEBUG_PRINTLN("Reading Config");
  boolean ret = false;
  EEPROM.begin(EEPROM_SIZE);
  long magicBytesBegin = sizeof(config) - 4;

  if (EEPROM.read(magicBytesBegin) == MAGICBYTES[0] && EEPROM.read(magicBytesBegin + 1) == MAGICBYTES[1] && EEPROM.read(magicBytesBegin + 2) == MAGICBYTES[2]) {
    DEBUG_PRINTLN("Configuration found");
    for (unsigned int t = 0; t < sizeof(config); t++) *((char*)&config + t) = EEPROM.read(t);
    EEPROM.end();
    // Standard
    boardName = String(config.boardName);
    IOTappStory1 = String(config.IOTappStory1);
    IOTappStoryPHP1 = String(config.IOTappStoryPHP1);
    IOTappStory2 = String(config.IOTappStory2);
    IOTappStoryPHP2 = String(config.IOTappStoryPHP2);
    ret = true;

  } else {
    DEBUG_PRINTLN("Configurarion NOT FOUND!!!!");
    writeConfig();
  }
}


//---------- WIFIMANAGER COMMON FUNCTIONS
void initWiFiManager() {
  WiFi.printDiag(Serial); //Remove this line if you do not want to see WiFi password printed

  if (WiFi.SSID() == "") {
    DEBUG_PRINTLN("We haven't got any access point credentials, so get them now");
  } else {
    WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
    unsigned long startedAt = millis();
    DEBUG_PRINT("After waiting ");
    int connRes = WiFi.waitForConnectResult();
    float waited = (millis() - startedAt);
    DEBUG_PRINT(waited / 1000);
    DEBUG_PRINT(" secs in setup() connection result is ");
    DEBUG_PRINTLN(connRes);
  }

  if (WiFi.status() != WL_CONNECTED) {
    DEBUG_PRINTLN("Failed to connect");
  } else {
    DEBUG_PRINT("Local ip: ");
    DEBUG_PRINTLN(WiFi.localIP());
  }
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  writeConfig();
}


//---------- REMOTE DEBUG ----------

#ifdef REMOTEDEBUGGING
void remoteDebugSetup() {
  MDNS.addService("telnet", "tcp", 23);
  // Initialize the telnet server of RemoteDebug
  Debug.begin(config.boardName); // Initiaze the telnet server
  Debug.setResetCmdEnabled(true); // Enable the reset command
  // Debug.showProfiler(true); // To show profiler - time between messages of Debug
  // Good to "begin ...." and "end ...." messages
  // This sample (serial -> educattional use only, not need in production)

  // Debug.showTime(true); // To show time
}
#endif



