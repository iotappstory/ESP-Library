/* This sketch monitors a PIR sensor and transmitts an "ON" or "OFF" signal to a receiving device. Together with a
   Sonoff wireless switch, it can be used as a wireless motion detector. At startup, it connects to the IOTappstore to
   check for updates.

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

/*
   Setup till done: Blink
   ON: green LED completely on
   OFF: Green LED blinking with very short on-time
   Setup: very fast blinking green LED

*/


#define VERSION "V5.1"
#define SKETCH "SonoffSender "
#define FIRMWARE SKETCH VERSION

#define SERIALDEBUG       // Serial is used to present debugging messages 
#define REMOTEDEBUGGING       // telnet is used to present
#define BOOTSTATISTICS    // send bootstatistics to Sparkfun


#define LEDS_INVERSE   // LEDS on = GND


#include <credentials.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266HTTPClient.h>
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
#define LEDgreen 13
#define LEDred 12
#define PIRpin 14
#else
#define GPIO0 D3
#define LEDgreen D7
#define LEDred D6
#define PIRpin D5
#endif


//---------- CODE DEFINITIONS ----------
#define MAXDEVICES 5
#define STRUCT_CHAR_ARRAY_SIZE 50  // length of config variables
#define SERVICENAME "SONOFF"  // name of the MDNS service used in this group of ESPs
#define MAX_WIFI_RETRIES 50

#define RTCMEMBEGIN 68
#define MAGICBYTE 85



//-------- SERVICES --------------
WiFiClient client;
HTTPClient http;

Ticker blink;

// remoteDebug
#ifdef REMOTEDEBUGGING
RemoteDebug Debug;
#endif


//--------- ENUMS AND STRUCTURES  -------------------
enum statusDef {
  powerOff,
  powerOn,
  Renew
} loopStatus;

