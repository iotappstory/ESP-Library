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
void IOTAppStory::firstBoot(const char ea){

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



void IOTAppStory::setCallHome(bool callHome) {} // <----- deprecated left for compatibility. Remove with version 3.0.0

void IOTAppStory::setCallHomeInterval(unsigned long interval) {
	_callHomeInterval = interval * 1000; //Convert to millis so users can pass seconds to this function
}



void IOTAppStory::begin(const char ea){
	
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
	
	// set the "hard" reset(power) pin for the Nextion display
	// and turn the display on
	#if OTA_UPD_CHECK_NEXTION == true
		pinMode(NEXT_RES, OUTPUT);
		digitalWrite(NEXT_RES, HIGH);
	#endif

	// Read the "bootTime" & "boardMode" from the Non-volatile storage on ESP32 processor
	boardInfo boardInfo(bootTimes, boardMode);
	boardInfo.read();
	
	// on first boot of the app run the firstBoot() function
	if(strcmp(config.compDate,_compDate) != 0){
		firstBoot(ea);
	}
	
	// BOOT STATISTICS read and increase boot statistics (optional)
	#if BOOTSTATISTICS == true && DEBUG_LVL >= 1
		bootTimes++;
		boardInfo.write();
		
		#if DEBUG_LVL >= 1
			printBoardInfo();
		#endif
	#endif

	// process added fields
	processField();
	
	
	// --------- START WIFI --------------------------
	// Setup wifi with cred etc connect to AP
	WiFiSetupAndConnect();
	

	// Synchronize time useing SNTP. This is necessary to verify that
	// the TLS certificates offered by servers are currently valid.
	#if SNTP_INT_CLOCK_UPD == true
		if(_connected){
			this->setClock();
		}
	#endif
	
	//---------- SELECT BOARD MODE -----------------------------
	#if CFG_INCLUDE == true
		if(boardMode == 'C'){
			{
				// callback entered config mode
				if(_configModeCallback){
					_configModeCallback();
				}
				
				// notifi IAS & enduser this device went to config mode (also sends localIP)
				#if CFG_STORAGE != ST_SPIFSS && CFG_ANNOUNCE == true
					if(_connected){
						this->iasLog("1");
					}
				#endif
			#ifdef ESP32	//<<--- this is to prevent nasty async tcp errors
			}	
			#endif
				// run config server
				configServer configServer(*this);
				configServer.run();
			#ifdef ESP8266
			}	
			#endif
			delay(100);
			
			// notifi IAS & enduser this device has left config mode (also sends localIP)
			#if CFG_STORAGE != ST_SPIFSS && CFG_ANNOUNCE == true
				if(_connected){
					this->iasLog("0");
				}
			#endif
			
			// Restart & return to Normal Operation
			this->espRestart('N');
		}
	#endif
	
	// --------- if connection & automaticUpdate Update --------------------------
	if(_connected && _updateOnBoot == true){
		callHome();
	}

	_buttonEntry = millis() + MODE_BUTTON_VERY_LONG_PRESS;    // make sure the timedifference during startup is bigger than 10 sec. Otherwise it will go either in config mode or calls home
	_appState = AppStateNoPress;


	#if DEBUG_FREE_HEAP == true
		DEBUG_PRINTLN(" end of IAS::begin");
		DEBUG_PRINTF(" Free heap: %u\n", ESP.getFreeHeap());
	#endif
		
	#if DEBUG_LVL >= 1
		DEBUG_PRINT(F("\n\n\n\n\n"));
	#endif
}



/** print BoardInfo */
#if DEBUG_LVL >= 1
void IOTAppStory::printBoardInfo(){
	DEBUG_PRINTF_P(SER_BOOTTIMES_UPDATE, bootTimes, boardMode);
	DEBUG_PRINTLN(FPSTR(SER_DEV));
}
#endif



