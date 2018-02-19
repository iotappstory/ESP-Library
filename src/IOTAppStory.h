#ifndef IOTAppStory_h
	#define IOTAppStory_h
	
	/**
		------ ------ ------ ------ ------ ------ DEFINES for library ------ ------ ------ ------ ------ ------ 
	*/
	
	#define DEBUG_LVL 							2			// Debug level: 0, 1, 2 or 3 | none - max
	#define SERIAL_SPEED						115200
	
	// config 
	#define INC_CONFIG 							true  // include Config mode (Wifimanager!!!)
	#define CFG_PAGE_INFO						true	// include the info page in Config mode
	#define CFG_PAGE_IAS 						true  // include the IAS page in Config mode
	
	// Wifi defines
	#define SMARTCONFIG							false	// Set to true to enable smartconfig by smartphone app "ESP Smart Config" or "ESP8266 SmartConfig" | This will add (2%) of program storage space and 1%) of dynamic memory
	#define WIFI_MULTI							true	// false: only 1 ssid & pass will be used | true: 3 sets of ssid & pass will be used
	#define MAX_WIFI_RETRIES 				20		// sets the maximum number of retries when trying to connect to the wifi
	#define USEMDNS 								true  // include MDNS responder http://yourboard.local
	#define DNS_PORT								53
	#define UDP_PORT      					514
	
	#define HTTPS         					true	// Try to use HTTPS for updates
	#define HEAPFORHTTPS  					31500	// Min free heap needed for HTTPS

	#if defined  ESP8266
		#define EEPROM_SIZE 					4096	// EEPROM_SIZE depending on device
		#define MAXNUMEXTRAFIELDS 		6			// wifimanger | max num of fields that can be added
	#elif defined ESP32
		#define EEPROM_SIZE		 				1984
		#define MAXNUMEXTRAFIELDS 		12
	#else
		#define EEPROM_SIZE 					1024
		#define MAXNUMEXTRAFIELDS 		6
	#endif

	#define MAGICBYTES    					"CFG" 
	#define MAGICEEP      					"%"
	#define MAGICBYTE     					85
	#define RTCMEMBEGIN   					68

	// length of config variables
	#define STRUCT_CHAR_ARRAY_SIZE  50    
	#define STRUCT_COMPDATE_SIZE    20
	#define STRUCT_BNAME_SIZE       30
	#define STRUCT_HOST_SIZE        24
	#define STRUCT_FILE_SIZE        31

	// constants used to define the status of the mode button based on the time it was pressed. (miliseconds)
	#define MODE_BUTTON_SHORT_PRESS       500
	#define MODE_BUTTON_LONG_PRESS        4000
	#define MODE_BUTTON_VERY_LONG_PRESS   10000

	
	
	
	/** 
		------ ------ ------ ------ ------ ------ library includes ------ ------ ------ ------ ------ ------
		Strange place for includes...
		But this makes it easier to edit DEFINES by users
	*/
	#ifdef ESP32
			#include <WiFi.h>
			#include <WiFiMulti.h>
			#include <ESPmDNS.h>
			#include <Preferences.h>
			#include <AsyncTCP.h>           	// https://github.com/me-no-dev/AsyncTCP
			#include <HTTPClient.h>
	#else
			#include <ESP8266WiFi.h>
			#include <ESP8266WiFiMulti.h>
			#include <ESP8266mDNS.h>
			extern "C" {
				#include "user_interface.h"			// used by the RTC memory read/write functions
			}
			#include <ESPAsyncTCP.h>        	// https://github.com/me-no-dev/ESPAsyncTCP
			#include <ESP8266HTTPClient.h>
	#endif
	
	#include <DNSServer.h> 
	#include <FS.h>
	#include <EEPROM.h>
	#include <ESPAsyncWebServer.h>    		// https://github.com/me-no-dev/ESPAsyncWebServer




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
	#ifndef ESP32
		typedef struct {
			byte markerFlag;
			int bootTimes;
			char boardMode = 'N';  									// Normal operation or Configuration mode?
		} rtcMemDef __attribute__((aligned(4)));
	#endif
	
	typedef struct  {
		const char *fieldLabel;
		char* (*varPointer);
		int length;
		char type;
	} eFields;
	
  typedef struct  {
    char ssid[3][STRUCT_CHAR_ARRAY_SIZE];			// 3x SSID
    char password[3][STRUCT_CHAR_ARRAY_SIZE];	// 3x PASS
		char deviceName[STRUCT_BNAME_SIZE];
    char compDate[STRUCT_COMPDATE_SIZE];			// saved compile date time
    char actCode[7];													// saved IotAppStory activation code
    
    char magicBytes[4];
  } strConfig;
  
	enum ModeButtonState {
			ModeButtonNoPress,           						// mode button is not pressed
			ModeButtonShortPress,        						// short press - will enter in firmware update mode
			ModeButtonLongPress,         						// long press - will enter in configuration mode
			ModeButtonVeryLongPress,     						// very long press - won't do anything (but the app developer might want to do something)
			ModeButtonFirmwareUpdate,    						// about to enter in firmware update mode
			ModeButtonConfigMode         						// about to enter in configuration mode
	};

  typedef std::function<void(void)> THandlerFunction;

	
	
	
  /** 
		------ ------ ------ ------ ------ ------ PROGMEM ------ ------ ------ ------ ------ ------
	*/
  const char SER_DEV[] PROGMEM          = "*-------------------------------------------------------------------------*";
   
  const char HOST2[] PROGMEM            = "iotappstory.com";
  const char FILE2[] PROGMEM            = "/ota/esp32-v1.php";
	
	const char HTTP_200[] PROGMEM         = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	const char HTTP_TEMP_START[] PROGMEM	= "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><meta name=\"theme-color\" content=\"#000\" />{h}<title>Config</title></head><body>";
	const char HTTP_TEMP_END[] PROGMEM		= "</body></html>";
	
	const char HTTP_STA_JS[] PROGMEM    	= "<script type=\"text/javascript\" src=\"https://iotappstory.com/ota/cfg-esp/cfg.js\"></script>";

	const char HTTP_AP_JS[] PROGMEM     	= "<script>var d=document;var p=d.getElementById(\"pop\");var l=d.getElementById(\"load\");var m=d.getElementById(\"mes\");var t=d.getElementById(\"text\");function c(l){d.getElementById('s').value=l.innerText||l.textContent;d.getElementById('p').focus()}function ld(u){var b=\"\";var x=new XMLHttpRequest();x.open(\"POST\",u,true);x.setRequestHeader(\"Content-type\",\"application/x-www-form-urlencoded\");p.classList.add('fade_in');l.classList.add('fade_in');var c=d.querySelectorAll(\"input,select\");for(i=0;i<c.length;i++){if(b!=\"\"){b+=\"&\"}b+=c[i].name+\"=\"+c[i].value}x.onreadystatechange=function(){if(this.readyState==4&&this.status==200){l.classList.remove('fade_in');if(this.responseText.includes(\"1:\")){var p=this.responseText.split(\":\");var a=\"http://\"+p[1];t.innerHTML+='Connected! Changed to STA mode. You will be redirected to '+a+' in <b id=\"cd\">22</b>s';cd(22);setTimeout(function(){window.location.href=a},22000)}else{t.innerHTML+=\"Failed to connect. Try again\"}m.classList.add('fade_in')}};x.send(b)}function cd(s){setTimeout(function(){s--;d.getElementById(\"cd\").innerHTML=s;cd(s)},1000)}function ep(){p.classList.remove('fade_in');m.classList.remove('fade_in');t.innerHTML=\"\"}</script>";
	const char HTTP_AP_CSS[] PROGMEM    	= "<style>#pop,body,html{height:100%;width:100%}#load,#mes,#pop,.fade_in{opacity:0}#cnt,body{position:relative}#pop,body,html,input{width:100%}body,body a,input{color:#FFF}#pop,.btn{text-align:center}.btn,body a:hover{text-decoration:none}.btn,.q{float:right}@keyframes fadeIn{to{z-index:10;opacity:1}}.fade_in{z-index:10;animation:fadeIn .5s ease-in 1 forwards}body,html{padding:0;margin:0;-webkit-box-sizing:border-box;-moz-box-sizing:border-box;box-sizing:border-box}*,:after,:before{-webkit-box-sizing:inherit;-moz-box-sizing:inherit;box-sizing:inherit}body{background:#000;font:16px sans-serif}#pop{padding-top:100px;position:fixed;z-index:0;background:rgba(0,0,0,.8)}#mes{width:200px;padding:10px;display:inline-block;background:#FFF;color:#000}#load{position:absolute;left:0;right:0;margin-left:auto;margin-right:auto;width:64px;height:64px}#load:after{content:" ";display:block;width:46px;height:46px;margin:1px;border-radius:50%;border:5px solid #fff;border-color:#fcbc12 transparent;animation:lds-dual-ring 1.2s linear infinite}@keyframes lds-dual-ring{0%{transform:rotate(0)}100%{transform:rotate(360deg)}}#cnt{min-width:280px;max-width:425px;margin:0 auto;padding:0 15px;z-index:2;border:1px solid #000}h1{font-size:3em}h2{font-size:2em}input{padding:4px 8px;margin:4px 0 10px;background:0;outline:0;border-radius:0;border:1px solid #ccc;font-size:14px}input:focus{border-color:#fcbc12}input:invalid{border-color:red}.btn{width:50%;margin:10px 0 0;padding:5px 14px 8px;display:block;background:#fcbc12;border-radius:4px;border:0;font-size:1.2em;color:#fff}.btn.sm{font-size:15px;padding:3px 14px 4px}.btn:active,.btn:focus,.btn:hover{background:#d3d3d3}.msg{background:#364048;margin:10px 0;padding:1.5em;border-radius:4px}.q{width:64px;text-align:right}.l{background:url('data:image/png; base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAl5JREFUeNrsV89rE0EUfm83odVaFKmWhlIPWlAUxFZb8SSieJEWeirqUXrxqH+C99JDjz3k4MGDYPVUCoInUWpOokJVEIRiaSldE8Qks89vNhPdbjbZ3XRLD2bIx87uzLz3zfs1ExYR2s+WafwEQuUNPN2otTbQCzDgAKpBjtUNDUcSEqhukaxcJylvQnQmbM0tYBoYAfrMN824ADwBXnhfVJH4+CTxuYWEBOASqWwDW0ECw8A8cCNEzjHgDHAHWAbug8AqqZJnnmQEsILZrin/R+Ai8BwYiOFWTfAV1k4wWSvJY0C8H/lCcxBYDFGuzf7F9E/63EG1ubJIVnYc+/neioAVY0dzQM73/gt4CJwFLhvo/gMzZiR35aj4aU6+PiIqfWgqnBvSsLJJ7usxBOGGdoEOtHe+0d/AJLDURN5NY60u+BGJVEFGOaPWhccFHrjdlgWmAu+zLZSTGZutB7MXQ5nsFHG2bReM+hMUyMdwWd7MDZORmMBRXx95SWsxCKyZuWEyEhPwj6udydG0SaAqWrshkMZBIbshoAKCnBgKnZ1KRbF9MEkl1FXQOoXetUD+9/hynVvs9oCZ+7ceyPqzGS6vv6Tc3c/BjGisA6WPJ9y3V9+IW+6PV6cimq4H1Z+6vP+wrhTGqef0t9YuKL4fw0GUjvJ6PbAP4VHth+xLkTEgnnWZ0m+s/cNxgnAvr0jSzmG0p61DoEOgQ2DfCYT881B2vR6mW4HEyI4gwN1Dq7poinJSNJBbK/CQHW2B3vMFHrw3TdVtXEjtbDoEVIUzh59q2dHX8v8tCP8IMADfe61URz1LDwAAAABJRU5ErkJggg==') left center no-repeat;background-size:1em}.table{width:100%}.table td{padding:.5em;text-align:left}.table tbody>:nth-child(2n-1){background:#364048}</style>";
	
	const char HTTP_APP_INFO[] PROGMEM		= "{\"l\":\"{l}\", \"v\":\"{v}\", \"n\":\"{n}\", \"m\":\"{m}\", \"t\":\"{t}\"}";
	
	const char HTTP_WIFI_SCAN[] PROGMEM		= "{\"s\":\"{s}\", \"q\":\"{q}\", \"e\":\"{e}\"}";
	const char HTTP_WIFI_FORM[] PROGMEM		= "<div id=\"pop\"><div id=\"mes\"><div id=\"text\"></div><button class=\"btn sm\" onclick=\"ep()\">Ok</button></div><div id=\"load\"></div></div><div id=\"cnt\"><label>SSID</label><input id=\"s\" name=\"s\" maxlength=50 placeholder=\"SSID\"><label>Password</label><input id=\"p\" name=\"p\" maxlength=50 placeholder=\"password\"><br><br><button class=\"btn\" onclick=\"ld('\wsa')\">Save</button></div>";

	const char HTTP_DEV_INFO[] PROGMEM		= "{\"s1\":\"{s1}\", \"s2\":\"{s2}\", \"s3\":\"{s3}\", \"fs\":\"{fs}\", \"ab\":\"{ab}\", \"ac\":\"{ac}\", \"mc\":\"{mc}\", \"xf\":\"{xf}\"}";
	
	
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
	
    class IOTAppStory {
    
        public:
            /** 
							------ ------ ------ ------ ------ ------ VARIABLES ------ ------ ------ ------ ------ ------
						*/
						
						#ifndef ESP32
							rtcMemDef rtcMem;
						#endif
						
						int bootTimes;
						char boardMode = 'N';   	// Normal operation or Configuration mode?
						
						
            strConfig config = {
								{"","",""},
                {"","",""},
                "yourESP",
                "",
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

						
            /** 
							------ ------ ------ ------ ------ ------ FUCNTION DEFINITIONS ------ ------ ------ ------ ------ ------ 
						*/
            IOTAppStory(const char* appName, const char* appVersion, const char *compDate, const int modeButton);
            
						// function for pre setting config parameters ssid & password, deviceName, automatic update, HOST1 and FILE1
            void preSetDeviceName(String deviceName);
						void preSetAutoUpdate(bool automaticUpdate);
						void preSetAutoConfig(bool automaticConfig);
						void preSetWifi(String ssid, String password);
						//void preSetServer(String HOST1, String FILE1);
            
						void setCallHome(bool callHome);
            void setCallHomeInterval(unsigned long interval);
						
						void begin(bool bootstats, char ea);
						
            ModeButtonState buttonLoop();

						void writeConfig(bool wifiSave = false);
						void readConfig();
						void espRestart(char mmode);
						void eraseFlash(int eepFrom, int eepTo);
						
						void connectNetwork();
						bool isNetworkConnected();
						
						void callHome(bool spiffs = true);
						bool iotUpdater(bool spiffs, bool loc = false);
						void addField(char* &defaultVal,const char *fieldLabel, int length, char type = 'L');
						void runConfigServer();
						int dPinConv(String orgVal);
            
            void onFirstBoot(THandlerFunction fn);												// called at the end of firstBoot
            //void onWifiConnected(THandlerFunction fn);									// called on succesfull Wifi connection SYSTEM_EVENT_STA_GOT_IP
            //void onWifiDisonnected(THandlerFunction fn);								// called when Wifi is dissconnected SYSTEM_EVENT_STA_DISCONNECTED

            void onModeButtonNoPress(THandlerFunction fn);								// called when state is changed to idle (mode button is not pressed)
            void onModeButtonShortPress(THandlerFunction fn);							// called when state is changed to short press
            void onModeButtonLongPress(THandlerFunction fn);							// called when state is changed to long press
            void onModeButtonVeryLongPress(THandlerFunction fn);					// called when state is changed to very long press

            void onFirmwareUpdateCheck(THandlerFunction fn);							// called when the app checks for firmware updates
            void onFirmwareUpdateDownload(THandlerFunction fn);						// called when firmware update
            void onFirmwareUpdateError(THandlerFunction fn);							// called when firmware update ends in an error

            void onConfigMode(THandlerFunction fn);												// called when the app is about to enter in configuration mode
						
						
						
						
        private:

						std::unique_ptr<DNSServer>        dnsServer;
						std::unique_ptr<AsyncWebServer> 	server;
						//std::unique_ptr<Preferences> 			preferences;
						
						#if WIFI_MULTI == true
							#ifdef ESP32
								WiFiMulti wifiMulti;
							#else
								ESP8266WiFiMulti wifiMulti;
							#endif
						#endif

            const char *_compDate;
            const int     _modeButton;
						int _nrXF 													= 0;											// nr of extra fields required in the config manager
						
						const char* _appName								= NULL;
						const char* _appVersion							= NULL;
						
						const char* _updateHost							= "iotappstory.com";			// ota update host
						#if defined  ESP8266
							char* _updateFile									= "/ota/esp8266-v2.php";	// file at host that handles 8266 updates
						#elif defined ESP32
							char* _updateFile									= "/ota/esp32-v1.php";		// file at host that handles 32 updates
						#endif
						
						
						bool _updateOnBoot									= true; 									// update on boot? (end of begin();)
						bool _automaticConfig								=	true;										// automaticly go to config on boot if there is no wifi connection present
				
						bool _setPreSet 										= false;									// ;) have there been any preSets set?
						bool _setDeviceName 								= false;									// is the device name set?
            bool _configReaded 									= false;									// has the config already been read?
            const static bool _boolDefaulValue 	= false;
						bool _callHome 											= false;
						
            unsigned long _lastCallHomeTime			= 0; 											//Time when we last called home
            unsigned long _callHomeInterval 		= 7200000;  							//Interval we want to call home at in milliseconds, default start at 2hrs
				
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
						
            THandlerFunction _configModeCallback;
						
						
						/** 
							------ ------ ------ ------ ------ ------ FUNCTION DEFINITIONS ------ ------ ------ ------ ------ ------
						*/
						void firstBoot(char ea);
						
            void readPref();
            void writePref();
            void printPref();
						
						
            //void readRTCmem();
            //void writeRTCmem();
            //void printRTCmem();
						void processField();
						void httpClientSetup(HTTPClient& http, bool httpSwitch, String url, bool spiffs=false);
						
						void servHdlRoot(AsyncWebServerRequest *request);
						void servHdlDevInfo(AsyncWebServerRequest *request);
						void servHdlWifiScan(AsyncWebServerRequest *request);
						void servHdlWifiSave(AsyncWebServerRequest *request);
						
						void servHdlAppInfo(AsyncWebServerRequest *request);
						void servHdlAppSave(AsyncWebServerRequest *request);
						
						void servHdlDevSave(AsyncWebServerRequest *request);
						
						void hdlReturn(AsyncWebServerRequest *request, String &retHtml, String type = "text/html");

						
            void updateLoop();
            bool isModeButtonPressed();
            ModeButtonState getModeButtonState();
    

						
						
						
            /** 
							------ ------ ------ ------ ------ ------ AUXILIARY FUNCTIONS ------ ------ ------ ------ ------ ------
						*/
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
