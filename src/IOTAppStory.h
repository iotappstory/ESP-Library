#ifndef IOTAppStory_h
#define IOTAppStory_h

#include <Arduino.h>
#include "WiFiManagerMod.h"

/* ------ ------ ------ DEFINES for library ------ ------ ------ */
#define MAGICBYTES "CFG"
#define EEPROM_SIZE 1024
#define MAXNUMEXTRAFIELDS 12
#define MAGICEEP "%"
#define UDP_PORT 514
#define MAX_WIFI_RETRIES 15
#define RTCMEMBEGIN 68
#define MAGICBYTE 85
#define STRUCT_CHAR_ARRAY_SIZE 50  // length of config variables
#define STRUCT_COMPVERSION_SIZE 20
#define STRUCT_HOST_SIZE 24
#define STRUCT_FILE_SIZE 32

// macros for debugging
#ifdef DEBUG_PORT
    #define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
#else
    #define DEBUG_MSG(...)
#endif

#define DEBUG_PRINT(x)    { if(_serialDebug) Serial.print(x);   }
#define DEBUG_PRINTLN(x)  { if(_serialDebug) Serial.println(x); }

#ifdef LEDS_INVERSE
    #define LEDON  0
    #define LEDOFF 1
#else
    #define LEDON  1
    #define LEDOFF 0
#endif

#define MODE_BUTTON_NO_PRESS     0
#define MODE_BUTTON_SHORT_PRESS  1
#define MODE_BUTTON_LONG_PRESS   2

class IOTAppStory {
public:
    /* ------ ------ ------ VARIABLES & STRUCTURES ------ ------ ------ */
    typedef struct {
        byte markerFlag;
        int bootTimes;
        char boardMode = 'N';      // Normal operation or Configuration mode?
    } rtcMemDef __attribute__((aligned(4)));
    rtcMemDef rtcMem;

    typedef struct {
        const char *fieldIdName;
        const char *fieldLabel;
        char* (*varPointer);
        int length;
    } eFields;

    typedef struct {
        char ssid[STRUCT_CHAR_ARRAY_SIZE];
        char password[STRUCT_CHAR_ARRAY_SIZE];
        char boardName[STRUCT_CHAR_ARRAY_SIZE];
        char IOTappStory1[STRUCT_HOST_SIZE];
        char IOTappStoryPHP1[STRUCT_FILE_SIZE];
        char IOTappStory2[STRUCT_HOST_SIZE];
        char IOTappStoryPHP2[STRUCT_FILE_SIZE];

        bool automaticUpdate;    // right after boot
        char compVersion[STRUCT_COMPVERSION_SIZE];
        char devPass[7];

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
        false,
        "",
        "CFG"  // Magic Bytes
    };

    eFields fieldStruct[MAXNUMEXTRAFIELDS];
    WiFiManagerParameter parArray[MAXNUMEXTRAFIELDS];
    unsigned long buttonEntry, debugEntry;
    int buttonStateOld;
    long counter = 0;

    /* ------ ------ ------ FUNCTION DEFINITIONS ------ ------ ------ */
    IOTAppStory(const char *appName, const char *appVersion, const char *compVersion, const int modeButton);

    void serialdebug(bool onoff,int speed=115200);

    // function for pre setting config parameters ssid & password, boardname, automatic update, IOTappStory1 and IOTappStoryPHP1
    void preSetConfig(String boardName, bool automaticUpdate = false);
    void preSetConfig(String ssid, String password, bool automaticUpdate = false);
    void preSetConfig(String ssid, String password, String boardName, bool automaticUpdate = false);
    void preSetConfig(String ssid, String password, String boardName, String IOTappStory1, String IOTappStoryPHP1, bool automaticUpdate = false);

    void begin(bool bootstats=true, bool ea = false); // ea = erase all eeprom
    void firstBoot(bool ea = false);

    bool readRTCmem();
    void writeRTCmem();
    void printRTCmem();

    void configESP();
    void readFullConfiguration();

    void connectNetwork();
    bool isNetworkConnected();
    String getMACaddress();
    void printMacAddress();

    bool callHome(bool spiffs = true);
    byte iotUpdaterSketch(String server, String url, String firmware, bool immediately);
    byte iotUpdaterSPIFFS(String server, String url, String firmware, bool immediately);

    void addField(char* &defaultVal,const char *fieldIdName,const char *fieldLabel, int length);
    void processField();
    int dPinConv(String orgVal);

    void initWiFiManager();
    void loopWiFiManager();

    void espRestart(char mmode, char* message);
    void eraseFlash(unsigned int eepFrom = 0, unsigned int eepTo = EEPROM_SIZE);

    void writeConfig(bool wifiSave=false);
    bool readConfig();
    void routine();
    void JSONerror(String err);
    void saveConfigCallback();
    void sendDebugMessage();
    bool isModeButtonPressed();
    int getModeButtonStatus(bool checkCurrentButtonState = true);

private:
    String  _appName;
    String  _appVersion;
    String  _firmware;
    String  _compVersion;
    int     _modeButton;
    int     _nrXF = 0;          // nr of extra fields required in the config manager
    bool    _serialDebug;
    bool    _setPreSet = false;

private: // methods

    bool isFirstBoot();
};

#endif
