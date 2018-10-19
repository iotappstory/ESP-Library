#include "IOTAppStory.h"


IOTAppStory::IOTAppStory(const char *compDate, const int modeButton)
: _compDate(compDate)
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
			DEBUG_PRINTLN(SER_ERASE_FULL);
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
			DEBUG_PRINTLN(SER_ERASE_PART);
		#endif
		
		#if DEBUG_LVL >= 2
			DEBUG_PRINTLN(SER_ERASE_PART_EXT);
		#endif
		
		// erase eeprom but leave the config settings
		eraseFlash((sizeof(config)+2),EEPROM_SIZE);
	}
	#if DEBUG_LVL >= 1
	else{
		DEBUG_PRINTLN(SER_ERASE_NONE);
	}
	#endif
	
	boardMode = 'N';
	bootTimes = 0;
	boardInfo boardInfo(bootTimes, boardMode);
	boardInfo.write();
	
	// update first boot config flag (date)
	strcpy(config.compDate, _compDate);
	writeConfig();

	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN(FPSTR(SER_DEV));
	#endif
	
	if (_firstBootCallback){
		
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(SER_CALLBACK_FIRST_BOOT);
		#endif
		_firstBootCallback();
	}
}



void IOTAppStory::preSetAppName(String appName){
	if (!_configReaded) {
		readConfig();
	}
	_setDeviceName = true;
	SetConfigValueCharArray(config.appName, appName, 33, _setPreSet);
}
void IOTAppStory::preSetAppVersion(String appVersion){
	if (!_configReaded) {
		readConfig();
	}
	_setDeviceName = true;
	SetConfigValueCharArray(config.appVersion, appVersion, 11, _setPreSet);
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
			DEBUG_PRINTLN(SER_SAVE_CONFIG);
		#endif
	}

	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN(FPSTR(SER_DEV));
		
		DEBUG_PRINT(SER_START);
		DEBUG_PRINT(config.appName);
		DEBUG_PRINT(F(" v"));
		DEBUG_PRINTLN(config.appVersion);
	#endif
	#if DEBUG_LVL >= 2
		DEBUG_PRINTLN(FPSTR(SER_DEV));
		DEBUG_PRINTF_P(SER_MODE_SEL_BTN, _modeButton, config.deviceName, _updateOnBoot);
	#endif
	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN(FPSTR(SER_DEV));
	#endif
	
	// set the input pin for Config/Update mode selection
	pinMode(_modeButton, INPUT_PULLUP);

	// Read the "bootTime" & "boardMode" from the Non-volatile storage on ESP32 processor
	//readPref();
	boardInfo boardInfo(bootTimes, boardMode);
	boardInfo.read();
	
	// on first boot of the app run the firstBoot() function
	if(strcmp(config.compDate,_compDate) != 0){
		firstBoot(ea);
	}
	
	// BOOT STATISTICS read and increase boot statistics (optional)
	#if BOOTSTATISTICS == true && DEBUG_LVL >= 1
		bootTimes++;
		//writePref();
		boardInfo.write();
		
		#if DEBUG_LVL >= 1
			printBoardInfo();
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


#if DEBUG_LVL >= 1
/** print BoardInfo */
void IOTAppStory::printBoardInfo(){
	DEBUG_PRINTF_P(SER_BOOTTIMES_UPDATE, bootTimes, boardMode);
	DEBUG_PRINTLN(FPSTR(SER_DEV));
}
#endif


/** send msg to iasLog */
void IOTAppStory::iasLog(String msg) {
		// notifi IAS & enduser about the localIP
		String url = F("https://");
		url += _updateHost;
		url += F("/ota/cfg-sta.php");
		url += "?msg=";
		url += msg;

		HTTPClient http;
		httpClientSetup(http, url, false);

		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(SER_UPDATE_IASLOG);
		#endif

		if(http.GET() != HTTP_CODE_OK){
			#if DEBUG_LVL >= 3
				DEBUG_PRINTLN(SER_FAILED_COLON);
				DEBUG_PRINTLN(http.getString());
			#endif
		}
		http.end();
}