/** send msg to iasLog */
void IOTAppStory::iasLog(String msg) {
	// notifi IAS & enduser about the localIP
	callServer		callServer(config, U_LOGGER);
	callServer.sm(&statusMessage);
	msg.replace(" ", "_");
	msg = "msg="+msg;
	
	#if DEBUG_LVL >= 3
		DEBUG_PRINT(SER_UPDATE_IASLOG);
	#endif
	
	if(!callServer.get(OTA_LOG_FILE, msg)){
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(SER_FAILED_COLON);
			DEBUG_PRINTLN(" " + statusMessage);
		#endif
	}
}



/** Connect to Wifi AP */
void IOTAppStory::WiFiSetupAndConnect() {
	
	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN(SER_CONNECTING);
	#endif
	
	// setup wifi credentials
	#if WIFI_MULTI == true
		// add multiple credentials
		wifiMulti.addAP(config.ssid[0], config.password[0]);
		wifiMulti.addAP(config.ssid[1], config.password[1]);
		wifiMulti.addAP(config.ssid[2], config.password[2]);
	#else
		// add single credential
		WiFi.begin(config.ssid[0], config.password[0]);
	#endif
	
	// connect to access point
	if(!WiFiConnectToAP()){
		// FAILED
		// if conditions are met, set to config mode (C)
		if(_automaticConfig || boardMode == 'C'){
			
			if(boardMode == 'N'){
				boardMode = 'C';
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
		// SUCCES
		// Show connection details if debug level is set

		#if DEBUG_LVL >= 2
			DEBUG_PRINT(SER_DEV_MAC);
			DEBUG_PRINTLN(WiFi.macAddress());
		#endif

		// Register host name in WiFi and mDNS
		#if WIFI_USE_MDNS == true
			
			// wifi_station_set_hostname(config.deviceName);
			// WiFi.hostname(hostNameWifi);

			if(MDNS.begin(config.deviceName)){

				#if DEBUG_LVL >= 1
					DEBUG_PRINT(SER_DEV_MDNS);
					DEBUG_PRINT(config.deviceName);
					DEBUG_PRINT(".local");
				#endif

				#if DEBUG_LVL >= 3
					DEBUG_PRINTLN(SER_DEV_MDNS_INFO);
				#endif
				#if DEBUG_LVL == 2
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
	Returns false if not connected after WIFI_CONN_MAX_RETRIES retries 
*/
bool IOTAppStory::WiFiConnectToAP(bool multi){
	#if defined  ESP8266
		int retries = WIFI_CONN_MAX_RETRIES;
	#elif defined ESP32
		int retries = (WIFI_CONN_MAX_RETRIES/2);
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

		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(SER_CONNECTED);
		#endif
		#if DEBUG_LVL >= 1
			DEBUG_PRINT(SER_DEV_IP);
			DEBUG_PRINTLN(WiFi.localIP());
		#endif
		_connected = true;
		return true;
	}else{
		#if DEBUG_LVL >= 1
			DEBUG_PRINT(F("\n Failed!"));
		#endif
		_connected = false;
		return false;
	}
}



/**
	Dusconnect wifi
*/
void IOTAppStory::WiFiDisconnect(){
	WiFi.disconnect();
	_connected = false;
	#if DEBUG_LVL >= 2
		DEBUG_PRINTLN(F(" WiFi disconnected!"));
		DEBUG_PRINTLN(FPSTR(SER_DEV));
	#endif
}



/**
	Set time via NTP, as required for x.509 validation
*/
void IOTAppStory::setClock(){
	#if defined  ESP8266
		int retries = WIFI_CONN_MAX_RETRIES;
	#elif defined ESP32
		int retries = (WIFI_CONN_MAX_RETRIES/2);
	#endif

	configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

	#if DEBUG_LVL >= 2
		DEBUG_PRINT(SER_SYNC_TIME_NTP);
	#endif
	
	time_t now = time(nullptr);
	while (now < 8 * 3600 * 2 && retries-- > 0 ){
		delay(500);
		#if DEBUG_LVL >= 2
			DEBUG_PRINT(F("."));
		#endif
		now = time(nullptr);
	}

	if(retries > 0){
		struct tm timeinfo;
		gmtime_r(&now, &timeinfo);
		_timeSet 		= true;
		_lastTimeSet 	= millis();
		
		#if DEBUG_LVL >= 3
			DEBUG_PRINT(F("\n Current time: "));
			DEBUG_PRINT(asctime(&timeinfo));
		#endif
	}else{
		_timeSet 		= false;
		_lastTimeSet 	= 0;
		
		#if DEBUG_LVL >= 2
			DEBUG_PRINTLN(SER_FAILED_EXCL);
		#endif
	}
	#if DEBUG_LVL >= 2
		DEBUG_PRINTLN(F(""));
		DEBUG_PRINTLN(FPSTR(SER_DEV));
	#endif
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

	{
		// try to update sketch from IOTAppStory
		iotUpdater();
	}

	// try to update spiffs from IOTAppStory
	#if OTA_UPD_CHECK_SPIFFS == true
		if(spiffs){
			{
				iotUpdater(U_SPIFFS);
			}
		}
	#endif
	
	#if OTA_UPD_CHECK_NEXTION == true
		{
			iotUpdater(U_NEXTION);
		}
	#endif

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
bool IOTAppStory::iotUpdater(int command) {

	bool result;
	{
		#if DEBUG_LVL >= 2
			DEBUG_PRINT(F("\n"));
		#endif
		#if DEBUG_LVL >= 1
			DEBUG_PRINT(SER_CHECK_FOR);
		#endif
		#if DEBUG_LVL >= 1
			if(command == U_FLASH){
				DEBUG_PRINT(SER_APP_SKETCH);
				
			}else if(command == U_SPIFFS){
				DEBUG_PRINT(SER_SPIFFS);
				
			}
			#if OTA_UPD_CHECK_NEXTION == true
				else if(command == U_NEXTION){
					DEBUG_PRINT(SER_NEXTION);
				}
			#endif
		#endif
		#if DEBUG_LVL >= 2
			DEBUG_PRINT(SER_UPDATES_FROM);
		#endif
		#if DEBUG_LVL == 1
			DEBUG_PRINT(SER_UPDATES);
		#endif
		#if DEBUG_LVL >= 2
			#if HTTPS == true
				DEBUG_PRINT(F("https://"));
			#else
				DEBUG_PRINT(F("http://"));
			#endif
			DEBUG_PRINT(OTA_HOST);
			DEBUG_PRINTLN(OTA_UPD_FILE);
		#endif
		#if DEBUG_LVL == 1
			DEBUG_PRINTLN("");
		#endif

		firmwareStruct	firmwareStruct;
		callServer		callServer(config, command);
		callServer.sm(&statusMessage);
		
		
		Stream &clientStream = callServer.getStream(&firmwareStruct);
		
		if(!firmwareStruct.success){
			#if DEBUG_LVL >= 2
				DEBUG_PRINTLN(" " + statusMessage);
			#endif
			
			return false;
		}
		
		
		if (_firmwareUpdateDownloadCallback){
			_firmwareUpdateDownloadCallback();
		}

		
		
		if(command == U_FLASH || command == U_SPIFFS){
			// sketch / spiffs
			result = espInstaller(clientStream, &firmwareStruct, UpdateESP, command);
		}
		#if OTA_UPD_CHECK_NEXTION == true
			if(command == U_NEXTION){
				// nextion display
				espInstaller(clientStream, &firmwareStruct, UpdateNextion, command);
			}
		#endif
	}
	
	if(result && (command == U_FLASH || command == U_NEXTION)){

		// write changes to config
		writeConfig();

		// succesfull update
		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(SER_REBOOT_NEC);
		#endif
		
		// reboot to start the new updated firmware
		ESP.restart();
	}
	
	return true;
}



/**
	espInstaller
*/
bool IOTAppStory::espInstaller(Stream &streamPtr, firmwareStruct *firmwareStruct, UpdateClassVirt& devObj, int command) {
	devObj.sm(&statusMessage);
	bool result = devObj.prepareUpdate((*firmwareStruct).xlength, (*firmwareStruct).xmd5, command);

	if(!result){
		#if DEBUG_LVL >= 2
			DEBUG_PRINTLN(statusMessage);
		#endif
	}else{
		
		#if DEBUG_LVL >= 2
			DEBUG_PRINT(SER_INSTALLING);
		#endif
		
		
		// Write the buffered bytes to the esp. If this fails, return false.
		//Serial.println(streamPtr.available());
		/*result = devObj.update(streamPtr);*/

		{
			// create buffer for read
			uint8_t buff[2048] = { 0 };
			
			// to do counter
			uint32_t updTodo = (*firmwareStruct).xlength;
			
			// Upload the received byte Stream to the device
			while(updTodo > 0 || updTodo == -1){
				
				// get available data size
				size_t size = streamPtr.available();

				if(size){
					// read up to 2048 byte into the buffer
					size_t c = streamPtr.readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

					// Write the buffered bytes to the esp. If this fails, return false.
					result = devObj.update(buff, c);
					
					if(updTodo > 0) {
						updTodo -= c;
					}

					if (_firmwareUpdateProgressCallback){
						_firmwareUpdateProgressCallback((*firmwareStruct).xlength - updTodo, (*firmwareStruct).xlength);
					}
				}
				delay(1);
			}
		}



	  
		if(!result){
			#if DEBUG_LVL >= 2
				DEBUG_PRINT(SER_UPDATEDERROR);
				DEBUG_PRINTLN(statusMessage);
			#endif
		}else{

			// end: wait(delay) for the nextion to finish the update process, send nextion reset command and end the serial connection to the nextion
			result = devObj.end();

			if(result){
				// on succesfull firmware installation
				#if DEBUG_LVL >= 2
					DEBUG_PRINT(SER_UPDATEDTO);
					DEBUG_PRINTLN((*firmwareStruct).xname+" v"+ (*firmwareStruct).xver);
				#endif
				

				if(command == U_FLASH){
					// write received appName & appVersion to config
					(*firmwareStruct).xname.toCharArray(config.appName, 33);
					(*firmwareStruct).xver.toCharArray(config.appVersion, 12);
				}
				
				#if OTA_UPD_CHECK_NEXTION == true
					if(command == U_NEXTION){
						// update nextion md5
						(*firmwareStruct).xmd5.toCharArray(config.next_md5, 33);
					}
				#endif
				
				

				if (_firmwareUpdateSuccessCallback){
					_firmwareUpdateSuccessCallback();
				}
				

			}else{
				// update failed
				#if DEBUG_LVL >= 2
					DEBUG_PRINTLN(" " + statusMessage);
				#endif
				if(_firmwareUpdateErrorCallback){
					_firmwareUpdateErrorCallback();
				}
			}
		}
		
		
	}
	return result;
}



/** 
	Add fields to the fieldStruct
*/
void IOTAppStory::addField(char* &defaultVal, const char *fieldLabel, const int length, const char type){
	
	
	if(_nrXF <= MAXNUMEXTRAFIELDS){
		// add values to the fieldstruct
		fieldStruct[_nrXF].fieldLabel 	= fieldLabel;
		fieldStruct[_nrXF].varPointer 	= &defaultVal;
		fieldStruct[_nrXF].length 		= length+1;
		fieldStruct[_nrXF].type 		= type;
		
		_nrXF++;
	}else{
		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(SER_PROC_ERROR);
		#endif
	}
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
	if(_nrXF > 0 && _nrXF <= MAXNUMEXTRAFIELDS){
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
			strcpy((*fieldStruct[nr].varPointer), tempValue[nr]);
		}

		
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
			const int sizeOfConfig = sizeof(config)+2;
			const unsigned int eeBeg = sizeOfConfig+prevTotLength+nr+((nr-1)*2);
			const unsigned int eeEnd = sizeOfConfig+(prevTotLength+sizeOfVal)+nr+1+((nr-1)*2);

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
	
	// wifi connector
	#if WIFI_MULTI_FORCE_RECONN_ANY == true
	if(WiFi.status() == WL_NO_SSID_AVAIL){
		_connected = false;
		WiFi.disconnect(false);
		delay(10);
		
		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(SER_CONN_LOST_RECONN);
		#endif

		this->WiFiConnectToAP(true);

		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(F(""));
			DEBUG_PRINTLN(FPSTR(SER_DEV));
		#endif
	}
	#endif
	
	// Synchronize the internal clock useing SNTP every SNTP_INT_CLOCK_UPD_INTERVAL
	#if SNTP_INT_CLOCK_UPD == true
		if(_connected && millis() - _lastTimeSet > SNTP_INT_CLOCK_UPD_INTERVAL){
			this->setClock();
		}
	#endif
	
	// Call home and check for updates every _callHomeInterval
	if (_connected && _callHomeInterval > 0 && millis() - _lastCallHomeTime > _callHomeInterval) {
		this->callHome();
		_lastCallHomeTime = millis();
	}
	
	// handle button presses: short, long, xlong
	this->buttonLoop();
	
	#if DEBUG_FREE_HEAP == true
		DEBUG_PRINTLN(" end of IAS::loop");
		DEBUG_PRINTF(" Free heap: %u\n", ESP.getFreeHeap());
	#endif
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
#if CFG_INCLUDE == true
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
#if CFG_INCLUDE == true	
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
void IOTAppStory::onFirmwareUpdateProgress(THandlerFunctionArg value) {
	_firmwareUpdateProgressCallback = value;
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
String IOTAppStory::servHdlRoot() {

	String retHtml;
	retHtml += FPSTR(HTTP_TEMP_START);

	if(_connected){

		retHtml.replace("{h}", FPSTR(HTTP_STA_JS));

	}else{

		retHtml.replace("{h}", FPSTR(HTTP_AP_CSS));
		retHtml += FPSTR(HTTP_WIFI_FORM);
		retHtml.replace("{r}", strWifiScan());
		retHtml += FPSTR(HTTP_AP_JS);
	}

	retHtml += FPSTR(HTTP_TEMP_END);
	return retHtml;
}



/** Handle device information */
String IOTAppStory::servHdlDevInfo(){
	#if DEBUG_LVL >= 3
		DEBUG_PRINTLN(SER_SERV_DEV_INFO);
	#endif

	String retHtml;
	retHtml += FPSTR(HTTP_DEV_INFO);
	retHtml.replace(F("{s1}"), config.ssid[0]);
	retHtml.replace(F("{s2}"), config.ssid[1]);
	retHtml.replace(F("{s3}"), config.ssid[2]);
	
	retHtml.replace(F("{cid}"), String(ESP_GETCHIPID));
	retHtml.replace(F("{fid}"), String(ESP_GETFLASHCHIPID));

	#if defined  ESP8266 && HTTPS_8266_TYPE == FNGPRINT
		retHtml.replace(F("{f}"), config.sha1);
	#endif

	retHtml.replace(F("{fss}"), String(ESP.getFreeSketchSpace()));
	retHtml.replace(F("{ss}"), String(ESP.getSketchSize()));
		
	retHtml.replace(F("{fs}"), String(ESP.getFlashChipSize()));
	retHtml.replace(F("{ab}"), ARDUINO_BOARD);
	retHtml.replace(F("{mc}"), WiFi.macAddress());
	retHtml.replace(F("{xf}"), String(_nrXF));

	if(String(config.actCode) == "000000" || String(config.actCode) == ""){
		retHtml.replace(F("{ac}"), "0");	
	}else{
		retHtml.replace(F("{ac}"), "1");	
	}
	
	return retHtml;
}



/** Handle wifi scan */
String IOTAppStory::strWifiScan(){
	
	#if DEBUG_LVL >= 3
		DEBUG_PRINTLN(SER_SERV_WIFI_SCAN_RES);
	#endif
	
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



/** Handle save wifi credentials */
String IOTAppStory::servHdlWifiSave(String newSSID, String newPass, int apNr) {
	String retHtml = F("0");
	
	// are both ssid & password sent
	if(newSSID!="" && newPass != ""){
		
		if(!apNr){
			if(_tryToConn == false){
				//Saved from first screen. When in Wifi AP mode
				if(_connFail){
					_connFail = false;
					retHtml = F("3");			// return html Failed
					
					// read config & reset credentials
					readConfig();
					
				}else if(_connected){
					_connChangeMode = true;
						
					// Saving config to eeprom
					_writeConfig = true;
					
					retHtml = F("1:");	// ok:ip
					retHtml += WiFi.localIP().toString();
					delay(100);
					
				}else if(!_connected){
					
					#if WIFI_MULTI == true
						// Shift array members to the right
						strncpy(config.ssid[2], config.ssid[1], STRUCT_CHAR_ARRAY_SIZE);
						strncpy(config.password[2], config.password[1], STRUCT_PASSWORD_SIZE);
						
						strncpy(config.ssid[1], config.ssid[0], STRUCT_CHAR_ARRAY_SIZE);
						strncpy(config.password[1], config.password[0], STRUCT_PASSWORD_SIZE);
					#endif
					
					// Replace the first array member
					newSSID.toCharArray(config.ssid[0], STRUCT_CHAR_ARRAY_SIZE);
					newPass.toCharArray(config.password[0], STRUCT_PASSWORD_SIZE);
					
					_tryToConn = true;
					retHtml = F("2");		// busy
				}
			}else{
				
				// if server is called while connecting wifi anser busy
				retHtml = F("2");		// busy
			}
		
		}else{

				//DEBUG_PRINTLN("Saved / added from config. When in Wifi STA mode");
				// Saved / added from config. When in Wifi STA mode
				_writeConfig = true;


				#if DEBUG_LVL == 3
					DEBUG_PRINTF_P(SER_CONN_ADDED_AP_CRED,apNr);
				#endif
				
				newSSID.toCharArray(config.ssid[apNr-1], STRUCT_CHAR_ARRAY_SIZE);
				newPass.toCharArray(config.password[apNr-1], STRUCT_PASSWORD_SIZE);
				retHtml = F("1");		// ok
		}

	}else{

		#if DEBUG_LVL >= 2
			DEBUG_PRINT(SER_CONN_CRED_MISSING);
		#endif
	}

	return retHtml;
}



/** Handle app / firmware information */
String IOTAppStory::servHdlAppInfo(){
	#if DEBUG_LVL >= 3
		DEBUG_PRINTLN(SER_SERV_APP_SETTINGS);
	#endif

	String retHtml = F("[");
	for (unsigned int i = 0; i < _nrXF; ++i) {

		// return html results from the wifi scan
		if(i > 0){
			retHtml += F(",");
		}
		
		// add slashed to values where necessary to prevent the json repsons from being broken
		String value = (*fieldStruct[i].varPointer);
		value.replace("\\", "\\\\");
		value.replace("\"", "\\\"");
		value.replace("\n", "\\n");
		value.replace("\r", "\\r");
		value.replace("\t", "\\t");
		value.replace("\b", "\\b");
		value.replace("\f", "\\f");

		// get PROGMEM json string and replace {*} with values
		retHtml += FPSTR(HTTP_APP_INFO);
		retHtml.replace(F("{l}"), String(fieldStruct[i].fieldLabel));
		retHtml.replace(F("{v}"), value);
		retHtml.replace(F("{n}"), String(i));
		retHtml.replace(F("{m}"), String(fieldStruct[i].length));
		retHtml.replace(F("{t}"), String(fieldStruct[i].type));
		delay(10);
	}
	retHtml += F("]");
	
	#if DEBUG_LVL >= 3
		DEBUG_PRINTLN(retHtml);
	#endif
	
	return retHtml;
}



#if defined  ESP8266 && HTTPS_8266_TYPE == FNGPRINT
/** Save new fingerprint */
String IOTAppStory::servHdlFngPrintSave(String fngprint){
	#if DEBUG_LVL >= 3
		DEBUG_PRINTLN(SER_SAVE_FINGERPRINT);
	#endif
	
	fngprint.toCharArray(config.sha1, 60);
	
	writeConfig();
	return F("1");
}
#endif


#if defined  ESP32
/** Get all root certificates */
String IOTAppStory::strCertScan(String path){
	
	#if DEBUG_LVL >= 3
		DEBUG_PRINTLN(SER_SERV_CERT_SCAN_RES);
	#endif
	
	// open SPIFFS certificate directory
    File root = SPIFFS.open("/cert");
    if(!root || !root.isDirectory()){
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(" Failed to open directory");
		#endif
		
        return "0";
    }
	
	// delete requested file
	if(path != ""){
		if(!SPIFFS.remove(path)){
			#if DEBUG_LVL >= 3
				DEBUG_PRINTLN(" Failed to delete file!");
			#endif
		}
	}
	
	// return all the files found in this directory and return them as a json string
    File file = root.openNextFile();
	String retHtml = "[";
	bool pastOne = false;
	
    while(file){
        if(!file.isDirectory()){
			if(pastOne == true){
				retHtml += F(",");
			}
			retHtml += "{";
			retHtml += "\"n\":\"" + String(file.name()) + "\"";
			retHtml += ",\"s\":" + String(file.size());
			retHtml += "}";
			pastOne = true;
        }
        file = root.openNextFile();
    }
	retHtml += "]";

	// return json string
	return retHtml;
}
#else
String IOTAppStory::strCertScan(String path){
	
	#if DEBUG_LVL >= 3
		DEBUG_PRINTLN(SER_SERV_CERT_SCAN_RES);
	#endif
	
	//Initialize File System
	if(!ESP_SPIFFSBEGIN){
		#if DEBUG_LVL >= 3
			DEBUG_PRINT(F(" SPIFFS Mount Failed"));
		#endif
	}
	
	/* <-- always fails
    // check if SPIFFS certificate directory exists
    if(!SPIFFS.exists("/cert")){  // || !root.isDirectory()
		#if DEBUG_LVL >= 2
			DEBUG_PRINTLN(F(" Failed to open directory"));
		#endif
		
        //return "0";
    }*/
	
	// open SPIFFS certificate directory
	Dir dir = SPIFFS.openDir("/cert/");
	
	// delete requested file
	if(path != ""){
		if(!SPIFFS.remove(path)){
			#if DEBUG_LVL >= 3
				DEBUG_PRINTLN(F(" Failed to delete file!"));
			#endif
		}
	}
	
	// return all the files found in this directory and return them as a json string
	String retHtml = "[";
    while(dir.next()){
		
		if(dir.fileSize()) {
			File file = dir.openFile("r");
			
			if(retHtml != "["){
				retHtml += F(",");
			}
			retHtml += "{";
			retHtml += "\"n\":\"" + String(file.name()) + "\"";
			retHtml += ",\"s\":" + String(file.size());
			retHtml += "}";
		}
    }
	retHtml += "]";

	// return json string
	return retHtml;
}
#endif


/** Save App Settings */
String IOTAppStory::servHdlAppSave(AsyncWebServerRequest *request) {
	#if DEBUG_LVL >= 3
		DEBUG_PRINTLN(SER_SAVE_APP_SETTINGS);
	#endif
	
	if(_nrXF){
		for(unsigned int i = 0; i < _nrXF; i++){
			if(request->hasParam(String(i), true)){
				strcpy((*fieldStruct[i].varPointer), request->getParam(String(i), true)->value().c_str());
			}
		}
		
		_writeConfig = true;
		return F("1");
	}
	
	return F("0");
}



/** Save activation code */
String IOTAppStory::servHdlactcodeSave(String actcode) {
	#if DEBUG_LVL >= 3
		DEBUG_PRINT(SER_REC_ACT_CODE);
		DEBUG_PRINTLN(actcode);
	#endif
	
	if(actcode != ""){
		actcode.toCharArray(config.actCode, 7);
		
		writeConfig(true);
		return F("1");
	}

	return F("0");
}
