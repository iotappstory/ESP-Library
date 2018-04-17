/*
#include <ESP8266WiFi.h>
#include "ESP8266httpUpdateIasMod.h"
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <pgmspace.h>
#include <ArduinoJson.h>
#include <FS.h>
#include "IOTAppStory.h"
#include "INC_CONFIGagerMod.h"

#ifdef REMOTEDEBUGGING
	#include <WiFiUDP.h>
#endif
*/

#include "IOTAppStory.h"

#ifdef ESP32
	#include <WiFi.h>
	#include <ESPmDNS.h>
	#include <Preferences.h>
	#include <WiFiMulti.h>
	#include <AsyncTCP.h>           // https://github.com/me-no-dev/AsyncTCP
	#include <HTTPClient.h>
#else
	#include <ESP8266WiFi.h>
	#include <StreamString.h>
	#include <ESP8266mDNS.h>
	extern "C" {
		#include "user_interface.h"		// used by the RTC memory read/write functions
	}
	#include <ESP8266WiFiMulti.h>
	#include <ESPAsyncTCP.h>        // https://github.com/me-no-dev/ESPAsyncTCP
	#include <ESP8266HTTPClient.h>
#endif

#include "ESPhttpUpdateIasMod.h"
#include <DNSServer.h> 
#include <FS.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>    // https://github.com/me-no-dev/ESPAsyncWebServer






IOTAppStory::IOTAppStory(const char* appName, const char* appVersion, const char *compDate, const int modeButton)
: _appName(appName)
, _appVersion(appVersion)
, _compDate(compDate)
, _modeButton(modeButton)
{
	#if DEBUG_LVL >= 1
		Serial.begin(SERIAL_SPEED);
		while (!Serial){
			delay(10);
		}
		DEBUG_PRINT(F("\n\n\n\n\n"));
	#endif
}


/**
	THIS ONLY RUNS ON THE FIRST BOOT OF A JUST INSTALLED APP (OR AFTER RESET TO DEFAULT SETTINGS)
*/
void IOTAppStory::firstBoot(char ea){

	// erase eeprom after config (delete extra field data etc.)
	if(ea == 'F'){
		
		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(F(" Full erase of EEPROM"));
		#endif
		
		// Wipe out WiFi credentials.
		WiFi.disconnect();
		delay(200);
		
		// erase full eeprom
		eraseFlash(0,EEPROM_SIZE);
		

		String emty = F("000000");
		emty.toCharArray(config.actCode, 7);
		emty = "";
		emty.toCharArray(config.ssid[0], STRUCT_CHAR_ARRAY_SIZE);
		emty.toCharArray(config.password[0], STRUCT_PASSWORD_SIZE);
		emty.toCharArray(config.ssid[1], STRUCT_CHAR_ARRAY_SIZE);
		emty.toCharArray(config.password[1], STRUCT_PASSWORD_SIZE);
		emty.toCharArray(config.ssid[2], STRUCT_CHAR_ARRAY_SIZE);
		emty.toCharArray(config.password[2], STRUCT_PASSWORD_SIZE);

	}else if(ea == 'P'){
		
		#if DEBUG_LVL == 1
			DEBUG_PRINTLN(F(" Partial erase of EEPROM"));
		#endif
		
		#if DEBUG_LVL >= 2
			DEBUG_PRINTLN(F(" Partial erase of EEPROM but leaving config settings"));
		#endif
		
		// erase eeprom but leave the config settings
		eraseFlash((sizeof(config)+2),EEPROM_SIZE);
	}
	#if DEBUG_LVL >= 1
	else{
		DEBUG_PRINTLN(F(" Leave EEPROM intact"));
	#endif
	}
	
	boardMode = 'N';
	bootTimes = 0;
	writePref();
	
	// update first boot config flag (date)
	strcpy(config.compDate, _compDate);
	writeConfig();

	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN(FPSTR(SER_DEV));
	#endif
	
	if (_firstBootCallback){
		
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(F(" Run first boot callback"));
		#endif
		_firstBootCallback();
	}
}



void IOTAppStory::preSetDeviceName(String deviceName){
	if (!_configReaded) {
		readConfig();
	}
	_setDeviceName = true;
	SetConfigValueCharArray(config.deviceName, deviceName, STRUCT_BNAME_SIZE, _setPreSet);
}

void IOTAppStory::preSetAutoUpdate(bool automaticUpdate){
	_updateOnBoot = automaticUpdate;
}

void IOTAppStory::preSetAutoConfig(bool automaticConfig){
	_automaticConfig = automaticConfig;
}

void IOTAppStory::preSetWifi(String ssid, String password){
	if (!_configReaded) {
		readConfig();
	}

	_setPreSet = true;
	SetConfigValueCharArray(config.ssid[0], ssid, STRUCT_CHAR_ARRAY_SIZE, _setPreSet);
	SetConfigValueCharArray(config.password[0], password, STRUCT_PASSWORD_SIZE, _setPreSet);
}

/*
void IOTAppStory::preSetServer(String HOST1, String FILE1){
	if (!_configReaded) {
		readConfig();
	}
	SetConfigValueCharArray(config.HOST1, HOST1, STRUCT_CHAR_ARRAY_SIZE, _setPreSet);
	SetConfigValueCharArray(config.FILE1, FILE1, STRUCT_CHAR_ARRAY_SIZE, _setPreSet);
}
*/


void IOTAppStory::setCallHome(bool callHome) {
   _callHome = callHome;
}

void IOTAppStory::setCallHomeInterval(unsigned long interval) {
   _callHomeInterval = interval * 1000; //Convert to millis so users can pass seconds to this function
}