typedef struct {
  char ssid[STRUCT_CHAR_ARRAY_SIZE];
  char password[STRUCT_CHAR_ARRAY_SIZE];
  char boardName[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStore1[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStorePHP1[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStore2[STRUCT_CHAR_ARRAY_SIZE];
  char IOTappStorePHP2[STRUCT_CHAR_ARRAY_SIZE];
  char switchName1[STRUCT_CHAR_ARRAY_SIZE];
  char switchName2[STRUCT_CHAR_ARRAY_SIZE];
  // insert NEW CONSTANTS according boardname example HERE!
  char magicBytes[4];
} strConfig;

strConfig config = {
  mySSID,
  myPASSWORD,
  "SenderINIT",
  "192.168.0.200",
  "/iotappstore/iotappstorev20.php",
  "iotappstory.org",
  "ota/esp8266-v1.php",
  "",
  "",
  "CFG"  // Magic Bytes
};

typedef struct {
  byte markerFlag;
  int bootTimes;
} rtcMemDef __attribute__((aligned(4)));
rtcMemDef rtcMem;

//---------- VARIABLES ----------

String switchName1, switchName2, boardName, IOTappStore1, IOTappStorePHP1, IOTappStore2, IOTappStorePHP2; // add NEW CONSTANTS according boardname example
long onEntry;
IPAddress sonoffIP[10];
String deviceName[30];
char boardMode = 'N';  // Normal operation or Configuration mode?

unsigned long infoEntry = 0, dnsEntry = 0;

unsigned long eee;

volatile unsigned long buttonEntry, buttonTime;
volatile bool buttonChanged = false;
volatile int greenTimesOff = 0;
volatile int redTimesOff = 0;
volatile int greenTimes = 0;
volatile int redTimes = 0;

int discoverCount = 0;

bool PIRstatus;


//---------- FUNCTIONS ----------
void loopWiFiManager(void);
void discovermDNSServices(void);
bool switchAllSonoffs(bool);
bool switchSonoff(bool, String);
void readFullConfiguration(void);
bool readRTCmem(void);
void printRTCmem(void);
void showLoopState(statusDef);

//---------- OTHER .H FILES ----------
#include <ESP_Helpers.h>
#include "WiFiManager_Helpers.h"
#include <SparkfunReport.h>


//-------------------------- SETUP -----------------------------------------
void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 5; i++) DEBUG_PRINTLN("");
  DEBUG_PRINTLN("Start "FIRMWARE);
  REMOTEDEBUG_PRINTLN("Start "FIRMWARE);


  // ----------- PINS ----------------
  pinMode(GPIO0, INPUT_PULLUP);  // GPIO0 as input for Config mode selection
  pinMode(PIRpin, INPUT_PULLUP);
#ifdef LEDgreen
  pinMode(LEDgreen, OUTPUT);
  digitalWrite(LEDgreen, LEDOFF);
#endif
#ifdef LEDred
  pinMode(LEDred, OUTPUT);
  digitalWrite(LEDred, LEDOFF);
#endif
  blink.detach();

  // ------------- INTERRUPTS ----------------------------
  attachInterrupt(GPIO0, ISRbuttonStateChanged, CHANGE);


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


  DEBUG_PRINTLN("------------- Normal Mode -------------------");
  REMOTEDEBUG_PRINTLN("------------- Normal Mode -------------------");

  // --------- START WIFI --------------------------
  readFullConfiguration();
  WiFi.mode(WIFI_STA);
  WiFi.begin();

  if (!isNetworkConnected()) {
    DEBUG_PRINTLN("NoConn");
    if ( WiFi.psk() == "") {
      DEBUG_PRINTLN("Try to connect with saved PW");
      WiFi.begin(config.ssid, config.password);  // if password forgotten by firmwware try again with stored PW
      if (!isNetworkConnected()) ESP.restart();  // still no success
    } else {
      DEBUG_PRINTLN("No valid PW");
      ESP.restart();  // still no success
    }
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

#ifdef BOOTSTATISTICS
    sendSparkfun();   // send boot statistics to sparkfun
#endif



    // ----------- SPECIFIC SETUP CODE ----------------------------

    // Register host name in WiFi and mDNS
    String hostNameWifi = boardName;   // boardName is device name
    hostNameWifi.concat(".local");
    WiFi.hostname(hostNameWifi);
    if (MDNS.begin(config.boardName)) {
      DEBUG_PRINT("* MDNS responder started. http://");
      DEBUG_PRINTLN(hostNameWifi);
    } else espRestart('C', "No Credentials");

  }  // End WiFi necessary
  switchAllSonoffs(OFF);
  LEDswitch(None);
  loopStatus = powerOff;

  DEBUG_PRINTLN("setup done");
}


//--------------- LOOP ----------------------------------
void loop() {

  //-------- Standard Block ---------------
  if (buttonChanged && buttonTime > 4000) espRestart('C', "Going into Configuration Mode");  // long button press > 4sec
  if (buttonChanged && buttonTime > 500 && buttonTime < 4000) iotAppstory(); // long button press > 1sec
  buttonChanged = false;
#ifdef REMOTEDEBUGGING
  Debug.handle();
#endif
  yield();
  //-------- End Standard Block ---------------


  // ------- Debug Message --------
#ifdef REMOTEDEBUGGING
  if ((millis() - infoEntry) > 2000) {
    if (Debug.ative(Debug.INFO)) {
      Debug.printf("Firmware: %s", FIRMWARE);
      if (PIRstatus) Debug.print(" PIRstatus: ON ");
      else Debug.print(" PIRstatus: OFF ");
      int hi = abs((millis() - onEntry) / 1000);
      Debug.printf(" Delay: %d", hi);
      Debug.println(" sec:");
      Debug.print("Heap ");
      Debug.println(ESP.getFreeHeap());
    }
    infoEntry = millis();
  }
#endif
  // -------------------------------

  if (MDNS.hostname(0) == "" || (millis() - dnsEntry > 60000)) { // every minute or if no device detected
    if (MDNS.hostname(0) == "") {  // if no device detected
      discovermDNSServices();
      for (int i = 0; i < 5; i++) DEBUG_PRINTLN(sonoffIP[i]);
      discoverCount++;
      dnsEntry = millis();
    } else  discoverCount = 0;
  }
  if (discoverCount > 10) {
    LEDswitch(RedFastBlink);
    delay(2000);
    ESP.restart();    // restart if no services available
  }

  PIRstatus = digitalRead(PIRpin);

  switch (loopStatus) {
    case powerOn:        // Lamp is on
      LEDswitch(Green);

      // exit criteria
      if (PIRstatus == OFF) {
        loopStatus = powerOff;
        LEDswitch(None);
        showLoopState(loopStatus);

      }
      else if (abs(millis() - onEntry) > 10000) {  // every 10 sec
        loopStatus = Renew;
        showLoopState(loopStatus);
      }
      break;

    case Renew:       // Lamp is on, send additional message
      switchAllSonoffs(ON);
      onEntry = millis();

      // exit criteria
      loopStatus = powerOn;
      showLoopState(loopStatus);
      break;

    case powerOff:       // Lamp is off
      // exit criteria
      if (PIRstatus == ON) {
        switchAllSonoffs(ON);
        loopStatus = powerOn;
        showLoopState(loopStatus);
        onEntry = millis();
      }
      break;

    default:
      break;
  }
}
// ------------------------- END LOOP ----------------------------------


void showLoopState(statusDef loopStatus) {
  DEBUG_PRINT(millis() / 1000);
  REMOTEDEBUG_PRINT(millis() / 1000);
  DEBUG_PRINT(" Discovercount: ");
  DEBUG_PRINT(discoverCount);
  switch (loopStatus) {
    case powerOff:
      DEBUG_PRINT(" Status: ");
      DEBUG_PRINTLN("OFF");
      REMOTEDEBUG_PRINT(" Status: ");
      REMOTEDEBUG_PRINTLN("OFF");
      break;

    case powerOn:
      DEBUG_PRINT(" Status: ");
      DEBUG_PRINTLN("ON");
      REMOTEDEBUG_PRINT(" Status: ");
      REMOTEDEBUG_PRINTLN("ON");
      break;
    case Renew:
      DEBUG_PRINT(" Status: ");
      DEBUG_PRINTLN("Renew");
      REMOTEDEBUG_PRINT(" Status: ");
      REMOTEDEBUG_PRINTLN("Renew");
      break;

    default:
      break;
  }
}


bool switchAllSonoffs(bool command) {
  bool ret1, ret2;

  if (switchName1 != "") ret1 = switchSonoff(command, switchName1);
  if (!ret1) {
    DEBUG_PRINT(switchName1);
    DEBUG_PRINTLN(" not present");
    REMOTEDEBUG_PRINT(switchName1);
    REMOTEDEBUG_PRINTLN(" not present");
  }
  if (switchName2 != "") ret2 = switchSonoff(command, switchName2);
  if (!ret2) {
    DEBUG_PRINT(switchName2);
    DEBUG_PRINTLN(" not present");
    REMOTEDEBUG_PRINT(switchName2);
    REMOTEDEBUG_PRINTLN(" not present");
  }
  return ret1 & ret2;
}

bool switchSonoff(bool command, String device) {
  int i = 0;
  bool found = false;
  String payload;
  String switchString;

  if (WiFi.status() != WL_CONNECTED) espRestart('N', "Not connected");

  while ( deviceName[i].length() > 0 && i <= MAXDEVICES) {
    yield();
    payload = "";
    switchString = "";

    if (deviceName[i] == device) {
      switchString = "http://" + sonoffIP[i].toString();

      if (command == ON) switchString = switchString + "/SWITCH=ON";
      else switchString = switchString + "/SWITCH=OFF";
      DEBUG_PRINTLN(switchString);
      REMOTEDEBUG_PRINTLN(switchString);
      yield();
      http.begin(switchString);

      //  DEBUG_PRINT("[HTTP] GET...\n");
      // start connection and send HTTP header

      int httpCode = http.GET();
      Serial.printf("[HTTP] GET... code: %d\r\n", httpCode);
#ifdef REMOTEDEBUGGING
      Debug.printf("[HTTP] GET... code: %d\r\n", httpCode);
#endif
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        if (httpCode == HTTP_CODE_OK) {
          payload = http.getString();
          found = true;
        } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          LEDswitch(RedBlink);
        }
      } else {
        DEBUG_PRINTLN("HTTP code not ok");
        LEDswitch(RedBlink);
      }
      http.end();
    }
    i++;
  }
  return found;
}

