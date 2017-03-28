

#define MAGICBYTES "CFG"
#define EEPROM_SIZE 1024

#define UDP_PORT 514


// macros for debugging

#ifdef DEBUG_PORT
#define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

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

#define         UDPDEBUG_PRINT(x,y)    debugPrint(x,y)
#define         UDPDEBUG_PRINTTXT(x)   debugPrintTxt(x)
#define         UDPDEBUG_START()       debugStart()
#define         UDPDEBUG_SEND()        debugSend()


#else

#define         REMOTEDEBUG_PRINT(x)
#define         REMOTEDEBUG_PRINTLN(x)

#define         UDPDEBUG_PRINT(x,y)
#define         UDPDEBUG_PRINTTXT(x)
#define         UDPDEBUG_START()
#define         UDPDEBUG_SEND()
#endif



#ifdef LEDS_INVERSE
#define LEDON  0
#define LEDOFF 1
#else
#define LEDON  1
#define LEDOFF 0
#endif


#define MAX_WIFI_RETRIES 50
#define RTCMEMBEGIN 68
#define MAGICBYTE 85

typedef struct {
  byte markerFlag;
  int bootTimes;
} rtcMemDef __attribute__((aligned(4)));
rtcMemDef rtcMem;

char boardMode = 'N';  // Normal operation or Configuration mode?
long counter = 0;

String boardName, IOTappStory1, IOTappStoryPHP1, IOTappStory2, IOTappStoryPHP2;
volatile unsigned long buttonEntry;
unsigned long buttonTime;
volatile bool buttonChanged = false;
volatile int greenTimesOff = 0;
volatile int redTimesOff = 0;
volatile int greenTimes = 0;
volatile int redTimes = 0;

int tim=0;

#ifdef REMOTEDEBUGGING
WiFiUDP UDP;
#endif

Ticker blink;

bool connectUDP() {
#ifdef REMOTEDEBUGGING
  //  DEBUG_PRINTLN("");
  //  DEBUG_PRINTLN("Connecting to UDP");
  if (UDP.begin(UDP_PORT) == 1)
  {
    //    DEBUG_PRINTLN("UDP Connect successful");
    return true;
  }
  else {
    //   DEBUG_PRINTLN("UDP Connect failed!");
    return false;
  }
#endif
}

#ifdef REMOTEDEBUGGING

char debugBuffer[255];
IPAddress broadcastIp(255, 255, 255, 255);

void debugStart() {
  debugBuffer[0] = '\0';
}

void debugSend() {
  bool udpConnected = connectUDP();
  // if (udpConnected) Serial.println("UPD Connected");
  //   else Serial.println("UPD FAILED!");
  UDP.beginPacket(broadcastIp, UDP_PORT);
  UDP.write(debugBuffer);
  UDP.endPacket();
  DEBUG_PRINTLN(debugBuffer);
  UDP.stop();
}

void debugPrint(char*txt, int nbr) {
  char buf[100];
  sprintf(buf, "%s%i", txt, nbr);
  strcat(debugBuffer, buf);
}

void debugPrint(char*txt, long nbr) {
  char buf[100];
  sprintf(buf, "%s%i", txt, nbr);
  strcat(debugBuffer, buf);
}


void debugPrint(char*txt, float nbr) {
  char buf[100];
  sprintf(buf, "%s%i", txt, nbr);
  strcat(debugBuffer, buf);
}

void debugPrintTxt(char* buf) {
  strcat(debugBuffer, buf);
}

void debugPrintTxt(String hh) {
  char buf[100];
  hh.toCharArray(buf, 100);
  strcat(debugBuffer, buf);
}

void sendSysLogMessageReal(int severity, int facility, String hostName, String app, int procID, int msgID, String message) {
  int priVal = (8 * facility) + severity;
  app.replace(" ", "_");
  debugStart();
  debugPrint("<", priVal); //PRIVAL
  debugPrintTxt(">");
  debugPrintTxt("1");   //  ??
  debugPrintTxt(" - ");  // Timestamp
  debugPrintTxt(hostName);
  debugPrintTxt(" ");
  debugPrintTxt(app);
  debugPrint(" ", procID); //PRIVAL
  debugPrint(" ", msgID); //PRIVAL
  debugPrintTxt(" ");
  debugPrintTxt(message);
  debugSend();
}
#endif

void sendSysLogMessage(int severity, int facility, String hostName, String app, int procID, int msgID, String message) {
#ifdef REMOTEDEBUGGING
  sendSysLogMessageReal(severity, facility, hostName, app, procID, msgID, message);
#endif
}

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
  rtcMemDef rtcMem;
*/



//---------- LED FUNCTIONS ----------



void tickGreen() {
#ifdef LEDgreen
  if (greenTimes % greenTimesOff == 0) digitalWrite(LEDgreen, LEDON);
  else digitalWrite(LEDgreen, LEDOFF);
  greenTimes++;
#endif
}

void tickRed() {
#ifdef LEDred
  if (redTimes % redTimesOff == 0) digitalWrite(LEDred, LEDON);
  else digitalWrite(LEDred, LEDOFF);
  redTimes++;
#endif
}

