#ifndef IOTAppStory_h
    #define IOTAppStory_h


    /* ------ ------ ------ DEFINES for librairy ------ ------ ------ */
    #define MAGICBYTES "CFG"
    #define EEPROM_SIZE 1024
    #define MAXNUMEXTRAFIELDS 12
    #define MAGICEEP "%"
    #define UDP_PORT 514
    #define MAX_WIFI_RETRIES 50
    #define RTCMEMBEGIN 68
    #define MAGICBYTE 85
    #define STRUCT_CHAR_ARRAY_SIZE 50  // length of config variables
    #define STRUCT_COMPDATE_SIZE 20
    #define STRUCT_HOST_SIZE 24
    #define STRUCT_FILE_SIZE 32
	
	
	

    // macros for debugging
    #ifdef DEBUG_PORT
        #define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
    #else
        #define DEBUG_MSG(...)
    #endif

    //#ifdef SERIALDEBUG
    #define         DEBUG_PRINT(x)    Serial.print(x)
    #define         DEBUG_PRINTLN(x)  Serial.println(x)
    //#else
    //#define         DEBUG_PRINT(x)
    //#define         DEBUG_PRINTLN(x)
    //#endif

    #ifdef LEDS_INVERSE
        #define LEDON  0
        #define LEDOFF 1
    #else
        #define LEDON  1
        #define LEDOFF 0
    #endif



    /* ------ ------ ------ Arduino library ------ ------ ------ */
    #include <Arduino.h>


    /* ------ ------ ------ begin necessary library's for using IOTAppStory ------ ------ ------ */
    #ifndef WiFiManager_h
        #include "WiFiManagerMod.h"
    #endif
    #ifndef ESP8266WiFi_h
        #include <ESP8266WiFi.h>
    #endif
        #ifndef ESP8266httpUpdate_h
    #include <ESP8266httpUpdate.h>
    #endif
        #ifndef DNSServer_h
    #include <DNSServer.h>
    #endif
    #ifndef ESP8266mDNS_h
        #include <ESP8266mDNS.h>
    #endif
    //#ifndef Ticker_h
        //#include <Ticker.h>
    //#endif
    #ifndef EEPROM_h
        #include <EEPROM.h>
    #endif
    #ifndef __PGMSPACE_H_
        #include <pgmspace.h>
    #endif
    #ifndef ArduinoJson_h
        #include <ArduinoJson.h>
    #endif
    #ifndef FS_h
        #include <FS.h>
    #endif
    /* ------ ------ ------ end necessary library's for using IOTAppStory ------ ------ ------ */


    /* ------ ------ ------ if REMOTEDEBUGGING is defined include the WiFiUDP library ------ ------ ------ */
    #ifdef REMOTEDEBUGGING
        #ifndef WiFiUDP_h
            #include <WiFiUDP.h>
        #endif
    #endif


    /* ------ ------ ------ this is for the RTC memory read/write functions ------ ------ ------ */
    extern "C" {
        #ifndef user_interface_h
            #include "user_interface.h"
        #endif
    }


    class IOTAppStory {
        public:
            /* ------ ------ ------ VARIABLES & STRUCTURES ------ ------ ------ */
            typedef struct {
                byte markerFlag;
                int bootTimes;
				char boardMode = 'N';  	// Normal operation or Configuration mode?
            } rtcMemDef __attribute__((aligned(4)));
            rtcMemDef rtcMem;

            typedef struct  {
                const char *fieldIdName;
                const char *fieldLabel;
                char* (*varPointer);
                int length;

                //char *custom;
                //int labelPlacement;
            } eFields;

            typedef struct  {
                char ssid[STRUCT_CHAR_ARRAY_SIZE];
                char password[STRUCT_CHAR_ARRAY_SIZE];
                char boardName[STRUCT_CHAR_ARRAY_SIZE];
                char IOTappStory1[STRUCT_HOST_SIZE];
                char IOTappStoryPHP1[STRUCT_FILE_SIZE];
                char IOTappStory2[STRUCT_HOST_SIZE];
                char IOTappStoryPHP2[STRUCT_FILE_SIZE];

                bool automaticUpdate;	// right after boot
                char compDate[STRUCT_COMPDATE_SIZE];
				
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
            //volatile unsigned long (*buttonEntry);
            //unsigned long (*buttonTime);
            //volatile bool (*buttonChanged);
            //unsigned long debugEntry;
            //String sysMessage; 			<<-- is this still needed?
            long counter = 0;


            /* ------ ------ ------ FUCNTION DEFINITIONS ------ ------ ------ */
            IOTAppStory(const char *appName, const char *appVersion, const char *compDate, const int modeButton);

            void serialdebug(bool onoff,int speed=115200);

            // function for pre setting config parameters ssid & password, boardname, automatic update, IOTappStory1 and IOTappStoryPHP1
            void preSetConfig(String boardName);
            void preSetConfig(String boardName, bool automaticUpdate);
            void preSetConfig(String ssid, String password);
            void preSetConfig(String ssid, String password, bool automaticUpdate);
            void preSetConfig(String ssid, String password, String boardName);
            void preSetConfig(String ssid, String password, String boardName, bool automaticUpdate);
            void preSetConfig(String ssid, String password, String boardName, String IOTappStory1, String IOTappStoryPHP1, bool automaticUpdate);

            void begin(int feedBackLed=-1, bool bootstats=true);
            void firstBoot();

            bool readRTCmem();
            void writeRTCmem();
            void printRTCmem();

            void configESP();
            void readFullConfiguration();

            void connectNetwork();
            bool isNetworkConnected();
            String getMACaddress();
            void printMacAddress();

            bool callHome(bool spiffs);
            bool callHome();
            //void initialize();
            byte iotUpdaterSketch(String server, String url, String firmware, bool immediately);
            byte iotUpdaterSPIFFS(String server, String url, String firmware, bool immediately);

            //void addField(int &defaultVal,const char *fieldIdName,const char *fieldLabel, int length);
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
//	    void routine(volatile unsigned long (*org_buttonEntry), unsigned long (*org_buttonTime), volatile bool (*org_buttonChanged));
            void JSONerror(String err);
            void saveConfigCallback();
            void sendDebugMessage();

        private:
            String  _appName;
            String  _appVersion;
            String  _firmware;
            String  _compDate;
            int     _modeButton;
            int     _nrXF = 0;			// nr of extra fields required in the config manager
            bool    _serialDebug;
            bool    _setPreSet = false;		// ;)
    };

#endif