/** config server */
void IOTAppStory::runConfigServer() {
	
	bool exitConfig = false;

	// callback entered config mode
	if (_configModeCallback){
		_configModeCallback();
	}

	#if DEBUG_LVL >= 1
		DEBUG_PRINT(SER_CONFIG_MODE);
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
			DEBUG_PRINTF_P(SER_CONFIG_AP_MODE, config.deviceName);
		#endif
		
	}else{
		
		// notifi IAS & enduser this device went to config mode (also sends localIP)
		iasLog("1");
		
		// when there is wifi setup server in STA mode
		server.reset(new AsyncWebServer(80));
		WiFi.mode(WIFI_STA);
		
		#if DEBUG_LVL >= 2
			DEBUG_PRINT(SER_CONFIG_STA_MODE);
			DEBUG_PRINTLN(WiFi.localIP());
		#endif
		
	}

	
	server->on("/", HTTP_GET, [&](AsyncWebServerRequest *request){ servHdlRoot(request); });
	server->on("/i", HTTP_GET, [&](AsyncWebServerRequest *request){ servHdlDevInfo(request); });
	#if defined  ESP8266
		server->on("/fp", HTTP_POST, [&](AsyncWebServerRequest *request){ servHdlFngPrintSave(request); });
	#endif
	server->on("/wsc", HTTP_GET, [&](AsyncWebServerRequest *request){ servHdlWifiScan(request); });
	server->on("/wsa", HTTP_POST, [&](AsyncWebServerRequest *request){ servHdlWifiSave(request); });
	server->on("/app", HTTP_GET, [&](AsyncWebServerRequest *request){ servHdlAppInfo(request); });
	server->on("/as", HTTP_POST, [&](AsyncWebServerRequest *request){ servHdlAppSave(request); });

	server->on("/ds", HTTP_POST, [&](AsyncWebServerRequest *request){
		#if DEBUG_LVL >= 3
			DEBUG_PRINT(SER_REC_ACT_CODE);
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
					DEBUG_PRINT(SER_REC_CREDENTIALS);
					DEBUG_PRINT(config.ssid[0]);
					DEBUG_PRINT(F(" - "));
					DEBUG_PRINTLN(config.password[0]);
				#endif
				

				WiFi.begin(config.ssid[0], config.password[0]);
				_connected = isNetworkConnected(false);
				yield();
				
				if(_connected){
					
					// Saving config to eeprom
					_writeConfig = true;
					
					#if DEBUG_LVL >= 3
						DEBUG_PRINT(SER_CONN_SAVE_EEPROM);
					#endif
				}else{
					readConfig();
					#if DEBUG_LVL >= 1
						DEBUG_PRINTLN(SER_FAILED_TRYAGAIN);
					#endif
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
				
				// notifi IAS & enduser this device went to config mode (also sends localIP)
				iasLog("1");
				
				#if DEBUG_LVL >= 2
					//DEBUG_PRINTF_P(PSTR(" \n Changed to STA mode. Open %s\n"), WiFi.localIP().toString());
					
					DEBUG_PRINT(SER_CONFIG_STA_MODE_CHANGE);
					DEBUG_PRINTLN(WiFi.localIP());
					DEBUG_PRINTLN();
				#endif
			}
			
		}
  }
	
	#if DEBUG_LVL >= 2
		DEBUG_PRINTLN(SER_CONFIG_EXIT);
	#endif
	
	// notifi IAS & enduser this device has left config mode (also sends localIP)
	iasLog("0");
	
	// Return to Normal Operation
	espRestart('N');
}