void greenFlash(float takt, int timesOff) {
  greenTimesOff = timesOff;
  blink.attach(takt, tickGreen);
}

void redFlash(float takt, int timesOff) {
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

void writeRTCmem() {
  rtcMem.markerFlag = MAGICBYTE;
  system_rtc_mem_write(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
}


//---------- MISC FUNCTIONS ----------

void espRestart(char mmode, char* message) {
  LEDswitch(GreenFastBlink);
  DEBUG_PRINTLN(message);
  while (digitalRead(MODEBUTTON) == LOW) yield();    // wait till GPIOo released
  delay(500);
  system_rtc_mem_write(RTCMEMBEGIN + 100, &mmode, 1);
  system_rtc_mem_read(RTCMEMBEGIN + 100, &boardMode, 1);
  ESP.restart();
}

void eraseFlash() {
  Serial.println("Erasing Flash...");
  EEPROM.begin(EEPROM_SIZE);
  for (unsigned int t = 0; t < EEPROM_SIZE; t++) EEPROM.write(t, 0);
  EEPROM.end();
}

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

void connectNetwork() {
  WiFi.mode(WIFI_STA);
  if (!isNetworkConnected()) {
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("No Connection. Try to connect with saved PW");
    WiFi.begin(config.ssid, config.password);  // if password forgotten by firmwware try again with stored PW
    if (!isNetworkConnected()) espRestart('C', "Going into Configuration Mode"); // still no success
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("WiFi connected");
  getMACaddress();
  printMacAddress();
  DEBUG_PRINT("IP Address: ");
  DEBUG_PRINTLN(WiFi.localIP());

  // Register host name in WiFi and mDNS
  String hostNameWifi = config.boardName;   // boardName is device name
  hostNameWifi.concat(".local");
  wifi_station_set_hostname(config.boardName);
  //   WiFi.hostname(hostNameWifi);
  if (MDNS.begin(config.boardName)) {
    DEBUG_PRINT("* MDNS responder started. http://");
    DEBUG_PRINTLN(hostNameWifi);
  } else espRestart('N', "MDNS not started");
}


void ISRbuttonStateChanged() {
  if (digitalRead(MODEBUTTON) == 0) buttonEntry = millis();
  else {
    buttonTime = millis() - buttonEntry;
    buttonChanged = true;
  }
}

// prints important parametes

void welcome() {

  delay(2000);
  DEBUG_MSG("%s %s\n", (char *) SKETCH, (char *) VERSION);
  //DEBUG_MSG("Device: %s\n", (char *) getIdentifier().c_str());
  DEBUG_MSG("ChipID: %06X\n", ESP.getChipId());
  DEBUG_MSG("CPU frequency: %d MHz\n", ESP.getCpuFreqMHz());
  DEBUG_MSG("Last reset reason: %s\n", (char *) ESP.getResetReason().c_str());
  DEBUG_MSG("Memory size: %d bytes\n", ESP.getFlashChipSize());
  DEBUG_MSG("Free heap: %d bytes\n", ESP.getFreeHeap());
  DEBUG_MSG("Firmware size: %d bytes\n", ESP.getSketchSize());
  DEBUG_MSG("Free firmware space: %d bytes\n", ESP.getFreeSketchSpace());
#ifdef SPIFFS
  FSInfo fs_info;
  if (SPIFFS.info(fs_info)) {
    DEBUG_MSG("File system total size: %d bytes\n", fs_info.totalBytes);
    DEBUG_MSG("            used size : %d bytes\n", fs_info.usedBytes);
    DEBUG_MSG("            block size: %d bytes\n", fs_info.blockSize);
    DEBUG_MSG("            page size : %d bytes\n", fs_info.pageSize);
    DEBUG_MSG("            max files : %d\n", fs_info.maxOpenFiles);
    DEBUG_MSG("            max length: %d\n", fs_info.maxPathLength);
  }
#endif
  DEBUG_MSG("\n\n");

}


//---------- IOTappStory FUNCTIONS ----------
byte iotUpdaterSketch(String server, String url, String firmware, bool immediately) {
  byte retValue;
  DEBUG_PRINTLN("Updating Sketch from...");
  DEBUG_PRINT("Update_server ");
  DEBUG_PRINTLN(server);
  DEBUG_PRINT("UPDATE_URL ");
  DEBUG_PRINTLN(url);
  DEBUG_PRINT("FIRMWARE_VERSION ");
  DEBUG_PRINTLN(firmware);
  t_httpUpdate_return ret = ESPhttpUpdate.update(server, 80, url, firmware);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      #ifdef SERIALDEBUG
        Serial.printf("SKETCH_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      #endif
      DEBUG_PRINTLN();
      retValue = 'F';
    break;

    case HTTP_UPDATE_NO_UPDATES:
      DEBUG_PRINTLN("---------- SKETCH_UPDATE_NO_UPDATES ------------------");
      retValue = 'A';
    break;

    case HTTP_UPDATE_OK:
      DEBUG_PRINTLN("SKETCH_UPDATE_OK");
      retValue = 'U';
    break;
  }
  return retValue;
}

byte iotUpdaterSPIFFS(String server, String url, String firmware, bool immediately) {
  byte retValue;
  DEBUG_PRINTLN("Updating SPIFFS from...");
  DEBUG_PRINT("Update_server ");
  DEBUG_PRINTLN(server);
  DEBUG_PRINT("UPDATE_URL ");
  DEBUG_PRINTLN(url);
  DEBUG_PRINT("FIRMWARE_VERSION ");
  DEBUG_PRINTLN(firmware);

  t_httpUpdate_return retspiffs = ESPhttpUpdate.updateSpiffs("http://" + String(server + url), firmware);
  switch (retspiffs) {
    case HTTP_UPDATE_FAILED:
      #ifdef SERIALDEBUG
        Serial.printf("SPIFFS_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      #endif
      DEBUG_PRINTLN();
      retValue = 'F';
    break;

    case HTTP_UPDATE_NO_UPDATES:
      DEBUG_PRINTLN("---------- SPIFFS_UPDATE_NO_UPDATES ------------------");
      retValue = 'A';
    break;

    case HTTP_UPDATE_OK:
      DEBUG_PRINTLN("SPIFFS_UPDATE_OK");
      retValue = 'U';
    break;
  }
  return retValue;
}



void IOTappStory(bool spiffs) {
  // update from IOTappStory.com
  bool updateHappened=false;
  byte res1, res2;

  sendSysLogMessage(7, 1, config.boardName, FIRMWARE, 10, counter++, "------------- IOTappStory -------------------");
  LEDswitch(GreenSlowBlink);

  getMACaddress();
  printMacAddress();
  DEBUG_PRINT("IP = ");
  DEBUG_PRINTLN(WiFi.localIP());
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("");

  ESPhttpUpdate.rebootOnUpdate(false);
  
  res1 = iotUpdaterSketch(config.IOTappStory1, config.IOTappStoryPHP1, FIRMWARE, true);
  if (res1 == 'F') {
    String message = IOTappStory1 + ": Update not succesful";
    sendSysLogMessage(2, 1, config.boardName, FIRMWARE, 10, counter++, message);
    res2 = iotUpdaterSketch(config.IOTappStory2, config.IOTappStoryPHP2, FIRMWARE, true) ;
    if (res2 == 'F') {
      message = IOTappStory2 + ": Update not succesful";
      sendSysLogMessage(2, 1, config.boardName, FIRMWARE, 10, counter++, message);
    } 
  }
  if (res1 == 'U' || res2 == 'U')  updateHappened = true;
  
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("");

  if (spiffs) {
  res1 = iotUpdaterSPIFFS(config.IOTappStory1, config.IOTappStoryPHP1, FIRMWARE, true);
    if (res1 == 'F') {
      String message = IOTappStory1 + ": Update not succesful";
      sendSysLogMessage(2, 1, config.boardName, FIRMWARE, 10, counter++, message);
      res2 = iotUpdaterSPIFFS(config.IOTappStory2, config.IOTappStoryPHP2, FIRMWARE, true);
      if (res2 == 'F') {
        message = IOTappStory2 + ": Update not succesful";
        sendSysLogMessage(2, 1, config.boardName, FIRMWARE, 10, counter++, message);
      }
    }
  } 
  if (res1 == 'U' || res2 == 'U')  updateHappened = true;
  
  DEBUG_PRINTLN("Returning from IOTAppstory");
  DEBUG_PRINTLN("");
  
  if (updateHappened) {
  	initialize();
  	boardMode = 'N';
  	ESP.restart();
  }
}


void IOTappStory() {
  IOTappStory(true);
}


int handleModeButton() {
  if (buttonChanged && buttonTime > 3000) espRestart('C', "Going into Configuration Mode");  // long button press > 4sec
  if (buttonChanged && buttonTime > 500 && buttonTime < 4000) IOTappStory(); // long button press > 1sec
  if (buttonChanged) tim=buttonTime;
  else tim=0;
  buttonChanged = false;
  return tim;
}


//---------- CONFIGURATION PARAMETERS ----------


void writeConfig() {
  DEBUG_PRINTLN("------------------ Writing Config --------------------------------");
  if (WiFi.psk() != "") {
    WiFi.SSID().toCharArray(config.ssid, STRUCT_CHAR_ARRAY_SIZE);
    WiFi.psk().toCharArray(config.password, STRUCT_CHAR_ARRAY_SIZE);
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


bool readConfig() {
  DEBUG_PRINTLN("Reading Config");
  boolean ret = false;
  EEPROM.begin(EEPROM_SIZE);
  long magicBytesBegin = sizeof(config) - 4; // Magic bytes at the end of the structure

  if (EEPROM.read(magicBytesBegin) == MAGICBYTES[0] && EEPROM.read(magicBytesBegin + 1) == MAGICBYTES[1] && EEPROM.read(magicBytesBegin + 2) == MAGICBYTES[2]) {
    DEBUG_PRINTLN("EEPROM Configuration found");
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
    DEBUG_PRINTLN("EEPROM Configurarion NOT FOUND!!!!");
    writeConfig();
    LEDswitch(RedFastBlink);
    ret = false;
  }
  return ret;
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