void IOTAppStory::begin(char ea){
	
	// if deviceName is not set, set it to the appName
	if(_setDeviceName == false){
		preSetDeviceName("yourESP");
	}
	
	// read config if needed
	if (!_configReaded) {
		readConfig();
	}
	
	
	// write config if detected changes
	if(_setPreSet == true){
		writeConfig();
		
		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(F("Saving config presets...\n"));
		#endif
	}

	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN(FPSTR(SER_DEV));
		
		DEBUG_PRINT(F(" Start "));
		DEBUG_PRINT(_appName);
		DEBUG_PRINT(F(" "));
		DEBUG_PRINTLN(_appVersion);
		/*
		String firmware = _appName;
		firmware += " ";
		firmware += _appVersion;
		DEBUG_PRINTF_P(PSTR(" Start %s\n"), firmware);
		*/
	#endif
	#if DEBUG_LVL >= 2
		DEBUG_PRINTLN(FPSTR(SER_DEV));
		DEBUG_PRINTF_P(PSTR(" Mode select button: GPIO%d\n Boardname: %s\n Update on boot: %d\n"), _modeButton, config.deviceName, _updateOnBoot);
		//DEBUG_PRINTF_P(PSTR(" Mode select button: GPIO%d\n"), _modeButton);
		//DEBUG_PRINTF_P(PSTR(" Boardname: %s\n"), config.deviceName);
		//DEBUG_PRINTF_P(PSTR(" Automatic update: %d\n"), _updateOnBoot);
	#endif
	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN(FPSTR(SER_DEV));
	#endif
	
	// set the input pin for Config/Update mode selection
	pinMode(_modeButton, INPUT_PULLUP);

	// Read the "bootTime" & "boardMode" from the Non-volatile storage on ESP32 processor
	readPref();
	
	// on first boot of the app run the firstBoot() function
	if(strcmp(config.compDate,_compDate) != 0){
		firstBoot(ea);
	}
	
	// BOOT STATISTICS read and increase boot statistics (optional)
	#if BOOTSTATISTICS == true && DEBUG_LVL >= 1
		bootTimes++;
		writePref();
		
		#if DEBUG_LVL >= 1
			printPref();
		#endif
	#endif

	// process added fields
	processField();
	
	
	// --------- START WIFI --------------------------
	connectNetwork();
	
	//---------- SELECT BOARD MODE -----------------------------
	#if INC_CONFIG == true
		if(boardMode == 'C'){
			runConfigServer();
		}
	#endif
	
	// --------- READ FULL CONFIG --------------------------
	//readConfig();
	




	// --------- if automaticUpdate Update --------------------------
	if(_updateOnBoot == true){
		callHome();
	}

	_buttonEntry = millis() + MODE_BUTTON_VERY_LONG_PRESS;    // make sure the timedifference during startup is bigger than 10 sec. Otherwise it will go either in config mode or calls home
	_appState = AppStateNoPress;

	// ----------- END SPECIFIC SETUP CODE ----------------------------
	#if DEBUG_LVL >= 1
		DEBUG_PRINT(F("\n\n\n\n\n"));
	#endif
/*	*/
}


