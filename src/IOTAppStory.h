#ifndef IOTAppStory_h
    #define IOTAppStory_h

    #include <Arduino.h>
    #include <functional>

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
    #define MODE_BUTTON_SHORT_PRESS       500
    #define MODE_BUTTON_LONG_PRESS        4000
    #define MODE_BUTTON_VERY_LONG_PRESS   10000


    // macros for debugging
    #ifdef DEBUG_PORT
        #define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
    #else
        #define DEBUG_MSG(...)
    #endif

    //#ifdef SERIALDEBUG
    #define         DEBUG_PRINT(x)    { if(_serialDebug) Serial.print(x);   }
    #define         DEBUG_PRINTF(...) { if(_serialDebug) Serial.printf(__VA_ARGS__);  }
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

    enum ModeButtonState {
        ModeButtonNoPress,           // mode button is not pressed
        ModeButtonShortPress,        // short press - will enter in firmware update mode
        ModeButtonLongPress,         // long press - will enter in configuration mode
        ModeButtonVeryLongPress,     // very long press - won't do anything (but the app developer might want to do something)
        ModeButtonFirmwareUpdate,    // about to enter in firmware update mode
        ModeButtonConfigMode         // about to enter in configuration mode
    };

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
    
            // similar to the ModeButtonState but has some extra states
            // that are used internally by the state machine method
            enum AppState {
                AppStateNoPress,        // mode button is not pressed
                AppStateWaitPress,      // mode button is pressed but not long enough for a short press
                AppStateShortPress,     // mode button is pressed for a short time. Releasing it will make it go to firmware update mode.
                AppStateLongPress,      // mode button is pressed for a long time. Releasing it will make it go to config mode.
                AppStateVeryLongPress,  // mode button is pressed for a very long time. Releasing it won't do anything.
                AppStateFirmwareUpdate, // about to enter in firmware update mode
                AppStateConfigMode      // about to enter in config mode
            };

            typedef std::function<void(void)> THandlerFunction;
            
            eFields fieldStruct[MAXNUMEXTRAFIELDS];

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
            ModeButtonState buttonLoop();
            void JSONerror(String err);
            void saveConfigCallback();
            void sendDebugMessage();
            bool isModeButtonPressed();
            ModeButtonState getModeButtonState();
    
            // called when state is changed to idle (mode button is not pressed)
            void onModeButtonNoPress(THandlerFunction fn);
    
            // called when state is changed to short press
            void onModeButtonShortPress(THandlerFunction fn);
    
            // called when state is changed to long press
            void onModeButtonLongPress(THandlerFunction fn);
    
            // called when state is changed to very long press
            void onModeButtonVeryLongPress(THandlerFunction fn);
    
            // called when the app is about to update the firmware
            void onModeButtonFirmwareUpdate(THandlerFunction fn);
    
            // called when the app is about to enter in configuration mode
            void onModeButtonConfigMode(THandlerFunction fn);
    

        private:
            //String  _appName;				// may not be necessary
            //String  _appVersion;			// may not be necessary
            String  _firmware;
            String  _compDate;
            int     _modeButton;
            int     _nrXF = 0;				// nr of extra fields required in the config manager
            bool    _serialDebug;
            bool    _setPreSet = false;		// ;)
            unsigned long   _buttonEntry;
            unsigned long   _debugEntry;
            AppState        _appState;
    
            THandlerFunction _noPressCallback;
            THandlerFunction _shortPressCallback;
            THandlerFunction _longPressCallback;
            THandlerFunction _veryLongPressCallback;
            THandlerFunction _firmwareUpdateCallback;
            THandlerFunction _configModeCallback;
    };

#endif