void discovermDNSServices() {
  int j;
  for (j = 0; j < 5; j++) sonoffIP[j] = (0, 0, 0, 0);
  j = 0;
  DEBUG_PRINTLN("Sending mDNS query");
  yield();
  int n = MDNS.queryService("SERVICENAME", "tcp"); // Send out query for esp tcp services
  yield();
  DEBUG_PRINTLN("mDNS query done");
  if (n == 0) {
    espRestart('N', "No services found");
  }
  else {
    DEBUG_PRINT(n);
    DEBUG_PRINTLN(" service(s) found");
    for (int i = 0; i < n; ++i) {
      yield();
      // Print details for each service found
      DEBUG_PRINT(i + 1);
      DEBUG_PRINT(": ");
      DEBUG_PRINT(MDNS.hostname(i));
      DEBUG_PRINT(" (");
      DEBUG_PRINT(MDNS.IP(i));
      deviceName[j] = MDNS.hostname(i);
      sonoffIP[j++] = MDNS.IP(i);
      DEBUG_PRINT(":");
      DEBUG_PRINT(MDNS.port(i));
      DEBUG_PRINTLN(")");
      yield();
    }
  }
  DEBUG_PRINTLN();
}


void readFullConfiguration() {
  readConfig();  // configuration in EEPROM
  // insert NEW CONSTANTS according switchName1 example
  switchName1 = String(config.switchName1);
  switchName2 = String(config.switchName2);
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

