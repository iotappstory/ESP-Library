#ifndef IOTAppStory_h
	#define IOTAppStory_h



	/**
		------ ------ ------ ------ ------ ------ library includes ------ ------ ------ ------ ------ ------
	*/

	#include <Arduino.h>
	#include "config.h"
	#include "includes.h"


	
	
	/**
		------ ------ ------ ------ ------ ------ macros for debugging ------ ------ ------ ------ ------ ------ 
	*/

	#if DEBUG_LVL >= 1

		// set to true to include code for show EEPROM contents in debug
		#ifndef DEBUG_EEPROM_CONFIG
			#define DEBUG_EEPROM_CONFIG false
		#endif

		// macros for debugging
		#ifdef DEBUG_PORT
			#define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
		#else
			#define DEBUG_MSG(...)
		#endif

		#define         DEBUG_PRINT(x)    { Serial.print(x);   }
		#define         DEBUG_PRINTF(...) { Serial.printf(__VA_ARGS__);  }
		#define         DEBUG_PRINTF_P(...) { Serial.printf_P(__VA_ARGS__);  }
		#define         DEBUG_PRINTLN(x)  { Serial.println(x); }
	#endif




	/**
		------ ------ ------ ------ ------ ------ STRUCTURES ------ ------ ------ ------ ------ ------
	**/
	
	typedef struct eFields{
		const char* fieldLabel;
		char* (*varPointer);
		int length;
		char type;
	};

	typedef struct strConfig{
		char actCode[7];                         // saved IotAppStory activation code
		char appName[33];
		char appVersion[12];
		char ssid[3][STRUCT_CHAR_ARRAY_SIZE];    // 3x SSID
		char password[3][STRUCT_PASSWORD_SIZE];  // 3x PASS
		char deviceName[STRUCT_BNAME_SIZE];
		char compDate[STRUCT_COMPDATE_SIZE];     // saved compile date time
		#if HTTPS_8266_TYPE == FNGPRINT
			char sha1[60];
		#endif
		#if CFG_AUTHENTICATE == true
			char cfg_pass[17];
		#endif
		#if NEXT_OTA == true
			char next_md5[33];
		#endif
		const char magicBytes[4];
	};
	
	struct firmwareStruct{
		bool		success;
		uint32_t 	xlength = 0;
		String		xname;
		String		xver;
		String		xmd5;
	};
		
	enum  ModeButtonState{
		ModeButtonNoPress,                       // mode button is not pressed
		ModeButtonShortPress,                    // short press - will enter in firmware update mode
		ModeButtonLongPress,                     // long press - will enter in configuration mode
		ModeButtonVeryLongPress,                 // very long press - won't do anything (but the app developer might want to do something)
		ModeButtonFirmwareUpdate,                // about to enter in firmware update mode
		ModeButtonConfigMode                     // about to enter in configuration mode
	};

		
	// callback template definition
	typedef std::function<void(void)> THandlerFunction;
	typedef std::function<void(int, int)> THandlerFunctionArg;

	extern UpdateESPClass UpdateESP;


	/**
		------ ------ ------ ------ ------ ------ PROGMEM ------ ------ ------ ------ ------ ------
	*/

	const char SER_DEV[] PROGMEM          = "*-------------------------------------------------------------------------*";
	
	const char HTTP_200[] PROGMEM         = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	const char HTTP_TEMP_START[] PROGMEM  = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><meta name=\"theme-color\" content=\"#000\" />{h}<title>Config</title></head><body>";
	const char HTTP_TEMP_END[] PROGMEM    = "</body></html>";

	const char HTTP_STA_JS[] PROGMEM      = "<script src=\"https://iotappstory.com/ota/cfg-esp2/cfg.js\"></script>";
	const char HTTP_AP_JS[] PROGMEM       = "<script>var d=document,p=d.getElementById(\"po\"),b=d.getElementById(\"b\"),m=d.getElementById(\"m\");function c(e){d.getElementById(\"s\").value=e.innerText||e.textContent,d.getElementById(\"p\").focus()}function ld(e){var t=\"\",n=new XMLHttpRequest;n.open(\"POST\",e,!0),n.setRequestHeader(\"Content-type\",\"application/x-www-form-urlencoded; charset=UTF-8\"),p.classList.add(\"fi\");var o=d.querySelectorAll(\"input,select\");for(i=0;i<o.length;i++)\"\"!=t&&(t+=\"&\"),t+=o[i].name+\"=\"+o[i].value;n.onreadystatechange=function(){if(4==this.readyState&&200==this.status){var e=this.responseText;if(e.includes(\"1:\")){var t=e.split(\":\"),n=\"http://\"+t[1];m.innerHTML='Connected!<br>Changed to STA mode.<br>You will be redirected to<br><a href=\"http://'+t[1]+'\">'+t[1]+'</a><br>in <b id=\"cd\">22</b>s',cd(22),setTimeout(function(){window.location.href=n},22e3)}else\"2\"==e?(setTimeout(function(){ld(\"wsa\")},8500),m.innerHTML=\"Connecting...\"):m.innerHTML+='<br>Failed to connect. Try again<button class=\"btn sm\" id=\"b\" onclick=\"ep()\">Ok</button>';m.classList.add(\"fi\")}},n.send(t)}function cd(e){setTimeout(function(){e--,d.getElementById(\"cd\").innerHTML=e,cd(e)},1e3)}function ep(){p.classList.remove(\"fi\"),m.classList.remove(\"fi\"),t.innerHTML=\"\"}</script>";

	const char HTTP_AP_CSS[] PROGMEM      = "<style>#po,body,html{height:100%;width:100%}#m,.fi{opacity:0}#m,#m a{color:#000}#cnt,body{position:relative}#po,body,html,input{width:100%}body,body a,input{color:#FFF}.btn,body a:hover{text-decoration:none}@keyframes kfi{to{z-index:10;opacity:1}}.fi{z-index:10;animation:kfi .5s ease-in 1 forwards}body,html{padding:0;margin:0;-webkit-box-sizing:border-box;-moz-box-sizing:border-box;box-sizing:border-box}*,:after,:before{-webkit-box-sizing:inherit;-moz-box-sizing:inherit;box-sizing:inherit}body{background:#000;font:14px sans-serif}#po{padding-top:100px;position:fixed;z-index:0;background:rgba(0,0,0,.8);text-align:center;opacity:1}#m{width:200px;padding:10px;display:inline-block;background:#FFF;text-align:left}#cnt{min-width:280px;max-width:425px;margin:0 auto;padding:0 15px;z-index:2;border:1px solid #000}input{padding:4px 8px;margin:4px 0 10px;background:0;outline:0;border:1px solid #ccc}input:focus{border-color:#fcbc12}.btn{width:50%;margin:10px 0 0;padding:5px 14px 8px;display:block;float:right;background:#fcbc12;border-radius:4px;border:0;font-size:16px;color:#fff;text-align:center}.btn.sm{font-size:15px;padding:3px 14px 4px}.btn:active,.btn:focus,.btn:hover{background:#d3d3d3}table{width:100%}table tr:nth-child(2n-1){background:#1A1A1A}table td{padding:3px 4px}table td:nth-child(3):not([data-e=\"7\"]){width:20px;background:url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAACXBIWXMAAAsTAAALEwEAmpwYAAABFElEQVQ4je2Pu07DUBBEz/oGBAUPG3dpEEI0CApEjSIlvIrwP6Hj9TN8QmxACKhIgeySHtFgRAMKvvZSxJFIcIoISqZZaWdnZ0YoQRr4NSFvIWwUq47mejqx83o3fCs/xKF7Jkir4N6KOQtkAkemkZx8v3cGxd6eIIfAs8K+qSeuuU1c0CbwonCcBn6tLDUA2YUX29DTtL1QH+bspX9gQ09t6N2XVtCbOTfrmkQgNo1kvczABu4jIkvmY2pGmk/vAxXsp6wBqEg8MqIjEeCk093V/qoCoOdMZrmz3M+jV/OLpRUztOear2iHB9kkFQAbetfA1kjnMijtynayW1SQ6lhiQIRqL80v8f/g7x5oNK5QlQjgCxBXWhCbZi53AAAAAElFTkSuQmCC) right center no-repeat;background-width:16px}table td:nth-child(3)[data-e=\"7\"]{background:0 0}</style>";

	const char HTTP_WIFI_SCAN[] PROGMEM   = "<tr><td onclick=\"c(this);\">{s}</td><td>{q} dBm</td><td data-e=\"{e}\"></td></tr>";
	const char HTTP_WIFI_FORM[] PROGMEM   = "<div id=\"po\"><div id=\"m\"></div></div><div id=\"cnt\"><h1>WIFI CONNECTION</h1><table>{r}</table><br><br><label>SSID</label><input id=\"s\" name=\"s\" maxlength=50 placeholder=\"SSID\"><label>Password</label><input id=\"p\" name=\"p\" maxlength=64 placeholder=\"password\"><br><br><button class=\"btn\" onclick=\"ld('/wsa')\">Save</button></div>";

	const char HTTP_APP_INFO[] PROGMEM    = "{\"l\":\"{l}\", \"v\":\"{v}\", \"n\":\"{n}\", \"m\":\"{m}\", \"t\":\"{t}\"}";

	#if HTTPS_8266_TYPE == FNGPRINT
		const char HTTP_DEV_INFO[] PROGMEM = "{\"s1\":\"{s1}\", \"s2\":\"{s2}\", \"s3\":\"{s3}\", \"cid\":\"{cid}\", \"fss\":\"{fss}\", \"ss\":\"{ss}\", \"fs\":\"{fs}\", \"ab\":\"{ab}\", \"ac\":\"{ac}\", \"mc\":\"{mc}\", \"xf\":\"{xf}\", \"f\":\"{f}\"}";
	#else
		const char HTTP_DEV_INFO[] PROGMEM = "{\"s1\":\"{s1}\", \"s2\":\"{s2}\", \"s3\":\"{s3}\", \"cid\":\"{cid}\", \"fss\":\"{fss}\", \"ss\":\"{ss}\", \"fs\":\"{fs}\", \"ab\":\"{ab}\", \"ac\":\"{ac}\", \"mc\":\"{mc}\", \"xf\":\"{xf}\"}";
	#endif



	class IOTAppStory {
		
		

		public:
		
		/** 
			------ ------ ------ ------ ------ ------ VARIABLES ------ ------ ------ ------ ------ ------
		*/

		int bootTimes;
		char boardMode = 'N';                    // Normal operation or Configuration mode?
		String statusMessage = "";

		strConfig config = {
			"",
			"",
			"",
			{"","",""},
			{"","",""},
			"yourESP",
			"",
			#if HTTPS_8266_TYPE == FNGPRINT
				HTTPS_FNGPRINT,
			#endif
			#if CFG_AUTHENTICATE == true
				CFG_PASS,
			#endif
			#if NEXT_OTA == true
				"00000000000000000000000000000000",
			#endif
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


		/** 
			------ ------ ------ ------ ------ ------ FUCNTION DEFINITIONS ------ ------ ------ ------ ------ ------ 
		*/

		IOTAppStory(const char *compDate, const int modeButton);

		// function for pre setting config parameters ssid & password, deviceName, automatic update, HOST1 and FILE1
		void preSetAppName(String appName);
		void preSetAppVersion(String appVersion);
		void preSetDeviceName(String deviceName);
		void preSetAutoUpdate(bool automaticUpdate);
		void preSetAutoConfig(bool automaticConfig);
		void preSetWifi(String ssid, String password);
		//void preSetServer(String HOST1, String FILE1);

		void setCallHome(bool callHome);
		void setCallHomeInterval(unsigned long interval);

		void begin(const char ea);

		void loop();


		void writeConfig(bool wifiSave = false);
		void readConfig();
		void espRestart(char mmode);
		void eraseFlash(int eepFrom, int eepTo);

		void connectNetwork();
		bool isNetworkConnected(bool multi = true);
		void setClock();
		
		void callHome(bool spiffs = true);
		bool iotUpdater(int command = U_FLASH);
		bool espInstaller(Stream &streamPtr, firmwareStruct *firmwareStruct, UpdateClassVirt& devObj, int command = U_FLASH);
		
		void addField(char* &defaultVal, const char *fieldLabel, const int length, const char type = 'L');
		void iasLog(String msg);
		int dPinConv(String orgVal);

		void onFirstBoot(THandlerFunction fn);              	// called at the end of firstBoot
		//void onWifiConnected(THandlerFunction fn);        	// called on succesfull Wifi connection SYSTEM_EVENT_STA_GOT_IP
		//void onWifiDisonnected(THandlerFunction fn);      	// called when Wifi is dissconnected SYSTEM_EVENT_STA_DISCONNECTED

		void onModeButtonNoPress(THandlerFunction fn);      	// called when state is changed to idle (mode button is not pressed)
		void onModeButtonShortPress(THandlerFunction fn);   	// called when state is changed to short press
		void onModeButtonLongPress(THandlerFunction fn);    	// called when state is changed to long press
		void onModeButtonVeryLongPress(THandlerFunction fn); 	// called when state is changed to very long press

		void onFirmwareUpdateCheck(THandlerFunction fn);    	// called when the app checks for firmware updates
		void onFirmwareUpdateDownload(THandlerFunction fn); 	// called when firmware download starts
		void onFirmwareUpdateProgress(THandlerFunctionArg fn); 	// called during update process
		void onFirmwareUpdateError(THandlerFunction fn);    	// called when firmware update ends in an error
		void onFirmwareUpdateSuccess(THandlerFunction fn);  	// called when firmware update ends in success

		void onConfigMode(THandlerFunction fn);             	// called when the app is about to enter in configuration mode

		private:
		
		#if WIFI_MULTI == true
			#ifdef ESP32
				WiFiMulti wifiMulti;
			#elif defined ESP8266
				ESP8266WiFiMulti wifiMulti;
			#endif
		#endif

		const char *_compDate;
		const int _modeButton;
		unsigned int _nrXF                   = 0;       // nr of extra fields required in the config manager

		bool _updateOnBoot                   = true;    // update on boot? (end of begin();)
		bool _automaticConfig                = true;    // automaticly go to config on boot if there is no wifi connection present

		bool _setPreSet                      = false;   // ;) have there been any preSets set?
		bool _setDeviceName                  = false;   // is the device name set?
		bool _configReaded                   = false;   // has the config already been read?
		bool _callHome                       = false;

		bool _tryToConn                      = false;   // try to connect to wifi bool
		bool _tryToConnFail                  = false;   // try to connect to wifi bool
		bool _connected                      = false;   // wifi connection status bool
		bool _writeConfig                    = false;
		bool _changeMode                     = false;

		unsigned long _lastCallHomeTime      = 0;       // Time when we last called home
		unsigned long _callHomeInterval      = 7200000; // Interval we want to call home at in milliseconds, default start at 2hrs

		unsigned long _buttonEntry;
		unsigned long _debugEntry;
		AppState      _appState;

		eFields fieldStruct[MAXNUMEXTRAFIELDS];


		THandlerFunction _firstBootCallback;
		//THandlerFunction _wifiConnectedCallback;
		//THandlerFunction _wifiDisonnectedCallback;

		THandlerFunction _noPressCallback;
		THandlerFunction _shortPressCallback;
		THandlerFunction _longPressCallback;
		THandlerFunction _veryLongPressCallback;

		THandlerFunction _firmwareUpdateCheckCallback;
		THandlerFunction _firmwareUpdateDownloadCallback;
		
		THandlerFunctionArg _firmwareUpdateProgressCallback;
		
		THandlerFunction _firmwareUpdateErrorCallback;
		THandlerFunction _firmwareUpdateSuccessCallback;
		THandlerFunction _configModeCallback;


		/**
			------ ------ ------ ------ ------ ------ FUNCTION DEFINITIONS ------ ------ ------ ------ ------ ------
		*/

		void firstBoot(const char ea);
		void printBoardInfo();
		void processField();


		String strWifiScan();
		String strCertScan(String path = "");
		String servHdlRoot();
		String servHdlDevInfo();
		String servHdlAppInfo();
		/*#if defined  ESP8266
			String servHdlFngPrintSave(String fngprint);
		#endif*/
		String servHdlWifiSave(String newSSID, String newPass, int apNr=0);
		String servHdlAppSave(AsyncWebServerRequest *request);
		String servHdlactcodeSave(String actcode="");

		void updateLoop();
		bool isModeButtonPressed();
		ModeButtonState getModeButtonState();
		ModeButtonState buttonLoop();


		friend class configServer;

		/** 
			------ ------ ------ ------ ------ ------ AUXILIARY FUNCTIONS ------ ------ ------ ------ ------ ------
		*/

		bool SetConfigValueCharArray(char* a, String &b, int len, bool &changeFlag) {
			if (b != a) {
				b.toCharArray(a, len);
				changeFlag = true;
				return true;
			} else {
				return false;
			}
		}
		/* ------ */

		/* ------ CONVERT BYTE TO STRING   */
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