/** read / get the stored preferences */
void IOTAppStory::readPref() {
	#ifdef ESP32
		Preferences preferences;
		
		// Open Preferences
		preferences.begin("pref", false);
		
		// Get the bootTimes value, if the key does not exist, return a default value of 0
		bootTimes	= preferences.getUInt("bootTimes", 0);
		
		// Get the boardMode value, if the key does not exist, return a default value of 'N'
		boardMode	= preferences.getUInt("boardMode", 'N');
		
		// Close the Preferences
		preferences.end();
		
	#else
		
		system_rtc_mem_read(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
		if (rtcMem.markerFlag != MAGICBYTE) {
			rtcMem.markerFlag = MAGICBYTE;
			rtcMem.bootTimes = 0;
			rtcMem.boardMode = 'N';
			system_rtc_mem_write(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
		}
		boardMode = rtcMem.boardMode;
		bootTimes = rtcMem.bootTimes;
	#endif
}


/** store preferences */
void IOTAppStory::writePref(){
	#ifdef ESP32
		Preferences preferences;
		
		// Open Preferences
		preferences.begin("pref", false);
		
		// Store the bootTimes value
		preferences.putUInt("bootTimes", bootTimes);
		
		// Store the boardMode value
		preferences.putUInt("boardMode", boardMode);
		
		// Close the Preferences
		preferences.end();
		
	#else
		rtcMem.boardMode = boardMode;
		rtcMem.bootTimes = bootTimes;
		
		rtcMem.markerFlag = MAGICBYTE;
		system_rtc_mem_write(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
	#endif
}


/** print preferences */
void IOTAppStory::printPref(){
	#ifdef ESP32
		DEBUG_PRINTF_P(PSTR(" bootTimes since last update: %d\n boardMode: %c\n"), bootTimes, boardMode);
	#else
		DEBUG_PRINTF_P(PSTR(" rtcMem\n bootTimes since powerup: %d\n boardMode: %c\n"), rtcMem.bootTimes, rtcMem.boardMode);
		//DEBUG_PRINTF_P(PSTR(" rtcMem\n markerFlag: %c\n"), rtcMem.markerFlag);
		//DEBUG_PRINTF_P(PSTR(" bootTimes since powerup: "));
		//DEBUG_PRINT(rtcMem.bootTimes);
		//DEBUG_PRINTF_P(PSTR("\n boardMode: %c\n"), rtcMem.boardMode);
	#endif
	DEBUG_PRINTLN(FPSTR(SER_DEV));
}


/** config server */
void IOTAppStory::runConfigServer() {
	
	bool exitConfig = false;

	// callback entered config mode
	if (_configModeCallback){
		_configModeCallback();
	}
	
	#if DEBUG_LVL >= 1
		DEBUG_PRINT(F("\n\n\n\n C O N F I G U R A T I O N   M O D E\n"));
	#endif
	
	if(WiFi.status() != WL_CONNECTED){
		
		// when there is no wifi setup server in AP mode
		IPAddress apIP(192, 168, 4, 1);
		dnsServer.reset(new DNSServer());
		server.reset(new AsyncWebServer(80));

		WiFi.mode(WIFI_AP_STA);
		#if SMARTCONFIG == true
			WiFi.beginSmartConfig();
		#endif
		WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
		WiFi.softAP(config.deviceName);
		
		dnsServer->start(DNS_PORT, "*", apIP);
		
		#if DEBUG_LVL >= 2
			DEBUG_PRINTF_P(PSTR(" AP mode. Connect to Wifi AP \"%s\"\n And open 192.168.4.1\n"), config.deviceName);
		#endif
		
	}else{
		
		// when there is wifi setup server in STA mode
		server.reset(new AsyncWebServer(80));
		WiFi.mode(WIFI_STA);
		
		#if DEBUG_LVL >= 2
			DEBUG_PRINT(F(" STA mode. Open "));
			DEBUG_PRINTLN(WiFi.localIP());
		#endif
		
	}

	
  server->on("/", HTTP_GET, [&](AsyncWebServerRequest *request){ servHdlRoot(request); });
  server->on("/i", HTTP_GET, [&](AsyncWebServerRequest *request){ servHdlDevInfo(request); });
  server->on("/wsc", HTTP_GET, [&](AsyncWebServerRequest *request){ servHdlWifiScan(request); });
  server->on("/wsa", HTTP_POST, [&](AsyncWebServerRequest *request){ servHdlWifiSave(request); });
  server->on("/app", HTTP_GET, [&](AsyncWebServerRequest *request){ servHdlAppInfo(request); });
  server->on("/as", HTTP_POST, [&](AsyncWebServerRequest *request){ servHdlAppSave(request); });
  
	server->on("/ds", HTTP_POST, [&](AsyncWebServerRequest *request){
		#if DEBUG_LVL >= 3
			DEBUG_PRINT(F("\n Received activation code from browser"));
		#endif
		
		String retHtml = F("0");
		
		if(request->hasParam("ac", true)){
			
			request->getParam("ac", true)->value().toCharArray(config.actCode, 7); //???
			
			if(!_confirmed && !_tryToConf){
				_tryToConf = true;
				retHtml = F("1");
			}
		}
		
		hdlReturn(request, retHtml);
	});
	
	server->on("/close", HTTP_GET, [&](AsyncWebServerRequest *request){ exitConfig = true; });
	
	// start the server
  server->begin();

	// config server loop
  while(exitConfig == false){
		
		yield();
		
		if(WiFi.status() != WL_CONNECTED){
			
			//DNS
			dnsServer->processNextRequest();
			
			// smartconfig default false / off
			#if SMARTCONFIG == true
				if(WiFi.smartConfigDone()){
					WiFi.mode(WIFI_AP_STA);
					isNetworkConnected();
				}
			#endif
			
			// wifi connect when asked
			if(_tryToConn == true){
				#if DEBUG_LVL >= 3
					DEBUG_PRINT(F("Received cred: "));
					DEBUG_PRINT(config.ssid[0]);
					DEBUG_PRINT(F(" - "));
					DEBUG_PRINTLN(config.password[0]);
				#endif
				
				#if WIFI_MULTI == true
					wifiMulti.addAP(config.ssid[0], config.password[0]);
					///wifiMulti.run();
				#else
					WiFi.begin(config.ssid[0], config.password[0]);
				#endif
					
				_connected = isNetworkConnected();
				yield();
				
				if(_connected){
					
					// Saving config to eeprom
					_writeConfig = true;
					
					#if DEBUG_LVL >= 3
						DEBUG_PRINT(F(" Connected. Saving config to eeprom"));
					#endif
				}else{
					readConfig();
					DEBUG_PRINTLN(F("\n Failed, try again!!!"));					// <---------- temp...remove...
					_tryToConnFail = true;
				}
				
				_tryToConn = false;
			}
			
		}else{
			
			// write actCode to EEPROM
			if(!_confirmed && _tryToConf){
				_confirmed = true;
				_tryToConf = false;
				_writeConfig = true;
			}			
			
			// write EEPROM
			if(_writeConfig){
				writeConfig();
				yield();
				_writeConfig = false;
			}
			
			// when succesfully added wifi cred in AP mode change to STA mode
			if(_changeMode){
				delay(1000);
				WiFi.mode(WIFI_STA);
				delay(100);
				_changeMode = false;

				#if DEBUG_LVL >= 2
					//DEBUG_PRINTF_P(PSTR(" \n Changed to STA mode. Open %s\n"), WiFi.localIP().toString());
					
					DEBUG_PRINT(F("\n Changed to STA mode. Open "));
					DEBUG_PRINTLN(WiFi.localIP());
					DEBUG_PRINTLN();
				#endif
			}
			
		}
  }
	
	#if DEBUG_LVL >= 2
		DEBUG_PRINTLN(F(" Exit config"));
	#endif
	
	// Return to Normal Operation
	espRestart('N');
}


/** Connect to Wifi AP */
void IOTAppStory::connectNetwork() {
	
	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN(F(" Connecting to WiFi AP"));
	#endif
	
	#if WIFI_MULTI == true
    wifiMulti.addAP(config.ssid[0], config.password[0]);
    wifiMulti.addAP(config.ssid[1], config.password[1]);
    wifiMulti.addAP(config.ssid[2], config.password[2]);
	#else
		WiFi.begin(config.ssid[0], config.password[0]);
	#endif
	

	
	//if(WiFi.status() != WL_CONNECTED) {
	if(!isNetworkConnected()) {
		

			if(_automaticConfig || boardMode == 'C'){
				
				if(boardMode == 'N'){
					boardMode = 'C';
					writePref();
				}
				
				#if DEBUG_LVL >= 1
					DEBUG_PRINT(F("\n No Connection. Going into Configuration Mode\n"));
				#endif				
				
			}else{
				
				#if DEBUG_LVL >= 1
					// this point is only reached if _automaticConfig = false
					DEBUG_PRINT(F("\n WiFi NOT connected. Continuing anyway\n"));
				#endif				
			}
		
	}else{
		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(F("\n WiFi connected\n"));
		#endif
		
		#if DEBUG_LVL >= 2
			DEBUG_PRINT(F(" Device MAC: "));
			DEBUG_PRINTLN(WiFi.macAddress());
		#endif
		
		#if DEBUG_LVL >= 1
			DEBUG_PRINT(F(" Device IP Address: "));
			DEBUG_PRINTLN(WiFi.localIP());
		#endif
		
		#if USEMDNS == true
			// Register host name in WiFi and mDNS
			String hostNameWifi = config.deviceName;
			hostNameWifi += ".local";
			
			// wifi_station_set_hostname(config.deviceName);
			// WiFi.hostname(hostNameWifi);
			
			if(MDNS.begin(config.deviceName)){
				
				#if DEBUG_LVL >= 1
					DEBUG_PRINT(F(" MDNS responder started: http://"));
					DEBUG_PRINT(hostNameWifi);
				#endif
				
				#if DEBUG_LVL >= 2
					DEBUG_PRINTLN(F("\n\n To use mDNS Install host software:\n - For Linux, install Avahi (http://avahi.org/)\n - For Windows, install Bonjour (https://commaster.net/content/how-resolve-multicast-dns-windows)\n - For Mac OSX and iOS support is built in through Bonjour already"));
				#else
					DEBUG_PRINTLN(F(""));
				#endif
				
			}else{
				#if DEBUG_LVL >= 1
					DEBUG_PRINTLN(F(" MDNS responder failed to start"));
				#endif
			}
		#endif
	}
	

	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN(FPSTR(SER_DEV));
	#endif
}


/**
	Wait until network is connected. 
	Returns false if not connected after MAX_WIFI_RETRIES retries 
*/
bool IOTAppStory::isNetworkConnected() {
	#if defined  ESP8266
		int retries = MAX_WIFI_RETRIES;
	#elif defined ESP32
		int retries = (MAX_WIFI_RETRIES/2);
	#endif
	
	DEBUG_PRINT(" ");
	
#if WIFI_MULTI == true
	while (wifiMulti.run() != WL_CONNECTED && retries-- > 0 ) {
#else
	while (WiFi.status() != WL_CONNECTED && retries-- > 0 ) {
#endif
		
		delay(500);
		#if DEBUG_LVL >= 1
			DEBUG_PRINT(F("."));
		#endif
	}
	
	if(retries > 0){
		return true;
	}else{
		return false;
	}
}


/** 
	call home and check for updates
*/
void IOTAppStory::callHome(bool spiffs /*= true*/) {
	/**/
	// update from IOTappStory.com
	bool updateHappened = false;
	
	#if DEBUG_LVL >= 2
		DEBUG_PRINTLN(F(" Calling Home"));
		//DEBUG_PRINT(PSTR(" Calling Home\n Current App: "));
		//DEBUG_PRINT(_appName);
		//DEBUG_PRINT(F(" "));
		//DEBUG_PRINTLN(_appVersion);
	#endif

	if (_firmwareUpdateCheckCallback){
		_firmwareUpdateCheckCallback();
	}
	
	// try to update from address 1
	updateHappened = iotUpdater(0,0);
	
	// if address 1 was unsuccesfull try address 2
	if (updateHappened == false) {
		updateHappened = iotUpdater(0,1);
	}
	

	//#if defined ESP8266
		if (spiffs) {
			
			// try to update spiffs from address 1
			updateHappened = iotUpdater(1,0);
			
			// if address 1 was unsuccesfull try address 2
			if (updateHappened == false) {
				updateHappened = iotUpdater(1,1);
			}
		}
	//#elif defined ESP32
	//	#if DEBUG_LVL >= 2
	//		DEBUG_PRINTLN(F("\n No OTA SPIFFS support for ESP32 yet!"));
	//	#endif
	//#endif

	#if DEBUG_LVL >= 2
		DEBUG_PRINTLN(F("\n Returning from IOTAppStory.com"));
	#endif
	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN(FPSTR(SER_DEV));
	#endif
	
}


/** 
	IOT updater
*/
bool IOTAppStory::iotUpdater(bool spiffs, bool loc) {
	String url = "";
	bool httpSwitch = false;
	
	if(HTTPS == true && system_get_free_heap_size() > HEAPFORHTTPS){
		httpSwitch = true;
	}

	#if defined ESP32 && HTTPS == true
		httpSwitch = false;	// Overide. For HTTPS 443. As of today, HTTPS doesn't work for ota updates on the esp32.
		#if DEBUG_LVL >= 2
			DEBUG_PRINT(F("\n No HTTPS OTA support for ESP32 yet!"));
		#endif
	#endif
	
	#if DEBUG_LVL == 1
		DEBUG_PRINT(F(" Checking for "));
	#endif
	#if DEBUG_LVL >= 2
		DEBUG_PRINT(F("\n Checking for "));
	#endif
	#if DEBUG_LVL >= 1
		if(spiffs == false){
			// type = sketch
			DEBUG_PRINT(F("App(Sketch)"));
		}
		if(spiffs == true){
			// type = spiffs
			DEBUG_PRINT(F("SPIFFS"));
		}
	#endif

	#if DEBUG_LVL >= 2
		DEBUG_PRINT(F(" updates from: "));
	#else
		DEBUG_PRINT(F(" updates"));
	#endif

	if(httpSwitch == true){
		url = F("https://");
	}else{
		url = F("http://");
	}

	if(loc == 0){
		// location 1
		url += _updateHost;
		url += _updateFile;
	}else{
		// location 2
		url += FPSTR(HOST2);
		url += FPSTR(FILE2);
	}
	
	#if DEBUG_LVL >= 2
		DEBUG_PRINTLN(url);
	#else
		DEBUG_PRINTLN("");		
	#endif

	HTTPClient http;
	httpClientSetup(http, httpSwitch, url, spiffs);

	// track these headers for later use
	const char * headerkeys[] = { "x-MD5", "x-name" };
	size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
	http.collectHeaders(headerkeys, headerkeyssize);

	int code = http.GET();
	int len = http.getSize();
	
	if(code == HTTP_CODE_OK){
		
		ESP8266HTTPUpdate ESPhttpUpdate;
		ESPhttpUpdate.rebootOnUpdate(true);
		
		#if DEBUG_LVL >= 1
			DEBUG_PRINT(F(" Download & Process update..."));
		#endif
		
		if (_firmwareUpdateDownloadCallback){
			_firmwareUpdateDownloadCallback();
		}

		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN("[httpUpdate] Header read fin.\n");
			DEBUG_PRINTLN("[httpUpdate] Server header:\n");
			DEBUG_PRINTF_P("[httpUpdate]  - code: %d\n", code);
			DEBUG_PRINTF_P("[httpUpdate]  - len: %d\n", len);

			if(http.hasHeader("x-MD5")) {
				DEBUG_PRINTF_P("[httpUpdate]  - MD5: %s\n", http.header("x-MD5").c_str());
			}
			
			DEBUG_PRINTLN("[httpUpdate] ESP8266 info:\n");
			#if defined ESP32

			#elif defined ESP8266
				DEBUG_PRINTF_P("[httpUpdate]  - free Space: %d\n", ESP.getFreeSketchSpace());
				DEBUG_PRINTF_P("[httpUpdate]  - current Sketch Size: %d\n", ESP.getSketchSize());
			#endif
			DEBUG_PRINTF_P("[httpUpdate]  - current version: %s\n", _appVersion);
			
		#endif

		ESPhttpUpdate.handleUpdate(http, len, spiffs);
		return true;

	}else{
		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(http.getString());
		#endif
		
		#if defined ESP32
			if(code == 399){
				return true;
			}else{
				if (_firmwareUpdateErrorCallback){
					_firmwareUpdateErrorCallback();
				}
				return false;
			}

		#elif defined ESP8266
			if(code == HTTP_CODE_NOT_MODIFIED){
				return true;
			}else{
				if (_firmwareUpdateErrorCallback){
					_firmwareUpdateErrorCallback();
				}
				return false;
			}
		#endif
		
	}
	http.end();
}


/** 
	Add fields to the fieldStruct
*/
void IOTAppStory::addField(char* &defaultVal,const char *fieldLabel, int length, char type){
	_nrXF++;
	
	// add values to the fieldstruct
	fieldStruct[_nrXF-1].fieldLabel = fieldLabel;
	fieldStruct[_nrXF-1].varPointer = &defaultVal;
	fieldStruct[_nrXF-1].length = length+1;
	fieldStruct[_nrXF-1].type = type;
}


/** 
	Process the added fields
	- save to eeprom
	- load stored values
*/
void IOTAppStory::processField(){

	// to prevent longer then default values overwriting each other
	// temp save value, overwrite variable with longest value posible
	// and then resave the temp value to the original variable
	if(_nrXF > 0){
		char* tempValue[_nrXF];
		
		for(unsigned int nr = 0; nr < _nrXF; nr++){
			tempValue[nr] = (*fieldStruct[nr].varPointer);
			
			char* tmpVal = new char[fieldStruct[nr].length];
			for (int i = 0; i < fieldStruct[nr].length-1; i++) {
				tmpVal[i] = 't';
			}
			
			(*fieldStruct[nr].varPointer) = tmpVal;
		}
		
		for(unsigned int nr = 0; nr < _nrXF; nr++){
			//String().toCharArray(, fieldStruct[nr].length);
			
			strcpy((*fieldStruct[nr].varPointer), tempValue[nr]);
		}
	}
		
	if(_nrXF > 0){
		#if DEBUG_LVL == 1
			DEBUG_PRINTLN(F(" Processing added fields"));
		#endif
		#if DEBUG_LVL >= 2
			DEBUG_PRINTLN(F(" Processing added fields\n ID | LABEL                          | LEN |  EEPROM LOC  | DEFAULT VALUE                  | CURRENT VALUE                  | STATUS\n"));
		#endif
		
		EEPROM.begin(EEPROM_SIZE);
		
		for (unsigned int nr = 1; nr <= _nrXF; nr++){
			delay(100);
			// loop through the fields struct array
			
			int prevTotLength = 0;
			for(unsigned int i = 0; i < (nr-1); i++){
				prevTotLength += fieldStruct[i].length;
			}
			const int sizeOfVal = fieldStruct[nr-1].length;
			const int sizeOfConfig = sizeof(config)+2;
			const int eeBeg = sizeOfConfig+prevTotLength+nr+((nr-1)*2);
			const int eeEnd = sizeOfConfig+(prevTotLength+sizeOfVal)+nr+1+((nr-1)*2);

			#if DEBUG_LVL >= 2
				DEBUG_PRINTF_P(PSTR(" %02d | %-30s | %03d | %04d to %04d | %-30s | "), nr, fieldStruct[nr-1].fieldLabel, fieldStruct[nr-1].length-1, eeBeg, eeEnd, (*fieldStruct[nr-1].varPointer));
			#endif
			
			char* eepVal = new char[fieldStruct[nr-1].length + 1];
			char* tmpVal = new char[fieldStruct[nr-1].length + 1];
			for (int i = 0; i < fieldStruct[nr-1].length; i++) {
				eepVal[i] = 0;
				tmpVal[i] = 0;
			}
			if ((*fieldStruct[nr-1].varPointer) != NULL) {
				strncpy(tmpVal, (*fieldStruct[nr-1].varPointer), fieldStruct[nr-1].length);
			}

			// read eeprom, check for MAGICEEP and get the updated value if present
			if(EEPROM.read(eeBeg) == MAGICEEP[0] && EEPROM.read(eeEnd) == '^'){

				for (unsigned int t = eeBeg; t <= eeEnd; t++){
					// start after MAGICEEP
					if(t != eeBeg && t != eeEnd && EEPROM.read(t) != 0){
						*((char*)eepVal + (t-eeBeg)-1) = EEPROM.read(t);
					}
				}
				
				// if eeprom value is different update the ret value
				if(strcmp(eepVal, (*fieldStruct[nr-1].varPointer)) != 0){
					#if DEBUG_LVL >= 2
						DEBUG_PRINTF_P(PSTR("%-30s | OVERWRITTEN"), eepVal);
					#endif
					
					(*fieldStruct[nr-1].varPointer) = eepVal;
				}else{
					#if DEBUG_LVL >= 2
						DEBUG_PRINTF_P(PSTR("%-30s | DEFAULT"), (*fieldStruct[nr-1].varPointer));
					#endif
				}

			}else{
				#if DEBUG_LVL >= 2
					DEBUG_PRINTF_P(PSTR("%-30s | WRITTING TO EEPROM"), (*fieldStruct[nr-1].varPointer));
				#endif

				// add MAGICEEP to value and write to eeprom
				for (unsigned int t = eeBeg; t <= eeEnd; t++){
					if(t == eeBeg){
						EEPROM.put(t, MAGICEEP[0]);						// magic begin marker
					}else  if(t == eeEnd){
						EEPROM.put(t, '^');								// magic end marker
					}else{
						EEPROM.put(t, *((char*)tmpVal + (t-eeBeg)-1));	// byte of value`
					}
				}
			}
		
			#if DEBUG_LVL >= 2
				DEBUG_PRINTLN();
			#endif
		}
		EEPROM.end();
		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(FPSTR(SER_DEV));
		#endif
	}
}


/** 
	convert dpins to int
*/
int IOTAppStory::dPinConv(String orgVal){
	#if defined ESP8266_OAK

		// DEBUG_PRINTLN("- Digistump OAK -");
		if      (orgVal == "P1"  || orgVal == "5")    return P1;
		else if (orgVal == "P2"  || orgVal == "0")    return P2;
		else if (orgVal == "P3"  || orgVal == "3")    return P3;
		else if (orgVal == "P4"  || orgVal == "1")    return P4;
		else if (orgVal == "P5"  || orgVal == "4")    return P5;
		else if (orgVal == "P6"  || orgVal == "15")   return P6;
		else if (orgVal == "P7"  || orgVal == "13")   return P7;
		else if (orgVal == "P8"  || orgVal == "12")   return P8;
		else if (orgVal == "P9"  || orgVal == "14")   return P9;
		else if (orgVal == "P10" || orgVal == "16")   return P10;
		else if (orgVal == "P11" || orgVal == "17")   return P11;
		else                                          return P1;

	#elif defined ESP8266_WEMOS_D1MINI || defined ESP8266_WEMOS_D1MINILITE || defined ESP8266_WEMOS_D1MINIPRO || defined ESP8266_NODEMCU || defined WIFINFO

		// DEBUG_PRINTLN("- Special ESP's -");
		if      (orgVal == "D0"  || orgVal == "16")   return D0;
		else if (orgVal == "D1"  || orgVal == "5")    return D1;
		else if (orgVal == "D2"  || orgVal == "4")    return D2;
		else if (orgVal == "D3"  || orgVal == "0")    return D3;
		else if (orgVal == "D4"  || orgVal == "2")    return D4;
		else if (orgVal == "D5"  || orgVal == "14")   return D5;
		else if (orgVal == "D6"  || orgVal == "12")   return D6;
		else if (orgVal == "D7"  || orgVal == "13")   return D7;
		else if (orgVal == "D8"  || orgVal == "15")   return D8;
		else if (orgVal == "D9"  || orgVal == "3")    return D9;
		else if (orgVal == "D10" || orgVal == "1")    return D10;
		else                                          return D0;
		
	#else

		// DEBUG_PRINTLN("- Generic ESP's -");
		
		// There are NO constants for the generic eps's!
		// But people makes mistakes when entering pin nr's in config
		// And if you originally developed your code for "Special ESP's"
		// this part makes makes it compatible when compiling for "Generic ESP's"
		
		if      (orgVal == "D0"  || orgVal == "16")   return 16;
		else if (orgVal == "D1"  || orgVal == "5")    return 5;
		else if (orgVal == "D2"  || orgVal == "4")    return 4;
		else if (orgVal == "D3"  || orgVal == "0")    return 0;
		else if (orgVal == "D4"  || orgVal == "2")    return 2;
		else if (orgVal == "D5"  || orgVal == "14")   return 14;
		else if (orgVal == "D6"  || orgVal == "12")   return 12;
		else if (orgVal == "D7"  || orgVal == "13")   return 13;
		else if (orgVal == "D8"  || orgVal == "15")   return 15;
		else if (orgVal == "D9"  || orgVal == "3")    return 3;
		else if (orgVal == "D10" || orgVal == "1")    return 1;
		else                                          return 16;

	#endif
}


/** 
	Set mode and reboot
*/
void IOTAppStory::espRestart(char mmode) {
	//while (isModeButtonPressed()) yield();    // wait till GPIOo released
	delay(500);
	
	boardMode = mmode;
	writePref();

	ESP.restart();
}



/** 
	Erase flash from till
*/
void IOTAppStory::eraseFlash(int eepFrom, int eepTo) {
	#if DEBUG_LVL >= 2
		DEBUG_PRINTF_P(PSTR(" Erasing Flash...\n From %4d to %4d\n"), eepFrom, eepTo);
	#endif
	
	EEPROM.begin(EEPROM_SIZE);
	for (int t = eepFrom; t < eepTo; t++) EEPROM.write(t, 0);
	EEPROM.end();
}



/** 
	Write the config struct to EEPROM
	If saveXF == true also write the added fields. If their are any.
*/
void IOTAppStory::writeConfig(bool saveXF) {
	EEPROM.begin(EEPROM_SIZE);
	
	// WRITE CONFIG TO EEPROM
	for (int t = 0; t < sizeof(config); t++) {
		EEPROM.write(t, *((char*)&config + t));
		
		#if DEBUG_EEPROM_CONFIG
			// DEBUG (show all config EEPROM slots in one line)
			DEBUG_PRINT(GetCharToDisplayInDebug(*((char*)&config + t)));
		#endif
		
	}
	EEPROM.commit();
	
	#if DEBUG_EEPROM_CONFIG
		DEBUG_PRINTLN();
	#endif
	
	
	if(saveXF == true && _nrXF > 0){
		// LOOP THROUGH ALL THE ADDED FIELDS, CHECK VALUES AND IF NECESSARY WRITE TO EEPROM
		for (int nr = 1; nr <= _nrXF; nr++){
			
			int prevTotLength = 0;
			for(int i = 0; i < (nr-1); i++){
				prevTotLength += fieldStruct[i].length;
			}
			const int sizeOfVal = fieldStruct[nr-1].length;
			const int sizeOfConfig = sizeof(config)+2;
			const int eeBeg = sizeOfConfig+prevTotLength+nr+((nr-1)*2);
			const int eeEnd = sizeOfConfig+(prevTotLength+sizeOfVal)+nr+1+((nr-1)*2);
			
			#if DEBUG_EEPROM_CONFIG
				DEBUG_PRINT(F(" EEPROM space: "));
				DEBUG_PRINT(eeBeg);
				DEBUG_PRINT(F("  to "));
				DEBUG_PRINTLN(eeEnd);
				DEBUG_PRINT(F("  Size: "));
				DEBUG_PRINTLN(sizeOfVal);
				DEBUG_PRINTLN((*fieldStruct[nr-1].varPointer));
			#endif
			
			char* tmpVal = new char[sizeOfVal + 1];
			for (int i = 0; i <= sizeOfVal; i++) {
				tmpVal[i] = 0;
			}
			if ((*fieldStruct[nr-1].varPointer) != NULL) {
				strncpy(tmpVal, (*fieldStruct[nr-1].varPointer), fieldStruct[nr-1].length);
			}
			
			// check for MAGICEEP
			if(EEPROM.read(eeBeg) == MAGICEEP[0] && EEPROM.read(eeEnd) == '^'){
				// add MAGICEEP to value and write to eeprom
				for (int t = eeBeg; t <= eeEnd; t++){
					char valueTowrite;
					
					if(t == eeBeg){
						valueTowrite = MAGICEEP[0];
					}else if(t == eeEnd){
						valueTowrite = '^';
					}else{
						valueTowrite = *((char*)tmpVal + (t-eeBeg)-1);
					}
					EEPROM.put(t, valueTowrite);
					
					#if DEBUG_EEPROM_CONFIG
						// DEBUG (show all wifiSave EEPROM slots in one line)
						DEBUG_PRINT(GetCharToDisplayInDebug(valueTowrite));
					#endif
		
				}
			}
			EEPROM.commit();
		}
	}
	
	EEPROM.end();
}



/** 
	Read the config struct to EEPROM
	If saveXF == true also write the added fields. If their are any.
*/
void IOTAppStory::readConfig() {

	EEPROM.begin(EEPROM_SIZE);
	// Magic bytes at the end of the structure
	int magicBytesBegin = sizeof(config) - 4;
	
	if(EEPROM.read(magicBytesBegin) == MAGICBYTES[0] && EEPROM.read(magicBytesBegin + 1) == MAGICBYTES[1] && EEPROM.read(magicBytesBegin + 2) == MAGICBYTES[2]) {
			
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(F(" EEPROM Configuration found"));
		#endif
		
		for(int t = 0; t < sizeof(config); t++){
			char valueReaded = EEPROM.read(t);
			*((char*)&config + t) = valueReaded;
			
			#if DEBUG_EEPROM_CONFIG
				// DEBUG (show all config EEPROM slots in one line)
				DEBUG_PRINT(GetCharToDisplayInDebug(valueReaded));
			#endif
		}
		EEPROM.end();
		
		#if DEBUG_EEPROM_CONFIG
			DEBUG_PRINTLN();
		#endif

	}else{
		
		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(F(" EEPROM Configuration NOT FOUND!!!!"));
		#endif
		
		writeConfig();
	}
	
	_configReaded = true;
}


void IOTAppStory::updateLoop() {
   if (_callHome && millis() - _lastCallHomeTime > _callHomeInterval) {
      this->callHome();
      _lastCallHomeTime = millis();
   }
}


ModeButtonState IOTAppStory::buttonLoop() {
   this->updateLoop();
   return getModeButtonState();
}


bool IOTAppStory::isModeButtonPressed() {
	return digitalRead(_modeButton) == LOW; // LOW means flash button IS pressed
}


ModeButtonState IOTAppStory::getModeButtonState() {
	
	while(true)
	{
		unsigned long buttonTime = millis() - _buttonEntry;
	
		switch(_appState) {
		case AppStateNoPress:
			if (isModeButtonPressed()) {
				_buttonEntry = millis();
				_appState = AppStateWaitPress;
				continue;
			}
			return ModeButtonNoPress;
		
		case AppStateWaitPress:
			if (buttonTime > MODE_BUTTON_SHORT_PRESS) {
				_appState = AppStateShortPress;
				if (_shortPressCallback)
					_shortPressCallback();
				continue;
			}
			if (!isModeButtonPressed()) {
				_appState = AppStateNoPress;
			}
			return ModeButtonNoPress;
		
		case AppStateShortPress:
			if (buttonTime > MODE_BUTTON_LONG_PRESS) {
				_appState = AppStateLongPress;
				if (_longPressCallback)
					_longPressCallback();
				continue;
			}
			if (!isModeButtonPressed()) {
				_appState = AppStateFirmwareUpdate;
				continue;
			}
			return ModeButtonShortPress;

		case AppStateLongPress:
			if (buttonTime > MODE_BUTTON_VERY_LONG_PRESS) {
				_appState = AppStateVeryLongPress;
				if (_veryLongPressCallback)
					_veryLongPressCallback();
				continue;
			}
#if INC_CONFIG == true
			if (!isModeButtonPressed()) {
				_appState = AppStateConfigMode;
				continue;
			}
#endif
			return ModeButtonLongPress;
	
		case AppStateVeryLongPress:
			if (!isModeButtonPressed()) {
				_appState = AppStateNoPress;
				if (_noPressCallback)
					_noPressCallback();
				continue;
			}
			return ModeButtonVeryLongPress;
		
		case AppStateFirmwareUpdate:
			_appState = AppStateNoPress;
			//DEBUG_PRINTLN("Calling Home");
			callHome();
			continue;
#if INC_CONFIG == true	
		case AppStateConfigMode:
			_appState = AppStateNoPress;
			DEBUG_PRINTLN(F(" Entering in Configuration Mode"));
			espRestart('C');
			continue;
#endif
		}
	}
	return ModeButtonNoPress; // will never reach here (used just to avoid compiler warnings)
}



/** 
	callBacks
*/
void IOTAppStory::onFirstBoot(THandlerFunction value) {
	_firstBootCallback = value;
}

void IOTAppStory::onModeButtonNoPress(THandlerFunction value) {
	_noPressCallback = value;
}
void IOTAppStory::onModeButtonShortPress(THandlerFunction value) {
	_shortPressCallback = value;
}
void IOTAppStory::onModeButtonLongPress(THandlerFunction value) {
	_longPressCallback = value;
}
void IOTAppStory::onModeButtonVeryLongPress(THandlerFunction value) {
	_veryLongPressCallback = value;
}

void IOTAppStory::onFirmwareUpdateCheck(THandlerFunction value) {
	_firmwareUpdateCheckCallback = value;
}
void IOTAppStory::onFirmwareUpdateDownload(THandlerFunction value) {
	_firmwareUpdateDownloadCallback = value;
}
void IOTAppStory::onFirmwareUpdateError(THandlerFunction value) {
	_firmwareUpdateErrorCallback = value;
}

void IOTAppStory::onConfigMode(THandlerFunction value) {
	_configModeCallback = value;
}



/** Handle root */
void IOTAppStory::servHdlRoot(AsyncWebServerRequest *request) {
    
    String retHtml;
		retHtml += FPSTR(HTTP_TEMP_START);
		
		if (WiFi.status() == WL_CONNECTED) {	
		
				retHtml.replace("{h}", FPSTR(HTTP_STA_JS));

		}else{
			
				retHtml.replace("{h}", FPSTR(HTTP_AP_CSS));
				retHtml += FPSTR(HTTP_WIFI_FORM);
				retHtml.replace("{r}", strWifiScan());
				retHtml += FPSTR(HTTP_AP_JS);
		}
		
		retHtml += FPSTR(HTTP_TEMP_END);
		
    hdlReturn(request, retHtml);
}



/** Handle device information */
void IOTAppStory::servHdlDevInfo(AsyncWebServerRequest *request){
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(F(" Serving device info"));
		#endif
		
    String retHtml;
		retHtml += FPSTR(HTTP_DEV_INFO);
		retHtml.replace(F("{s1}"), config.ssid[0]);
		retHtml.replace(F("{s2}"), config.ssid[1]);
		retHtml.replace(F("{s3}"), config.ssid[2]);

		#if defined  ESP8266
			retHtml.replace(F("{cid}"), String(ESP.getChipId()));
			retHtml.replace(F("{fid}"), String(ESP.getFlashChipId()));
			retHtml.replace(F("{fss}"), String(ESP.getFreeSketchSpace()));
			retHtml.replace(F("{ss}"), String(ESP.getSketchSize()));
		#elif defined ESP32
			retHtml.replace(F("{cid}"), "");				// not available yet
			retHtml.replace(F("{fid}"), "");				// not available yet
		#endif
		
		retHtml.replace(F("{fs}"), String(ESP.getFlashChipSize()));
		retHtml.replace(F("{ab}"), ARDUINO_BOARD);
		retHtml.replace(F("{mc}"), WiFi.macAddress());
		retHtml.replace(F("{xf}"), String(_nrXF));
		
		if(String(config.actCode) == "000000" || String(config.actCode) == ""){
			retHtml.replace(F("{ac}"), "0");	
		}else{
			retHtml.replace(F("{ac}"), "1");	
		}
		
    hdlReturn(request, retHtml, F("text/json"));
}



/** Handle wifi scan */
String IOTAppStory::strWifiScan(){

    // WiFi.scanNetworks will return the number of networks found	
		String retHtml;
		int n = WiFi.scanComplete();
		if(n == -2){
			
			WiFi.scanNetworks(true);
			
		}else if(n){
			
			for (int i = 0; i < n; ++i) {
					
					// return html results from the wifi scan
					retHtml += FPSTR(HTTP_WIFI_SCAN);
					retHtml.replace(F("{s}"), WiFi.SSID(i));
					retHtml.replace(F("{q}"), String(WiFi.RSSI(i)));
					retHtml.replace(F("{e}"), String(WiFi.encryptionType(i)));             
					delay(10);
			}
			
			WiFi.scanDelete();
			if(WiFi.scanComplete() == -2){
				WiFi.scanNetworks(true);
			}
		}
	return retHtml;
}

void IOTAppStory::servHdlWifiScan(AsyncWebServerRequest *request){
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(F(" Serving results of Wifiscan"));
		#endif
		
		String retHtml = strWifiScan();

    hdlReturn(request, retHtml, F("text/json"));
		retHtml = String();
}



/** Handle save wifi credentials */
void IOTAppStory::servHdlWifiSave(AsyncWebServerRequest *request) {

		String retHtml = F("0");
		
		// are both ssid & password sent
		if(request->hasParam("s", true) && request->hasParam("p", true)){
			
			if(!request->hasParam("i", true)){
				
					//Saved from first screen. When in Wifi AP mode
					if(_tryToConnFail){
						retHtml = F("3");					// return html Failed
						_tryToConnFail = false;		// reset for next try
					}else if(!_connected && !_tryToConn){
						
						#if WIFI_MULTI == true
							// Shift array members to the right
							String(config.ssid[1]).toCharArray(config.ssid[2], STRUCT_CHAR_ARRAY_SIZE);
							String(config.password[1]).toCharArray(config.password[2], STRUCT_PASSWORD_SIZE);
							
							String(config.ssid[0]).toCharArray(config.ssid[1], STRUCT_CHAR_ARRAY_SIZE);
							String(config.password[0]).toCharArray(config.password[1], STRUCT_PASSWORD_SIZE);
						#endif
						
						// Replace the first array member
						request->getParam("s", true)->value().toCharArray(config.ssid[0], STRUCT_CHAR_ARRAY_SIZE);
						request->getParam("p", true)->value().toCharArray(config.password[0], STRUCT_PASSWORD_SIZE);
							
						#if DEBUG_LVL == 2
							DEBUG_PRINTLN(F("\n Connect with received credentials"));
						#endif
						
						#if DEBUG_LVL == 3
							DEBUG_PRINTF_P(PSTR(" \n Connect with received credentials: %s - %s\n"), request->getParam("s", true)->value().c_str(), request->getParam("p", true)->value().c_str());
						#endif
						
						_tryToConn = true;
						retHtml = F("2");		// busy
						
					}else if(!_connected && _tryToConn){
						
						// if server is called while connecting wifi anser busy
						retHtml = F("2");		// busy
						
					}else if(_connected && !_tryToConn){
						_changeMode = true;
						
						#if DEBUG_LVL == 3
							DEBUG_PRINTLN(F("\n Processing received credentials"));
						#endif

						#if DEBUG_LVL == 3
							DEBUG_PRINTLN(F("\n Connected and saved received credentials"));
						#endif
						
						retHtml = F("1:");	// ok:ip
						retHtml += WiFi.localIP().toString();
						delay(100);
					}
			
			}else{
					// Saved / added from config. When in Wifi STA mode
					_writeConfig = true;

					int apNr = atoi(request->getParam("i", true)->value().c_str());
					#if DEBUG_LVL == 3
						DEBUG_PRINTF_P(PSTR("\n Added wifi credentials for AP%d\n"),apNr);
					#endif
					
					request->getParam("s", true)->value().toCharArray(config.ssid[apNr-1], STRUCT_CHAR_ARRAY_SIZE);
					request->getParam("p", true)->value().toCharArray(config.password[apNr-1], STRUCT_PASSWORD_SIZE);
					retHtml = F("1");		// ok
			}

		}else{
			#if DEBUG_LVL >= 2
				DEBUG_PRINT(F("SSID or Password is missing"));
			#endif
			// failed
		}

	hdlReturn(request, retHtml);
}



/** Handle app / firmware information */
void IOTAppStory::servHdlAppInfo(AsyncWebServerRequest *request){
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(F(" Serving App Settings"));
		#endif
		
    String retHtml = F("[");
    
		for (int i = 0; i < _nrXF; ++i) {
				
				// return html results from the wifi scan
				if(i > 0){
					retHtml += F(",");
				}
				retHtml += FPSTR(HTTP_APP_INFO);
				retHtml.replace(F("{l}"), String(fieldStruct[i].fieldLabel));
				retHtml.replace(F("{v}"), String((*fieldStruct[i].varPointer)));
				retHtml.replace(F("{n}"), String(i));
				retHtml.replace(F("{m}"), String(fieldStruct[i].length));
				retHtml.replace(F("{t}"), String(fieldStruct[i].type));
				delay(10);
		}
		retHtml += F("]");
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(retHtml);
		#endif
    hdlReturn(request, retHtml, F("application/json"));
}


/** Save App Settings */
void IOTAppStory::servHdlAppSave(AsyncWebServerRequest *request) {
	#if DEBUG_LVL >= 3
		DEBUG_PRINTLN(F(" Saving App Settings"));
	#endif
	
	String retHtml = F("0");
	
	if(_nrXF){
		for(unsigned int i = 0; i < _nrXF; i++){
			if(request->hasParam(String(i), true)){
				strcpy((*fieldStruct[i].varPointer), request->getParam(String(i), true)->value().c_str());
			}
		}
		retHtml = F("1");
		writeConfig(true);
	}
	hdlReturn(request,  retHtml);
}



/** default httpclient */
void IOTAppStory::httpClientSetup(HTTPClient& http, bool httpSwitch, String url, bool spiffs) {
	if(httpSwitch == true){
	
	#if defined  ESP8266
		http.begin(url, config.sha1);
	#elif defined ESP32
		http.begin(url, ROOT_CA);
	#endif
		
	}else{
		http.begin(url);
	}
	
	// use HTTP/1.0 the update handler does not support transfer encoding
	http.useHTTP10(true);
	http.setTimeout(8000);
	http.setUserAgent(F("ESP-http-Update"));

	http.addHeader(F("x-ESP-STA-MAC"), WiFi.macAddress());
	http.addHeader(F("x-ESP-ACT-ID"), String(config.actCode));
	
	#if defined ESP32
		http.addHeader(F("x-ESP-CORE-VERSION"), String(ESP.getSdkVersion()));
	#elif defined ESP8266
		http.addHeader(F("x-ESP-FREE-SPACE"), String(ESP.getFreeSketchSpace()));
		http.addHeader(F("x-ESP-SKETCH-SIZE"), String(ESP.getSketchSize()));
		http.addHeader(F("x-ESP-SKETCH-MD5"), String(ESP.getSketchMD5()));
		http.addHeader(F("x-ESP-FLASHCHIP-ID"), String(ESP.getFlashChipId()));
		http.addHeader(F("x-ESP-CHIP-ID"), String(ESP.getChipId()));
		http.addHeader(F("x-ESP-CORE-VERSION"), String(ESP.getCoreVersion()));
	#endif

	http.addHeader(F("x-ESP-FLASHCHIP-SIZE"), String(ESP.getFlashChipSize()));
	http.addHeader(F("x-ESP-VERSION"), String(_appName) + " " + String(_appVersion));
	
	if(spiffs) {
			http.addHeader(F("x-ESP-MODE"), F("spiffs"));
	} else {
			http.addHeader(F("x-ESP-MODE"), F("sketch"));
	}
}


/** return page handler */
void IOTAppStory::hdlReturn(AsyncWebServerRequest *request, String &retHtml, String type) {
	#if CFG_AUTHENTICATE == true
	if(!request->authenticate("admin", config.cfg_pass)){ 
		return request->requestAuthentication(); 
	}else{
	#endif
		
		AsyncWebServerResponse *response = request->beginResponse(200, type, retHtml);
		response->addHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
		response->addHeader(F("Pragma"), F("no-cache"));
		response->addHeader(F("Expires"), F("-1"));
		request->send(response);
	
	#if CFG_AUTHENTICATE == true
	}
	#endif
}