/** Connect to Wifi AP */
void IOTAppStory::connectNetwork() {
	
	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN(SER_CONNECTING);
	#endif
	
	#if WIFI_MULTI == true
		wifiMulti.addAP(config.ssid[0], config.password[0]);
		wifiMulti.addAP(config.ssid[1], config.password[1]);
		wifiMulti.addAP(config.ssid[2], config.password[2]);
	#else
		WiFi.begin(config.ssid[0], config.password[0]);
	#endif
	


	if(!isNetworkConnected()) {
		

			if(_automaticConfig || boardMode == 'C'){
				
				if(boardMode == 'N'){
					boardMode = 'C';
					//writePref();
					boardInfo boardInfo(bootTimes, boardMode);
					boardInfo.write();
				}
				
				#if DEBUG_LVL >= 1
					DEBUG_PRINT(SER_CONN_NONE_GO_CFG);
				#endif
				
			}else{
				
				#if DEBUG_LVL >= 1
					// this point is only reached if _automaticConfig = false
					DEBUG_PRINT(SER_CONN_NONE_CONTINU);
				#endif
			}

	}else{
		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(SER_CONNECTED);
		#endif

		#if DEBUG_LVL >= 2
			DEBUG_PRINT(SER_DEV_MAC);
			DEBUG_PRINTLN(WiFi.macAddress());
		#endif

		#if DEBUG_LVL >= 1
			DEBUG_PRINT(SER_DEV_IP);
			DEBUG_PRINTLN(WiFi.localIP());
		#endif

		#if USEMDNS == true
			// Register host name in WiFi and mDNS
			String hostNameWifi = config.deviceName;
			hostNameWifi += ".local";

			if(MDNS.begin(config.deviceName)){

				#if DEBUG_LVL >= 1
					DEBUG_PRINT(SER_DEV_MDNS);
					DEBUG_PRINT(hostNameWifi);
				#endif

				#if DEBUG_LVL >= 2
					DEBUG_PRINTLN(SER_DEV_MDNS_INFO);
				#endif
				#if DEBUG_LVL == 1
					DEBUG_PRINTLN(F(""));
				#endif

			}else{
				#if DEBUG_LVL >= 1
					DEBUG_PRINTLN(SER_DEV_MDNS_FAIL);
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
bool IOTAppStory::isNetworkConnected(bool multi) {
	#if defined  ESP8266
		int retries = MAX_WIFI_RETRIES;
	#elif defined ESP32
		int retries = (MAX_WIFI_RETRIES/2);
	#endif

	#if DEBUG_LVL >= 1
		DEBUG_PRINT(F(" "));
	#endif

	#if WIFI_MULTI == true
		if(multi){
			while (wifiMulti.run() != WL_CONNECTED && retries-- > 0 ) {
				delay(500);
				#if DEBUG_LVL >= 1
					DEBUG_PRINT(F("."));
				#endif
			}

		}else{
	#endif
			while (WiFi.status() != WL_CONNECTED && retries-- > 0 ) {	
				delay(500);
				#if DEBUG_LVL >= 1
					DEBUG_PRINT(F("."));
				#endif

			}
	#if WIFI_MULTI == true
		}
	#endif	

	
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

	// update from IOTappStory.com

	#if DEBUG_LVL >= 2
		DEBUG_PRINTLN(SER_CALLING_HOME);
	#endif

	if (_firmwareUpdateCheckCallback){
		_firmwareUpdateCheckCallback();
	}

	// try to update from IOTAppStory
	iotUpdater(0);

	// try to update spiffs from IOTAppStory
	if(spiffs){

		iotUpdater(1);
	}

	#if DEBUG_LVL >= 2
		DEBUG_PRINTLN(SER_RET_FROM_IAS);
	#endif
	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN(FPSTR(SER_DEV));
	#endif
	
}


/**
	IOT updater
*/
void IOTAppStory::iotUpdater(bool spiffs) {
	String url = "";

	#if DEBUG_LVL >= 2
		DEBUG_PRINT(F("\n"));
	#endif
	#if DEBUG_LVL >= 1
		DEBUG_PRINT(SER_CHECK_FOR);
	#endif
	#if DEBUG_LVL >= 1
		if(spiffs == false){
			// type = sketch
			DEBUG_PRINT(SER_APP_SKETCH);
		}
		if(spiffs == true){
			// type = spiffs
			DEBUG_PRINT(SER_SPIFFS);
		}
	#endif
	#if DEBUG_LVL >= 2
		DEBUG_PRINT(SER_UPDATES_FROM);
	#endif
	#if DEBUG_LVL == 1
		DEBUG_PRINT(SER_UPDATES);
	#endif

	#if HTTPS == true
		url = F("https://");
	#else
		url = F("http://");
	#endif

	// location 1
	url += _updateHost;
	url += _updateFile;
	
	#if DEBUG_LVL >= 2
		DEBUG_PRINTLN(url);
	#endif
	#if DEBUG_LVL == 1
		DEBUG_PRINTLN("");		
	#endif

	HTTPClient http;
	httpClientSetup(http, url, spiffs);

	// track these headers for later use
	const char * headerkeys[] = { "x-MD5", "x-name", "x-ver"};
	size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
	http.collectHeaders(headerkeys, headerkeyssize);

	int code = http.GET();
	int len = http.getSize();
	
	if(code == HTTP_CODE_OK){

		ESP8266HTTPUpdate ESPhttpUpdate;

		#if DEBUG_LVL >= 1
			DEBUG_PRINT(SER_DOWN_AND_PROC);
		#endif

		if (_firmwareUpdateDownloadCallback){
			_firmwareUpdateDownloadCallback();
		}


		if(ESPhttpUpdate.handleUpdate(http, len, spiffs)) {
			// succesfull update
			#if DEBUG_LVL >= 1
				DEBUG_PRINTLN(SER_REBOOT_NEC);
			#endif
			
			// store received appName & appVersion
			strcpy(config.appName,  http.header("x-name").c_str());
			strcpy(config.appVersion,  http.header("x-ver").c_str());
			writeConfig();

			if (_firmwareUpdateSuccessCallback){
				_firmwareUpdateSuccessCallback();
			}
			
			// reboot
			ESP.restart();
		}

	}else{
		#if DEBUG_LVL >= 1
			if(code > 0){
				DEBUG_PRINTLN(http.getString());
			}else{
				// Failed. Server down OR HTTPS problem
				DEBUG_PRINTLN(SER_CALLHOME_FAILED);
			}
		#endif
		
		#if defined ESP32
			if(code != 399 && _firmwareUpdateErrorCallback){
				_firmwareUpdateErrorCallback();
			}

		#elif defined ESP8266
			if(code != HTTP_CODE_NOT_MODIFIED && _firmwareUpdateErrorCallback){
				_firmwareUpdateErrorCallback();
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
	eepFreeFrom = sizeof(config)+2;
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
		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(SER_PROC_FIELDS);
		#endif
		#if DEBUG_LVL >= 2
			DEBUG_PRINTLN(SER_PROC_TBL_HDR);
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
			const unsigned int eeBeg = eepFreeFrom+prevTotLength+nr+((nr-1)*2);
			const unsigned int eeEnd = eepFreeFrom+(prevTotLength+sizeOfVal)+nr+1+((nr-1)*2);

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
						DEBUG_PRINTF_P(SER_PROC_TBL_OVRW, eepVal);
					#endif
					
					(*fieldStruct[nr-1].varPointer) = eepVal;
				}else{
					#if DEBUG_LVL >= 2
						DEBUG_PRINTF_P(SER_PROC_TBL_DEF, (*fieldStruct[nr-1].varPointer));
					#endif
				}

			}else{
				#if DEBUG_LVL >= 2
					DEBUG_PRINTF_P(SER_PROC_TBL_WRITE, (*fieldStruct[nr-1].varPointer));
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
			
			if(nr == _nrXF){
				eepFreeFrom = eeEnd+ 2;
			}
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

		// https://github.com/esp8266/Arduino/blob/master/variants/oak/pins_arduino.h
		// DEBUG_PRINTLN("- Digistump OAK -");
		if      (orgVal == "P0"  || orgVal == "2")    return P0;
		else if (orgVal == "P1"  || orgVal == "5")    return P1;
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
		else                                          return P0;

	#elif defined ESP8266_WEMOS_D1MINI || defined ESP8266_WEMOS_D1MINILITE || defined ESP8266_WEMOS_D1MINIPRO

		// https://github.com/esp8266/Arduino/blob/master/variants/d1_mini/pins_arduino.h
		// DEBUG_PRINTLN("- build-variant d1_mini -");
		if      (orgVal == "D0"  || orgVal == "16")   return D0;
		else if (orgVal == "D1"  || orgVal == "5")    return D1;
		else if (orgVal == "D2"  || orgVal == "4")    return D2;
		else if (orgVal == "D3"  || orgVal == "0")    return D3;
		else if (orgVal == "D4"  || orgVal == "2")    return D4;
		else if (orgVal == "D5"  || orgVal == "14")   return D5;
		else if (orgVal == "D6"  || orgVal == "12")   return D6;
		else if (orgVal == "D7"  || orgVal == "13")   return D7;
		else if (orgVal == "D8"  || orgVal == "15")   return D8;
		else if (orgVal == "RX"  || orgVal == "3")    return RX;
		else if (orgVal == "TX" || orgVal == "1")     return TX;
		else                                          return D0;

	#elif defined ESP8266_NODEMCU || defined WIFINFO

		// https://github.com/esp8266/Arduino/blob/master/variants/wifinfo/pins_arduino.h
		// https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h
		// DEBUG_PRINTLN("- build-variant nodemcu and wifinfo -");
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
	boardInfo boardInfo(bootTimes, boardMode);
	boardInfo.write();

	ESP.restart();
}



/** 
	Erase flash from till
*/
void IOTAppStory::eraseFlash(int eepFrom, int eepTo) {
	#if DEBUG_LVL >= 2
		DEBUG_PRINTF_P(SER_ERASE_FLASH, eepFrom, eepTo);
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
	for (unsigned int t = 0; t < sizeof(config); t++) {
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
		for (unsigned int nr = 1; nr <= _nrXF; nr++){
			
			int prevTotLength = 0;
			for(unsigned int i = 0; i < (nr-1); i++){
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
			DEBUG_PRINTLN(SER_EEPROM_FOUND);
		#endif
		
		for(unsigned int t = 0; t < sizeof(config); t++){
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
			DEBUG_PRINTLN(SER_EEPROM_NOT_FOUND);
		#endif
		
		writeConfig();
	}
	
	_configReaded = true;
}


void IOTAppStory::loop() {
	if (_callHome && millis() - _lastCallHomeTime > _callHomeInterval) {
		this->callHome();
		_lastCallHomeTime = millis();
	}

	this->buttonLoop();
}


ModeButtonState IOTAppStory::buttonLoop() {
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
			callHome();
			continue;
#if INC_CONFIG == true	
		case AppStateConfigMode:
			_appState = AppStateNoPress;
			#if DEBUG_LVL >= 1
				DEBUG_PRINTLN(SER_CONFIG_ENTER);
			#endif
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
void IOTAppStory::onFirmwareUpdateSuccess(THandlerFunction value) {
	_firmwareUpdateSuccessCallback = value;
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
		DEBUG_PRINTLN(SER_SERV_DEV_INFO);
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
		retHtml.replace(F("{f}"), config.sha1);
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
			DEBUG_PRINTLN(SER_SERV_WIFI_SCAN_RES);
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
							DEBUG_PRINTLN(SER_CONN_REC_CRED);
						#endif
						
						#if DEBUG_LVL == 3
							DEBUG_PRINTF_P(SER_CONN_REC_CRED_DB3, request->getParam("s", true)->value().c_str(), request->getParam("p", true)->value().c_str());
						#endif
						
						_tryToConn = true;
						retHtml = F("2");		// busy
						
					}else if(!_connected && _tryToConn){
						
						// if server is called while connecting wifi anser busy
						retHtml = F("2");		// busy
						
					}else if(_connected && !_tryToConn){
						_changeMode = true;
						
						#if DEBUG_LVL == 3
							DEBUG_PRINTLN(SER_CONN_REC_CRED_PROC);
						#endif

						#if DEBUG_LVL == 3
							DEBUG_PRINTLN(SER_CONN_SAVE_EEPROM);
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
						DEBUG_PRINTF_P(SER_CONN_ADDED_AP_CRED,apNr);
					#endif
					
					request->getParam("s", true)->value().toCharArray(config.ssid[apNr-1], STRUCT_CHAR_ARRAY_SIZE);
					request->getParam("p", true)->value().toCharArray(config.password[apNr-1], STRUCT_PASSWORD_SIZE);
					retHtml = F("1");		// ok
			}

		}else{
			#if DEBUG_LVL >= 2
				DEBUG_PRINT(SER_CONN_CRED_MISSING);
			#endif
			// failed
		}

	hdlReturn(request, retHtml);
}



/** Handle app / firmware information */
void IOTAppStory::servHdlAppInfo(AsyncWebServerRequest *request){
	#if DEBUG_LVL >= 3
		DEBUG_PRINTLN(SER_SERV_APP_SETTINGS);
	#endif

	String retHtml = F("[");

	for (unsigned int i = 0; i < _nrXF; ++i) {

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



#if defined  ESP8266
/** Save new fingerprint */
void IOTAppStory::servHdlFngPrintSave(AsyncWebServerRequest *request){
	#if DEBUG_LVL >= 3
		DEBUG_PRINTLN(SER_SAVE_FINGERPRINT);
	#endif
	
	request->getParam("f", true)->value().toCharArray(config.sha1, 60);
	
	String retHtml = F("1");
	writeConfig();
	hdlReturn(request,  retHtml);
}
#endif



/** Save App Settings */
void IOTAppStory::servHdlAppSave(AsyncWebServerRequest *request) {
	#if DEBUG_LVL >= 3
		DEBUG_PRINTLN(SER_SAVE_APP_SETTINGS);
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
void IOTAppStory::httpClientSetup(HTTPClient& http, String url, bool spiffs) {
	#if HTTPS == true
	
		#if defined  ESP8266
			http.begin(url, config.sha1);
		#elif defined ESP32
			http.begin(url, ROOT_CA);
		#endif
		
	#else
		http.begin(url);
	#endif
	
	// use HTTP/1.0 the update handler does not support transfer encoding
	http.useHTTP10(true);
	http.setTimeout(8000);
	http.setUserAgent(F("ESP-http-Update"));

	http.addHeader(F("x-ESP-STA-MAC"), WiFi.macAddress());
	http.addHeader(F("x-ESP-ACT-ID"), String(config.actCode));
	http.addHeader(F("x-ESP-LOCIP"), String(WiFi.localIP().toString()));
	
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
	http.addHeader(F("x-ESP-VERSION"), String(config.appName) + " v" + config.appVersion);
	
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
