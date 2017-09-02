#ifndef IOTAppStory_h
    #define IOTAppStory_h

    #include <Arduino.h>

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
    #define STRUCT_COMPDATE_SIZE 20
    #define STRUCT_HOST_SIZE 24
    #define STRUCT_FILE_SIZE 32

    // constants used to define the status of the mode button based on the time it was pressed. (miliseconds)
    #define ENTER_CHECK_FIRMWARE_TIME_MIN 500
    #define ENTER_CHECK_FIRMWARE_TIME_MAX 4000
    #define ENTER_CONFIG_MODE_TIME_MIN    ENTER_CHECK_FIRMWARE_TIME_MAX
    #define ENTER_CONFIG_MODE_TIME_MAX    10000


    // macros for debugging
    #ifdef DEBUG_PORT
        #define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
    #else
        #define DEBUG_MSG(...)
    #endif

    //#ifdef SERIALDEBUG
    #define         DEBUG_PRINT(x)    { if(_serialDebug) Serial.print(x);   }
    #define DEBUG_PRINTF(...) { if(_serialDebug) Serial.printf(__VA_ARGS__);  }
    #define         DEBUG_PRINTLN(x)  { if(_serialDebug) Serial.println(x); }
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
            unsigned long buttonEntry, debugEntry;
            int buttonStateOld;
            //String sysMessage; 			<<-- is this still needed?
            long counter = 0;


            /* ------ ------ ------ FUCNTION DEFINITIONS ------ ------ ------ */
            IOTAppStory(const char *appName, const char *appVersion, const char *compDate, const int modeButton);

            void serialdebug(bool onoff,int speed=115200);

            // function for pre setting config parameters ssid & password, boardname, automatic update, IOTappStory1 and IOTappStoryPHP1
            void preSetConfig(String boardName, bool automaticUpdate = false);
            void preSetConfig(bool automaticUpdate = false);
            void preSetConfig(String ssid, String password, bool automaticUpdate = false);
            void preSetConfig(String ssid, String password, String boardName, bool automaticUpdate = false);
            void preSetConfig(String ssid, String password, String boardName, String IOTappStory1, String IOTappStoryPHP1, bool automaticUpdate = false);

            void begin(bool bootstats=true, bool ea=false); 			// ea = erase all eeprom
            void firstBoot(bool ea=false);

            bool readRTCmem();
            void writeRTCmem();
            void printRTCmem();

            void configESP();

            void connectNetwork();
            bool isNetworkConnected();

            bool callHome(bool spiffs = true);
            byte iotUpdater(bool type, String server, String url);

            void addField(char* &defaultVal,const char *fieldIdName,const char *fieldLabel, int length);
            void processField();
            int dPinConv(String orgVal);

            void initWiFiManager();
            void loopWiFiManager();

            void espRestart(char mmode, char* message);
            void eraseFlash(unsigned int eepFrom = 0, unsigned int eepTo = EEPROM_SIZE);

            void writeConfig(bool wifiSave=false);
            bool readConfig();
            void buttonLoop();
            void JSONerror(String err);
            void saveConfigCallback();
            void sendDebugMessage();

        private:
            //String  _appName;				// may not be necessary
            //String  _appVersion;			// may not be necessary
            String  _firmware;
            String  _compDate;
            int     _modeButton;
            int     _nrXF = 0;				// nr of extra fields required in the config manager
            bool    _serialDebug;
            bool    _setPreSet = false;		// ;)
    };

#endif
