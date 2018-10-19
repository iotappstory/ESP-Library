#ifndef IOTAppStory_h
	#define IOTAppStory_h



	/**
		------ ------ ------ ------ ------ ------ library includes ------ ------ ------ ------ ------ ------
	*/

	#include "config.h"

	#ifdef ESP32
		#include <WiFi.h>
		#include <WiFiMulti.h>
		#include <ESPmDNS.h>
		#include "esp32/boardInfo.h"
		#include <AsyncTCP.h>                    // https://github.com/me-no-dev/AsyncTCP
		#include <HTTPClient.h>
	#elif defined  ESP8266
		#include <ESP8266WiFi.h>
		#include <ESP8266WiFiMulti.h>
		#include <ESP8266mDNS.h>
		#include "esp8266/boardInfo.h"
		#include <StreamString.h>
		#include <ESPAsyncTCP.h>                 // https://github.com/me-no-dev/ESPAsyncTCP
		#include <ESP8266HTTPClient.h>
	#endif

	#include "ESPhttpUpdateIasMod.h"
	#include <DNSServer.h> 
	#include <FS.h>
	#include <EEPROM.h>
	#include <ESPAsyncWebServer.h>               // https://github.com/me-no-dev/ESPAsyncWebServer




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
	*/
	typedef struct {
		const char *fieldLabel;
		char* (*varPointer);
		int length;
		char type;
	} eFields;

	typedef struct {
		char actCode[7];                         // saved IotAppStory activation code
		char appName[33];
		char appVersion[12];
		char ssid[3][STRUCT_CHAR_ARRAY_SIZE];    // 3x SSID
		char password[3][STRUCT_PASSWORD_SIZE];  // 3x PASS
		char deviceName[STRUCT_BNAME_SIZE];
		char compDate[STRUCT_COMPDATE_SIZE];     // saved compile date time
		#if defined  ESP8266
			char sha1[60];
		#endif
		#if CFG_AUTHENTICATE == true
			char cfg_pass[17];
		#endif
		const char magicBytes[4];
	} strConfig;

	enum ModeButtonState {
		ModeButtonNoPress,                       // mode button is not pressed
		ModeButtonShortPress,                    // short press - will enter in firmware update mode
		ModeButtonLongPress,                     // long press - will enter in configuration mode
		ModeButtonVeryLongPress,                 // very long press - won't do anything (but the app developer might want to do something)
		ModeButtonFirmwareUpdate,                // about to enter in firmware update mode
		ModeButtonConfigMode                     // about to enter in configuration mode
	};

	typedef std::function<void(void)> THandlerFunction;




	/**
		------ ------ ------ ------ ------ ------ PROGMEM ------ ------ ------ ------ ------ ------
	*/

	const char SER_DEV[] PROGMEM          = "*-------------------------------------------------------------------------*";
	
	const char HTTP_200[] PROGMEM         = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	const char HTTP_TEMP_START[] PROGMEM  = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><meta name=\"theme-color\" content=\"#000\" />{h}<title>Config</title></head><body>";
	const char HTTP_TEMP_END[] PROGMEM    = "</body></html>";

	const char HTTP_STA_JS[] PROGMEM      = "<script src=\"https://iotappstory.com/ota/cfg-esp/cfg.js\"></script>";
	const char HTTP_AP_JS[] PROGMEM       = "<script>var d=document,p=d.getElementById(\"po\"),b=d.getElementById(\"b\"),m=d.getElementById(\"m\");function c(e){d.getElementById(\"s\").value=e.innerText||e.textContent,d.getElementById(\"p\").focus()}function ld(e){var t=\"\",n=new XMLHttpRequest;n.open(\"POST\",e,!0),n.setRequestHeader(\"Content-type\",\"application/x-www-form-urlencoded\"),p.classList.add(\"fi\");var o=d.querySelectorAll(\"input,select\");for(i=0;i<o.length;i++)\"\"!=t&&(t+=\"&\"),t+=o[i].name+\"=\"+o[i].value;n.onreadystatechange=function(){if(4==this.readyState&&200==this.status){var e=this.responseText;if(e.includes(\"1:\")){var t=e.split(\":\"),n=\"http://\"+t[1];m.innerHTML='Connected!<br>Changed to STA mode.<br>You will be redirected to<br><a href=\"http://'+t[1]+'\">'+t[1]+'</a><br>in <b id=\"cd\">22</b>s',cd(22),setTimeout(function(){window.location.href=n},22e3)}else\"2\"==e?(setTimeout(function(){ld(\"wsa\")},8500),m.innerHTML=\"Connecting...\"):m.innerHTML+='<br>Failed to connect. Try again<button class=\"btn sm\" id=\"b\" onclick=\"ep()\">Ok</button>';m.classList.add(\"fi\")}},n.send(t)}function cd(e){setTimeout(function(){e--,d.getElementById(\"cd\").innerHTML=e,cd(e)},1e3)}function ep(){p.classList.remove(\"fi\"),m.classList.remove(\"fi\"),t.innerHTML=\"\"}</script>";

	const char HTTP_AP_CSS[] PROGMEM      = "<style>#po,body,html{height:100%;width:100%}#m,.fi{opacity:0}#m,#m a{color:#000}#cnt,body{position:relative}#po,body,html,input{width:100%}body,body a,input{color:#FFF}.btn,body a:hover{text-decoration:none}@keyframes kfi{to{z-index:10;opacity:1}}.fi{z-index:10;animation:kfi .5s ease-in 1 forwards}body,html{padding:0;margin:0;-webkit-box-sizing:border-box;-moz-box-sizing:border-box;box-sizing:border-box}*,:after,:before{-webkit-box-sizing:inherit;-moz-box-sizing:inherit;box-sizing:inherit}body{background:#000;font:14px sans-serif}#po{padding-top:100px;position:fixed;z-index:0;background:rgba(0,0,0,.8);text-align:center;opacity:1}#m{width:200px;padding:10px;display:inline-block;background:#FFF;text-align:left}#cnt{min-width:280px;max-width:425px;margin:0 auto;padding:0 15px;z-index:2;border:1px solid #000}input{padding:4px 8px;margin:4px 0 10px;background:0;outline:0;border:1px solid #ccc}input:focus{border-color:#fcbc12}.btn{width:50%;margin:10px 0 0;padding:5px 14px 8px;display:block;float:right;background:#fcbc12;border-radius:4px;border:0;font-size:16px;color:#fff;text-align:center}.btn.sm{font-size:15px;padding:3px 14px 4px}.btn:active,.btn:focus,.btn:hover{background:#d3d3d3}table{width:100%}table tr:nth-child(2n-1){background:#1A1A1A}table td{padding:3px 4px}table td:nth-child(3):not([data-e=\"7\"]){width:20px;background:url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAACXBIWXMAAAsTAAALEwEAmpwYAAABFElEQVQ4je2Pu07DUBBEz/oGBAUPG3dpEEI0CApEjSIlvIrwP6Hj9TN8QmxACKhIgeySHtFgRAMKvvZSxJFIcIoISqZZaWdnZ0YoQRr4NSFvIWwUq47mejqx83o3fCs/xKF7Jkir4N6KOQtkAkemkZx8v3cGxd6eIIfAs8K+qSeuuU1c0CbwonCcBn6tLDUA2YUX29DTtL1QH+bspX9gQ09t6N2XVtCbOTfrmkQgNo1kvczABu4jIkvmY2pGmk/vAxXsp6wBqEg8MqIjEeCk093V/qoCoOdMZrmz3M+jV/OLpRUztOear2iHB9kkFQAbetfA1kjnMijtynayW1SQ6lhiQIRqL80v8f/g7x5oNK5QlQjgCxBXWhCbZi53AAAAAElFTkSuQmCC) right center no-repeat;background-width:16px}table td:nth-child(3)[data-e=\"7\"]{background:0 0}</style>";

	const char HTTP_WIFI_SCAN[] PROGMEM   = "<tr><td onclick=\"c(this);\">{s}</td><td>{q} dBm</td><td data-e=\"{e}\"></td></tr>";
	const char HTTP_WIFI_FORM[] PROGMEM   = "<div id=\"po\"><div id=\"m\"></div></div><div id=\"cnt\"><h1>WIFI CONNECTION</h1><table>{r}</table><br><br><label>SSID</label><input id=\"s\" name=\"s\" maxlength=50 placeholder=\"SSID\"><label>Password</label><input id=\"p\" name=\"p\" maxlength=50 placeholder=\"password\"><br><br><button class=\"btn\" onclick=\"ld('/wsa')\">Save</button></div>";

	const char HTTP_APP_INFO[] PROGMEM    = "{\"l\":\"{l}\", \"v\":\"{v}\", \"n\":\"{n}\", \"m\":\"{m}\", \"t\":\"{t}\"}";

	#if defined  ESP8266
		const char HTTP_DEV_INFO[] PROGMEM = "{\"s1\":\"{s1}\", \"s2\":\"{s2}\", \"s3\":\"{s3}\", \"cid\":\"{cid}\", \"fid\":\"{fid}\", \"fss\":\"{fss}\", \"ss\":\"{ss}\", \"fs\":\"{fs}\", \"ab\":\"{ab}\", \"ac\":\"{ac}\", \"mc\":\"{mc}\", \"xf\":\"{xf}\", \"f\":\"{f}\"}";
	#endif

	#if defined ESP32
		const char HTTP_DEV_INFO[] PROGMEM = "{\"s1\":\"{s1}\", \"s2\":\"{s2}\", \"s3\":\"{s3}\", \"cid\":\"{cid}\", \"fid\":\"{fid}\", \"fss\":\"{fss}\", \"ss\":\"{ss}\", \"fs\":\"{fs}\", \"ab\":\"{ab}\", \"ac\":\"{ac}\", \"mc\":\"{mc}\", \"xf\":\"{xf}\"}";
		
		const char ROOT_CA[] PROGMEM = \
			"-----BEGIN CERTIFICATE-----\n" \
			"MIIF2DCCA8CgAwIBAgIQTKr5yttjb+Af907YWwOGnTANBgkqhkiG9w0BAQwFADCB\n" \
			"hTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4G\n" \
			"A1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNV\n" \
			"BAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAwMTE5\n" \
			"MDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBhTELMAkGA1UEBhMCR0IxGzAZBgNVBAgT\n" \
			"EkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMR\n" \
			"Q09NT0RPIENBIExpbWl0ZWQxKzApBgNVBAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNh\n" \
			"dGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCR\n" \
			"6FSS0gpWsawNJN3Fz0RndJkrN6N9I3AAcbxT38T6KhKPS38QVr2fcHK3YX/JSw8X\n" \
			"pz3jsARh7v8Rl8f0hj4K+j5c+ZPmNHrZFGvnnLOFoIJ6dq9xkNfs/Q36nGz637CC\n" \
			"9BR++b7Epi9Pf5l/tfxnQ3K9DADWietrLNPtj5gcFKt+5eNu/Nio5JIk2kNrYrhV\n" \
			"/erBvGy2i/MOjZrkm2xpmfh4SDBF1a3hDTxFYPwyllEnvGfDyi62a+pGx8cgoLEf\n" \
			"Zd5ICLqkTqnyg0Y3hOvozIFIQ2dOciqbXL1MGyiKXCJ7tKuY2e7gUYPDCUZObT6Z\n" \
			"+pUX2nwzV0E8jVHtC7ZcryxjGt9XyD+86V3Em69FmeKjWiS0uqlWPc9vqv9JWL7w\n" \
			"qP/0uK3pN/u6uPQLOvnoQ0IeidiEyxPx2bvhiWC4jChWrBQdnArncevPDt09qZah\n" \
			"SL0896+1DSJMwBGB7FY79tOi4lu3sgQiUpWAk2nojkxl8ZEDLXB0AuqLZxUpaVIC\n" \
			"u9ffUGpVRr+goyhhf3DQw6KqLCGqR84onAZFdr+CGCe01a60y1Dma/RMhnEw6abf\n" \
			"Fobg2P9A3fvQQoh/ozM6LlweQRGBY84YcWsr7KaKtzFcOmpH4MN5WdYgGq/yapiq\n" \
			"crxXStJLnbsQ/LBMQeXtHT1eKJ2czL+zUdqnR+WEUwIDAQABo0IwQDAdBgNVHQ4E\n" \
			"FgQUu69+Aj36pvE8hI6t7jiY7NkyMtQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB\n" \
			"/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAArx1UaEt65Ru2yyTUEUAJNMnMvl\n" \
			"wFTPoCWOAvn9sKIN9SCYPBMtrFaisNZ+EZLpLrqeLppysb0ZRGxhNaKatBYSaVqM\n" \
			"4dc+pBroLwP0rmEdEBsqpIt6xf4FpuHA1sj+nq6PK7o9mfjYcwlYRm6mnPTXJ9OV\n" \
			"2jeDchzTc+CiR5kDOF3VSXkAKRzH7JsgHAckaVd4sjn8OoSgtZx8jb8uk2Intzna\n" \
			"FxiuvTwJaP+EmzzV1gsD41eeFPfR60/IvYcjt7ZJQ3mFXLrrkguhxuhoqEwWsRqZ\n" \
			"CuhTLJK7oQkYdQxlqHvLI7cawiiFwxv/0Cti76R7CZGYZ4wUAc1oBmpjIXUDgIiK\n" \
			"boHGhfKppC3n9KUkEEeDys30jXlYsQab5xoq2Z0B15R97QNKyvDb6KkBPvVWmcke\n" \
			"jkk9u+UJueBPSZI9FoJAzMxZxuY67RIuaTxslbH9qh17f4a+Hg4yRvv7E491f0yL\n" \
			"S0Zj/gA0QHDBw7mh3aZw4gSzQbzpgJHqZJx64SIDqZxubw5lT2yHh17zbqD5daWb\n" \
			"QOhTsiedSrnAdyGN/4fy3ryM7xfft0kL0fJuMAsaDk527RH89elWsn2/x20Kk4yl\n" \
			"0MC2Hb46TpSi125sC8KKfPog88Tk5c0NqMuRkrF8hey1FGlmDoLnzc7ILaZRfyHB\n" \
			"NVOFBkpdn627G190\n" \
			"-----END CERTIFICATE-----\n";
	#endif



	class IOTAppStory {

		public:
		/** 
			------ ------ ------ ------ ------ ------ VARIABLES ------ ------ ------ ------ ------ ------
		*/

		int bootTimes;							// nr of times this device booted since powerup
		char boardMode = 'N';                   // Normal operation or Configuration mode?
		unsigned int eepFreeFrom;				// From where can I use eeprom?
		

		strConfig config = {
			"",
			"",
			"",
			{"","",""},
			{"","",""},
			"yourESP",
			"",
			#if defined  ESP8266
				"34:6D:0A:26:F0:40:3A:0A:1B:F1:CA:8E:C8:0C:F5:14:21:83:7C:B1",
			#endif
			#if CFG_AUTHENTICATE == true
				"admin",
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

		void begin(char ea);

		void loop();


		void writeConfig(bool wifiSave = false);
		void readConfig();
		void espRestart(char mmode);
		void eraseFlash(int eepFrom, int eepTo);

		void connectNetwork();
		bool isNetworkConnected(bool multi = true);

		void callHome(bool spiffs = true);
		void iotUpdater(bool spiffs);
		void addField(char* &defaultVal,const char *fieldLabel, int length, char type = 'L');
		void iasLog(String msg = "");
		void runConfigServer();
		int dPinConv(String orgVal);

		void onFirstBoot(THandlerFunction fn);              // called at the end of firstBoot
		//void onWifiConnected(THandlerFunction fn);        // called on succesfull Wifi connection SYSTEM_EVENT_STA_GOT_IP
		//void onWifiDisonnected(THandlerFunction fn);      // called when Wifi is dissconnected SYSTEM_EVENT_STA_DISCONNECTED

		void onModeButtonNoPress(THandlerFunction fn);      // called when state is changed to idle (mode button is not pressed)
		void onModeButtonShortPress(THandlerFunction fn);   // called when state is changed to short press
		void onModeButtonLongPress(THandlerFunction fn);    // called when state is changed to long press
		void onModeButtonVeryLongPress(THandlerFunction fn); // called when state is changed to very long press

		void onFirmwareUpdateCheck(THandlerFunction fn);    // called when the app checks for firmware updates
		void onFirmwareUpdateDownload(THandlerFunction fn); // called when firmware update
		void onFirmwareUpdateError(THandlerFunction fn);    // called when firmware update ends in an error
		void onFirmwareUpdateSuccess(THandlerFunction fn);  // called when firmware update ends in success

		void onConfigMode(THandlerFunction fn);             // called when the app is about to enter in configuration mode




		private:

		std::unique_ptr<DNSServer>        dnsServer;
		std::unique_ptr<AsyncWebServer> 	server;
		//std::unique_ptr<Preferences> 			preferences;

		#if WIFI_MULTI == true
			#ifdef ESP32
				WiFiMulti wifiMulti;
			#elif defined  ESP8266
				ESP8266WiFiMulti wifiMulti;
			#endif
		#endif

		const char *_compDate;
		const int _modeButton;
		unsigned int _nrXF                   = 0;                         // nr of extra fields required in the config manager


		const char* _updateHost              = "iotappstory.com";         // ota update host
		#if defined  ESP8266
			const char* _updateFile          	= "/ota/esp8266-v2.0.1.php"; // loc1, file at host that handles 8266 updates
		#elif defined ESP32
			const char* _updateFile          	= "/ota/esp32-v1.php";       // loc1, file at host that handles 32 updates
		#endif


		bool _updateOnBoot                   = true;    // update on boot? (end of begin();)
		bool _automaticConfig                = true;    // automaticly go to config on boot if there is no wifi connection present

		bool _setPreSet                      = false;   // ;) have there been any preSets set?
		bool _setDeviceName                  = false;   // is the device name set?
		bool _configReaded                   = false;   // has the config already been read?
		bool _callHome                       = false;

		bool _tryToConn                      = false;   // try to connect to wifi bool
		bool _tryToConnFail                  = false;   // try to connect to wifi bool
		bool _connected                      = false;   // wifi connection status bool
		bool _tryToConf                      = false;   // try to confirm device registration bool
		int _confirmed                       = false;   // confirmed status bool
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
		THandlerFunction _firmwareUpdateErrorCallback;
		THandlerFunction _firmwareUpdateSuccessCallback;

		THandlerFunction _configModeCallback;


		/**
			------ ------ ------ ------ ------ ------ FUNCTION DEFINITIONS ------ ------ ------ ------ ------ ------
		*/

		void firstBoot(char ea);
		void printBoardInfo();
		void processField();
		void httpClientSetup(HTTPClient& http, String url, bool spiffs=false);

		String strWifiScan();

		void servHdlRoot(AsyncWebServerRequest *request);
		void servHdlDevInfo(AsyncWebServerRequest *request);
		#if defined  ESP8266
			void servHdlFngPrintSave(AsyncWebServerRequest *request);
		#endif
		void servHdlWifiScan(AsyncWebServerRequest *request);
		void servHdlWifiSave(AsyncWebServerRequest *request);
		void servHdlAppInfo(AsyncWebServerRequest *request);
		void servHdlAppSave(AsyncWebServerRequest *request);
		//void servHdlDevSave(AsyncWebServerRequest *request);

		void hdlReturn(AsyncWebServerRequest *request, String &retHtml, String type = "text/html");

		void updateLoop();
		bool isModeButtonPressed();
		ModeButtonState getModeButtonState();
		ModeButtonState buttonLoop();




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
