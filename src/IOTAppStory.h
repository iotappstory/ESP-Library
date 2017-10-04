#ifndef IOTAppStory_h
    #define IOTAppStory_h

    #include <Arduino.h>
    #include <functional>

    /* ------ ------ ------ DEFINES for library ------ ------ ------ */
    #define MAGICBYTES "CFG"
    #define EEPROM_SIZE 1024
	#define WIFI_MANAGER_MAX_PARAMS 12
    #define MAXNUMEXTRAFIELDS 12
    #define MAGICEEP "%"
    #define UDP_PORT 514
    #define RTCMEMBEGIN 68
    #define MAGICBYTE 85
    #define STRUCT_CHAR_ARRAY_SIZE 50  // length of config variables
    #define STRUCT_COMPDATE_SIZE 20
	#define STRUCT_BNAME_SIZE 30
    #define STRUCT_HOST_SIZE 24
    #define STRUCT_FILE_SIZE 32

    // constants used to define the status of the mode button based on the time it was pressed. (miliseconds)
    #define MODE_BUTTON_SHORT_PRESS       500
    #define MODE_BUTTON_LONG_PRESS        4000
    #define MODE_BUTTON_VERY_LONG_PRESS   10000

	// sets the default value for the maximum number of retries when trying to connect to the wifi
	#ifndef MAX_WIFI_RETRIES
		#define MAX_WIFI_RETRIES 15
	#endif // !MAX_WIFI_RETRIES

    // macros for debugging
    #ifdef DEBUG_PORT
        #define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
    #else
        #define DEBUG_MSG(...)
    #endif
	
	
	// set to true to include code for show EEPROM contents in debug
	#ifndef DEBUG_EEPROM_CONFIG
		#define DEBUG_EEPROM_CONFIG false
	#endif



    #define         DEBUG_PRINT(x)    { if(_serialDebug) Serial.print(x);   }
    #define         DEBUG_PRINTF(...) { if(_serialDebug) Serial.printf(__VA_ARGS__);  }
    #define         DEBUG_PRINTF_P(...) { if(_serialDebug) Serial.printf_P(__VA_ARGS__);  }
    #define         DEBUG_PRINTLN(x)  { if(_serialDebug) Serial.println(x); }


    #ifdef LEDS_INVERSE
        #define LEDON  0
        #define LEDOFF 1
    #else
        #define LEDON  1
        #define LEDOFF 0
    #endif
	

	 
	 
	 /* ------ ------ ------ STRUCTURES ------ ------ ------ */
	typedef struct {
		byte markerFlag;
		int bootTimes;
		char boardMode = 'N';  	// Normal operation or Configuration mode?
	} rtcMemDef __attribute__((aligned(4)));
	

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
		char boardName[STRUCT_BNAME_SIZE];
		
		const char* (*appName);
		const char* (*appVersion);
		
		char HOST1[STRUCT_HOST_SIZE];
		char FILE1[STRUCT_FILE_SIZE];
		//const char IOTappStory2[16];
		//const char IOTappStoryPHP2[20];

		bool automaticUpdate;	// right after boot
		char compDate[STRUCT_COMPDATE_SIZE];
		char sha1[60];
		char devPass[7];
		
		char magicBytes[4];
	} strConfig;
	
    enum ModeButtonState {
        ModeButtonNoPress,           // mode button is not pressed
        ModeButtonShortPress,        // short press - will enter in firmware update mode
        ModeButtonLongPress,         // long press - will enter in configuration mode
        ModeButtonVeryLongPress,     // very long press - won't do anything (but the app developer might want to do something)
        ModeButtonFirmwareUpdate,    // about to enter in firmware update mode
        ModeButtonConfigMode         // about to enter in configuration mode
    };

	
	/* ------ ------ ------ PROGMEM ------ ------ ------ */
	
	// store "repeating" serial feedback strings in PROGMEM as this uses less than F()
	const char SER_DEV[] PROGMEM           = "*-------------------------------------------------------------------------*";
	 
	const char HOST2[] PROGMEM             = "iotappstory.com";
	const char FILE2[] PROGMEM             = "/ota/esp8266-v2.php";
	
    class IOTAppStory {
		
        public:
            /* ------ ------ ------ VARIABLES ------ ------ ------ */
			rtcMemDef rtcMem;

            strConfig config = {
                "",
                "",
                "yourESP",
                NULL,
                NULL,
                "iotappstory.com",
                "/ota/esp8266-v2.php",
                //"iotappstory.com",
                //"/ota/esp8266-v1.php",
                false,
                "",
                "76:31:B2:F5:9B:5C:F0:8D:CB:D2:D4:4A:B9:71:8B:32:C8:FD:0B:37",			// <<--- needs a field in the config pages
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
            IOTAppStory(const char* appName, const char* appVersion, const char *compDate, const int modeButton);

            void serialdebug(bool onoff,int speed=115200);

            // function for pre setting config parameters ssid & password, boardname, automatic update, IOTappStory1 and FILE1
            void preSetConfig(String boardName, bool automaticUpdate = false);
            void preSetConfig(bool automaticUpdate = false);
            void preSetConfig(String ssid, String password, bool automaticUpdate = false);
            void preSetConfig(String ssid, String password, String boardName, bool automaticUpdate = false);
            void preSetConfig(String ssid, String password, String boardName, String IOTappStory1, String FILE1, bool automaticUpdate = false);

            void begin(bool bootstats=true, bool ea=false); 			// ea = erase all eeprom
            void firstBoot(bool ea=false);

            bool readRTCmem();
            void writeRTCmem();
            void printRTCmem();

            void configESP();

            void connectNetwork();
            bool isNetworkConnected();

            bool callHome(bool spiffs = true);
            byte iotUpdater(bool type, bool loc = false);

            void addField(char* &defaultVal,const char *fieldIdName,const char *fieldLabel, int length);
            void processField();
            int dPinConv(String orgVal);

            void initWiFiManager();
            void loopWiFiManager();

            void espRestart(char mmode, const char* message);
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
            //const char *_appName;
            //String  _appVersion;			// may not be necessary
            String  _firmware;
            const char *_compDate;
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

            bool	_configReaded = false;
            const static bool _boolDefaulValue = false;

            /* ------ ------ ------ AUXILIARY FUNCTIONS ------ ------ ------ */
            /* ------ CHANGE CONFIG VALUES 									 */
            template <typename T, typename T2> bool SetConfigValue(T &a, T2 &b, bool &changeFlag = _boolDefaulValue) {
              if (a != b) {
                a = b;
                changeFlag = true;
                return true;
              } else {
                return false;
              }
            }

            bool SetConfigValueCharArray(char* a, String &b, int len, bool changeFlag = &_boolDefaulValue) {
              if (b != a) {
                b.toCharArray(a, len);
                changeFlag = true;
                return true;
              } else {
                return false;
              }
            }
            /* ------ */

            /* ------ CONVERT BYTE TO STRING 								 */
            String GetCharToDisplayInDebug(char value) {
              if (value>=32 && value<=126){
                return String(value);
              } else if (value == 0){
                return ".";
              } else {
                return String("[" + String(value, DEC) + "]");
              } 
            }
    };

#endif
