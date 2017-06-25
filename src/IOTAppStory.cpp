#ifndef IOTAppStory_h
	#include "IOTAppStory.h"
#endif

IOTAppStory::IOTAppStory(const char *appName, const char *appVersion, const char *compDate, const int modeButton){
	// initiating object
	_appName = appName;
	_appVersion = appVersion;
	_firmware = String(appName)+String(appVersion);
	_compDate = compDate;
	_modeButton = modeButton;
	readFullConfiguration();
}

void IOTAppStory::firstBoot(bool ea){
	if(_serialDebug == true){
		DEBUG_PRINT(" Running first boot sequence for ");
		DEBUG_PRINTLN(_firmware);
	}
	// THIS ONLY RUNS ON THE FIRST BOOT OF A JUST INSTALLED APP (OR AFTER RESET TO DEFAULT SETTINGS) <-----------------------------------------------------------------------------------------------------------------
	
	// get json config......
	
	// erase eeprom after config (delete extra field data etc.)
	if(ea == true){
		WiFi.disconnect(true); 							// Wipe out WiFi credentials.
		eraseFlash(0,EEPROM_SIZE);						// erase full eeprom
		
		
		String emty = "000000";
		emty.toCharArray(config.devPass, 7);
		emty = "";
		emty.toCharArray(config.ssid, STRUCT_CHAR_ARRAY_SIZE);
		emty.toCharArray(config.password, STRUCT_CHAR_ARRAY_SIZE);
	}else{
		eraseFlash((sizeof(config)+2),EEPROM_SIZE);		// erase eeprom but leave the config settings
	}
	
	// update first boot config flag (date)
	//String tmpdate = _compDate;
	String(_compDate).toCharArray(config.compDate, 20);
	writeConfig();
	
	
	if(_serialDebug == true){
		DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
	}
}

void IOTAppStory::serialdebug(bool onoff,int speed){
	_serialDebug = onoff;
	if(_serialDebug == true){
		Serial.begin(speed);
		for (int i = 0; i < 5; i++) DEBUG_PRINTLN(" ");
	}
}

void IOTAppStory::preSetConfig(String boardName){
    boardName.toCharArray(config.boardName, STRUCT_CHAR_ARRAY_SIZE);
}
void IOTAppStory::preSetConfig(String boardName, bool automaticUpdate){
	boardName.toCharArray(config.boardName, STRUCT_CHAR_ARRAY_SIZE);
	config.automaticUpdate = automaticUpdate;
	_setPreSet = true;
}
void IOTAppStory::preSetConfig(String ssid, String password){
	ssid.toCharArray(config.ssid, STRUCT_CHAR_ARRAY_SIZE);
	password.toCharArray(config.password, STRUCT_CHAR_ARRAY_SIZE);
	_setPreSet = true;
}
void IOTAppStory::preSetConfig(String ssid, String password, bool automaticUpdate){
	preSetConfig(ssid,password);
	config.automaticUpdate = automaticUpdate;
	_setPreSet = true;
}
void IOTAppStory::preSetConfig(String ssid, String password, String boardName){
	preSetConfig(ssid,password);
	boardName.toCharArray(config.boardName, STRUCT_CHAR_ARRAY_SIZE);
	_setPreSet = true;
}
void IOTAppStory::preSetConfig(String ssid, String password, String boardName, bool automaticUpdate){
	preSetConfig(ssid,password,boardName);
	config.automaticUpdate = automaticUpdate;
	_setPreSet = true;
}
void IOTAppStory::preSetConfig(String ssid, String password, String boardName, String IOTappStory1, String IOTappStoryPHP1, bool automaticUpdate){
	preSetConfig(ssid,password,boardName);
	IOTappStory1.toCharArray(config.IOTappStory1, STRUCT_HOST_SIZE);
	IOTappStoryPHP1.toCharArray(config.IOTappStoryPHP1, STRUCT_FILE_SIZE);
	config.automaticUpdate = automaticUpdate;
	_setPreSet = true;
}


void IOTAppStory::begin(int feedBackLed, bool bootstats, bool ea){
	if(_serialDebug == true){
		DEBUG_PRINTLN("");
		DEBUG_PRINTLN("");
	}
	if(_setPreSet == true){
		writeConfig();
		if(_serialDebug == true){
			DEBUG_PRINTLN("Saving config presets...");
			DEBUG_PRINTLN("");
		}
	}
	
	if(_serialDebug == true){
		DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
		DEBUG_PRINT(" Start ");
		DEBUG_PRINTLN(_firmware);
		DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
		DEBUG_PRINT(" Mode select button: Gpio");
		DEBUG_PRINTLN(_modeButton);
		DEBUG_PRINT(" Boardname: ");
		DEBUG_PRINTLN(config.boardName);
		DEBUG_PRINT(" Automatic update: ");
		DEBUG_PRINTLN(config.automaticUpdate);
		DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
	}

	// ----------- PINS ----------------
	pinMode(_modeButton, INPUT_PULLUP);     		// MODEBUTTON as input for Config mode selection
	if(feedBackLed > -1){
		//pinMode(feedBackLed, OUTPUT);         	// LEDgreen feedback
		//digitalWrite(feedBackLed, LEDOFF);
	}
//	attachInterrupt(_modeButton, ptr, CHANGE);

	//------------- LED and DISPLAYS ------------------------
	//LEDswitch(GreenBlink);

	// Read the "bootTime" & "boardMode" flag RTC memory
	readRTCmem();
	
	// --------- BOOT STATISTICS ------------------------
	// read and increase boot statistics (optional)
	if(bootstats == true){
		rtcMem.bootTimes++;
		writeRTCmem();
		if(_serialDebug == true){
			printRTCmem();
		}
	}

	// on first boot of the app run the firstBoot() function
	if(String(config.compDate) != String(_compDate)){
		firstBoot(ea);
	}

	// process added fields
	processField();
	
	
	//---------- SELECT BOARD MODE -----------------------------
	if (rtcMem.boardMode == 'C') configESP();

	// --------- READ FULL CONFIG --------------------------
	readFullConfiguration();
	


	// --------- START WIFI --------------------------
	connectNetwork();

	//sendSysLogMessage(2, 1, config.boardName, _firmware, 10, counter++, "------------- Normal Mode -------------------");

	// --------- if automaticUpdate Update --------------------------
	if(config.automaticUpdate == true){
		callHome();
	}
	buttonEntry = millis()+10000;    // make sure the timedifference during startup is bigger than 10 sec. Otherwise it will go either in config mode or calls home

	// ----------- END SPECIFIC SETUP CODE ----------------------------
	// LEDswitch(None);
	// sendSysLogMessage(7, 1, config.boardName, _firmware, 10, counter++, "Setup done");
	if(_serialDebug == true){
		DEBUG_PRINTLN("");
		DEBUG_PRINTLN("");
		DEBUG_PRINTLN("");
		DEBUG_PRINTLN("");
		DEBUG_PRINTLN("");
	}
}


//---------- RTC MEMORY FUNCTIONS ----------
bool IOTAppStory::readRTCmem() {
	bool ret = true;
	system_rtc_mem_read(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
	if (rtcMem.markerFlag != MAGICBYTE) {
		rtcMem.markerFlag = MAGICBYTE;
		rtcMem.bootTimes = 0;
		rtcMem.boardMode = 'N';
		system_rtc_mem_write(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
		ret = false;
	}
	return ret;
}

void IOTAppStory::writeRTCmem() {
	rtcMem.markerFlag = MAGICBYTE;
	system_rtc_mem_write(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
}

void IOTAppStory::printRTCmem() {
	if(_serialDebug == true){
		DEBUG_PRINTLN(" rtcMem");
		DEBUG_PRINT(" markerFlag: ");
		DEBUG_PRINTLN(rtcMem.markerFlag);
		DEBUG_PRINT(" bootTimes since powerup: ");
		DEBUG_PRINTLN(rtcMem.bootTimes);
		DEBUG_PRINT(" boardMode: ");
		DEBUG_PRINTLN(rtcMem.boardMode);
		DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
	}
}

void IOTAppStory::configESP() {
	// ----------- PINS ----------------
	/*
	#ifdef LEDgreen
	  pinMode(LEDgreen, OUTPUT);
	  digitalWrite(LEDgreen, LEDOFF);
	#endif
	#ifdef LEDred
	  pinMode(LEDred, OUTPUT);
	  digitalWrite(LEDred, LEDOFF);
	#endif
	*/

	// ------------- INTERRUPTS ----------------------------
	//blink.detach();

	//------------- LED and DISPLAYS ------------------------
	//LEDswitch(GreenFastBlink);

	readFullConfiguration();
	//connectNetwork();

	if(_serialDebug == true){
		for (int i = 0; i < 4; i++) DEBUG_PRINTLN("");
		DEBUG_PRINTLN("C O N F I G U R A T I O N    M O D E");
	}
	//sendSysLogMessage(6, 1, config.boardName, _firmware, 10, counter++, "------------- Configuration Mode -------------------");
	initWiFiManager();

	//--------------- LOOP ----------------------------------
	while (1) {
		//if ((*buttonChanged) && (*buttonTime) > 4000) espRestart('N', "Back to normal mode");  // long button press > 4sec
		yield();
		loopWiFiManager();
	}
} 

void IOTAppStory::readFullConfiguration() {
	readConfig();  // configuration in EEPROM
	if(_serialDebug == true){
		DEBUG_PRINTLN(" Exit config");
		DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
	}
}

void IOTAppStory::connectNetwork() {
	if(_serialDebug == true){
		DEBUG_PRINTLN(" Connecting to WiFi AP");
	}
	WiFi.mode(WIFI_STA);
	if (!isNetworkConnected()) {
		if(_serialDebug == true){
			DEBUG_PRINTLN("");
			DEBUG_PRINTLN(" No Connection. Try to connect with saved PW");
		}
		WiFi.begin(config.ssid, config.password);  // if password forgotten by firmwware try again with stored PW
		if (!isNetworkConnected()) espRestart('C', "Going into Configuration Mode"); // still no success
	}
	if(_serialDebug == true){
		DEBUG_PRINTLN(" ");
		DEBUG_PRINTLN(" WiFi connected");
	}
	
	getMACaddress();
	printMacAddress();
	
	if(_serialDebug == true){
		DEBUG_PRINT(" Device IP Address: ");
		DEBUG_PRINTLN(WiFi.localIP());
	}

	// Register host name in WiFi and mDNS
	String hostNameWifi = config.boardName;   // boardName is device name
	hostNameWifi.concat(".local");
	wifi_station_set_hostname(config.boardName);
	//   WiFi.hostname(hostNameWifi);
	if (MDNS.begin(config.boardName)) {
		if(_serialDebug == true){
			DEBUG_PRINT(" MDNS responder started: http://");
			DEBUG_PRINT(hostNameWifi);
			DEBUG_PRINTLN("");
			DEBUG_PRINTLN("");
			DEBUG_PRINTLN(" To use MDNS Install host software:");
			DEBUG_PRINTLN(" - For Linux, install Avahi (http://avahi.org/)");
			DEBUG_PRINTLN(" - For Windows, install Bonjour (https://commaster.net/content/how-resolve-multicast-dns-windows)");
			DEBUG_PRINTLN(" - For Mac OSX and iOS support is built in through Bonjour already");
			DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
		}
	} else espRestart('N', "MDNS not started");
}

// Wait till networl is connected. Returns false if not connected after MAX_WIFI_RETRIES retries
bool IOTAppStory::isNetworkConnected() {
	int retries = MAX_WIFI_RETRIES;
	DEBUG_PRINT(" ");
	while (WiFi.status() != WL_CONNECTED && retries-- > 0 ) {
		delay(500);
		DEBUG_PRINT(".");
	}
	
	if (retries <= 0){
		return false;
	}else{
		return true;
	}
}

String IOTAppStory::getMACaddress() {
	uint8_t mac[6];
	char macStr[18] = {0};
	WiFi.macAddress(mac);
	sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],  mac[1], mac[2], mac[3], mac[4], mac[5]);
	return  String(macStr);
}

void IOTAppStory::printMacAddress() {
	byte mac[6];
	WiFi.macAddress(mac);
	DEBUG_PRINT(" Device MAC: ");
	for (int i = 0; i < 5; i++) {
		Serial.print(mac[i], HEX);
		DEBUG_PRINT(":");
	}
	if(_serialDebug == true){
		Serial.println(mac[5], HEX);
	}
}

//---------- IOTappStory FUNCTIONS ----------
bool IOTAppStory::callHome(bool spiffs) {
	// update from IOTappStory.com
	bool updateHappened=false;
	byte res1, res2;

	//sendSysLogMessage(7, 1, config.boardName, _firmware, 10, counter++, "------------- IOTappStory -------------------");
	//LEDswitch(GreenSlowBlink);

	getMACaddress();

	ESPhttpUpdate.rebootOnUpdate(false);
	res1 = iotUpdaterSketch(config.IOTappStory1, config.IOTappStoryPHP1, _firmware, true);
	
	if (res1 == 'F') {
		String message = String(config.IOTappStory1) + ": Update not succesful";
		//sendSysLogMessage(2, 1, config.boardName, _firmware, 10, counter++, message);
		res2 = iotUpdaterSketch(config.IOTappStory2, config.IOTappStoryPHP2, _firmware, true) ;
		if (res2 == 'F') {
			message = String(config.IOTappStory2) + ": Update not succesful";
			//sendSysLogMessage(2, 1, config.boardName, _firmware, 10, counter++, message);
		} 
	}
	if (res1 == 'U' || res2 == 'U')  updateHappened = true;
	


	if (spiffs) {
		if(_serialDebug == true){
			DEBUG_PRINTLN("");
		}
		res1 = iotUpdaterSPIFFS(config.IOTappStory1, config.IOTappStoryPHP1, _firmware, true);
		if (res1 == 'F') {
			String message = String(config.IOTappStory1) + ": Update not succesful";
			//sendSysLogMessage(2, 1, config.boardName, _firmware, 10, counter++, message);
			res2 = iotUpdaterSPIFFS(config.IOTappStory2, config.IOTappStoryPHP2, _firmware, true);
			if (res2 == 'F') {
				message = String(config.IOTappStory2) + ": Update not succesful";
				//sendSysLogMessage(2, 1, config.boardName, _firmware, 10, counter++, message);
			}
		}
	} 
	if (res1 == 'U' || res2 == 'U')  updateHappened = true;

	if(_serialDebug == true){
		DEBUG_PRINTLN("");
		DEBUG_PRINTLN(" Returning from IOTAppStory.com");
		DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
	}
	
//	(*buttonTime) = 0;
//	(*buttonChanged) = false;
	
	if (updateHappened) {
		//initialize();
		rtcMem.boardMode = 'N';
		ESP.restart();
	}
	return updateHappened;
}

bool IOTAppStory::callHome() {
	return callHome(true);
}
/*
void IOTAppStory::initialize() {   // this function is called by callHome() before return. Here, you put a safe startup configuration

}
*/
byte IOTAppStory::iotUpdaterSketch(String server, String url, String firmware, bool immediately) {
	byte retValue;
	
	if(_serialDebug == true){
		DEBUG_PRINTLN(" Calling Home");
		DEBUG_PRINT(" Current App: ");
		DEBUG_PRINTLN(firmware);
		DEBUG_PRINTLN("");
		
		DEBUG_PRINT(" Checking for App(Sketch) updates from: ");
		DEBUG_PRINT(server);
		DEBUG_PRINTLN(url);
	}
	
	t_httpUpdate_return ret = ESPhttpUpdate.update(server, 80, url, firmware);
	switch (ret) {
		case HTTP_UPDATE_FAILED:
			if(_serialDebug == true){
				Serial.printf(" Update Failed. Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
			}
			retValue = 'F';
		break;
		case HTTP_UPDATE_NO_UPDATES:
			if(_serialDebug == true){DEBUG_PRINTLN(" No updates");}
			retValue = 'A';
		break;
		case HTTP_UPDATE_OK:
			if(_serialDebug == true){DEBUG_PRINTLN(" Received update");}
			retValue = 'U';
		break;
	}
	return retValue;
}

byte IOTAppStory::iotUpdaterSPIFFS(String server, String url, String firmware, bool immediately) {
	byte retValue;
	if(_serialDebug == true){
		DEBUG_PRINT(" Checking for SPIFFS updates from: ");
		DEBUG_PRINT(server);
		DEBUG_PRINTLN(url);
	}

	t_httpUpdate_return retspiffs = ESPhttpUpdate.updateSpiffs("http://" + String(server + url), firmware);
	switch (retspiffs) {
		case HTTP_UPDATE_FAILED:
			if(_serialDebug == true){
				Serial.printf(" SPIFFS Update Failed. Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
				DEBUG_PRINTLN();
			}
			retValue = 'F';
		break;
		case HTTP_UPDATE_NO_UPDATES:
			if(_serialDebug == true){DEBUG_PRINTLN(" SPIFFS No updates");}
			retValue = 'A';
		break;
		case HTTP_UPDATE_OK:
			if(_serialDebug == true){DEBUG_PRINTLN(" SPIFFS Received update");}
			retValue = 'U';
		break;
	}
	return retValue;
}

//---------- WIFIMANAGER COMMON FUNCTIONS
void IOTAppStory::initWiFiManager() {
	if(_serialDebug == true){WiFi.printDiag(Serial);} //Remove this line if you do not want to see WiFi password printed

	if (WiFi.SSID() == "") {
		if(_serialDebug == true){DEBUG_PRINTLN("We haven't got any access point credentials, so get them now");}
	}else{
		WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
		unsigned long startedAt = millis();
		if(_serialDebug == true){DEBUG_PRINT("After waiting ");}
		int connRes = WiFi.waitForConnectResult();
		float waited = (millis() - startedAt);
		if(_serialDebug == true){
			DEBUG_PRINT(waited / 1000);
			DEBUG_PRINT("secs in setup() connection result is ");
			DEBUG_PRINTLN(connRes);
		}
	}

	if (WiFi.status() != WL_CONNECTED) {
		if(_serialDebug == true){DEBUG_PRINTLN("Failed to connect");}
	}else{
		if(_serialDebug == true){
			DEBUG_PRINT("Local ip: ");
			DEBUG_PRINTLN(WiFi.localIP());
		}
	}
	
}

/*
// add int field to the wifi configuration page and add value to eeprom
void IOTAppStory::addField(int &defaultVal,const char *fieldIdName,const char *fieldLabel, int length){
	_nrXF++;
}
*/

// add char array field to the wifi configuration page and add value to eeprom
void IOTAppStory::addField(char* &defaultVal,const char *fieldIdName,const char *fieldLabel, int length){
	_nrXF++;
	
	// add values to the fieldstruct
	fieldStruct[_nrXF-1].fieldIdName = fieldIdName;
	fieldStruct[_nrXF-1].fieldLabel = fieldLabel;
	fieldStruct[_nrXF-1].varPointer = &defaultVal;
	fieldStruct[_nrXF-1].length = length+1;
}

// add char array field to the wifi configuration page and add value to eeprom
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
			String(tempValue[nr]).toCharArray((*fieldStruct[nr].varPointer), fieldStruct[nr].length);
		}
	}
		
	if(_nrXF > 0){
		if(_serialDebug == true){DEBUG_PRINTLN(" Processing added fields");}

		EEPROM.begin(EEPROM_SIZE);
		
		for (unsigned int nr = 1; nr <= _nrXF; nr++){
			delay(100);
			// loop threw the fields struc array	
			
			int prevTotLength = 0;
			for(unsigned int i = 0; i < (nr-1); i++){
				prevTotLength += fieldStruct[i].length;
			}
			const int sizeOfVal = fieldStruct[nr-1].length;
			const int sizeOfConfig = sizeof(config)+2;
			const int eeBeg = sizeOfConfig+prevTotLength+nr+((nr-1)*2);
			const int eeEnd = sizeOfConfig+(prevTotLength+sizeOfVal)+nr+1+((nr-1)*2);
			
			if(_serialDebug == true){

				DEBUG_PRINT(" ");
				DEBUG_PRINT(nr);
				DEBUG_PRINT(" | ");
				DEBUG_PRINT(fieldStruct[nr-1].fieldLabel);
				DEBUG_PRINT(" | Default value: ");
				DEBUG_PRINT((*fieldStruct[nr-1].varPointer));
				DEBUG_PRINT(" | Max. length: ");
				DEBUG_PRINT(fieldStruct[nr-1].length-1);		
				DEBUG_PRINT(" | EEPROM loc: ");
				DEBUG_PRINT(eeBeg);
				DEBUG_PRINT(" to ");
				DEBUG_PRINT(eeEnd);
			}

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
						//DEBUG_PRINTLN(EEPROM.read(t));
						*((char*)eepVal + (t-eeBeg)-1) = EEPROM.read(t);
					}
				}
				
				// if eeprom value is different update the ret value
				if(String(eepVal) != String((*fieldStruct[nr-1].varPointer))){
					if(_serialDebug == true){
						DEBUG_PRINT(" | Overwriting default with eeprom value: ");
						DEBUG_PRINT(eepVal);
					}
					(*fieldStruct[nr-1].varPointer) = eepVal;
				}else{
					if(_serialDebug == true){
						DEBUG_PRINT(" | Unchainged, keeping default value");
						//DEBUG_PRINTLN((*fieldStruct[nr-1].varPointer));
					}
				}
				
			}else{
				
				if(_serialDebug == true){
					//DEBUG_PRINTLN(" NO existing EEPROM value found.");
					DEBUG_PRINT(" | Writing default value to EEPROM.");
				}
				
				// add MAGICEEP to value and write to eeprom
				for (unsigned int t = eeBeg; t <= eeEnd; t++){
					//DEBUG_PRINT(t);
					//DEBUG_PRINT("  | ");
					if(t == eeBeg){
						EEPROM.put(t, MAGICEEP[0]);						// magic begin marker
						//DEBUG_PRINT(MAGICEEP[0]);
					}else  if(t == eeEnd){
						EEPROM.put(t, '^');								// magic end marker
						//DEBUG_PRINT('^');
					}else{
						EEPROM.put(t, *((char*)tmpVal + (t-eeBeg)-1));	// byte of value`
						//DEBUG_PRINT(*((char*)tmpVal + (t-eeBeg)-1));
					}
					//DEBUG_PRINTLN(" ");
				}
				
				//DEBUG_PRINTLN(" ------------");
			}
		
			// add values to the fieldstruct
			//fieldStruct[_nrXF-1].varPointer = (*fieldStruct[nr-1].varPointer);
			if(_serialDebug == true){
				DEBUG_PRINTLN("");
			}		
		}
		EEPROM.end();
		if(_serialDebug == true){
			DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
		}
	}
}
int IOTAppStory::dPinConv(String orgVal){
	#ifdef ARDUINO_ESP8266_ESP01  // Generic ESP's 

		//Serial.println("- Generic ESP's -");
		if (orgVal == "D0" || orgVal == "16"){
			return 16;
		}else if (orgVal == "D1" || orgVal == "5"){
			return 5;
		}else if (orgVal == "D2" || orgVal == "4"){
			return 4;
		}else if (orgVal == "D4" || orgVal == "2"){
			return 2;
		}else if (orgVal == "D5" || orgVal == "14"){
			return 14;
		}else if (orgVal == "D6" || orgVal == "12"){
			return 12;
		}else if (orgVal == "D7" || orgVal == "13"){
			return 13;
		}else if (orgVal == "D8" || orgVal == "15"){
			return 15;
		}else if (orgVal == "D9" || orgVal == "3"){
			return 3;
		}else if (orgVal == "D10" || orgVal == "1"){
			return 1;
		}else{
			return 16;
		}

	#else

		//Serial.println("- Special ESP's -");
		if (orgVal == "D0" || orgVal == "16"){
			return D0;
		}else if (orgVal == "D1" || orgVal == "5"){
			return D1;
		}else if (orgVal == "D2" || orgVal == "4"){
			return D2;
		}else if (orgVal == "D4" || orgVal == "2"){
			return D4;
		}else if (orgVal == "D5" || orgVal == "14"){
			return D5;
		}else if (orgVal == "D6" || orgVal == "12"){
			return D6;
		}else if (orgVal == "D7" || orgVal == "13"){
			return D7;
		}else if (orgVal == "D8" || orgVal == "15"){
			return D7;
		}else if (orgVal == "D9" || orgVal == "3"){
			return D7;
		}else if (orgVal == "D10" || orgVal == "1"){
			return D7;
		}else{
			return D0;
		}

	#endif
}

void IOTAppStory::loopWiFiManager() {
	for(unsigned int i = 0; i < _nrXF; i++){
		
		// add the WiFiManagerParameter to parArray so it can be referenced to later
		parArray[i] = WiFiManagerParameter(fieldStruct[i].fieldIdName, fieldStruct[i].fieldLabel, (*fieldStruct[i].varPointer), fieldStruct[i].length);
	}
	
	// Just a quick hint
	WiFiManagerParameter p_hint("<small>*Hint: if you want to reuse the currently active WiFi credentials, leave SSID and Password fields empty</small></br></br>");

	// Initialize WiFIManager
	WiFiManager wifiManager;
	wifiManager.devPass = String(config.devPass);
	wifiManager.addParameter(&p_hint);

	//add all parameters here
	for(unsigned int i = 0; i < _nrXF; i++){
		wifiManager.addParameter(&parArray[i]);
	}

	// Sets timeout in seconds until configuration portal gets turned off.
	// If not specified device will remain in configuration mode until
	// switched off via webserver or device is restarted.
	wifiManager.setConfigPortalTimeout(1200);

	// It starts an access point
	// and goes into a blocking loop awaiting configuration.
	// Once the user leaves the portal with the exit button
	// processing will continue
	if (!wifiManager.startConfigPortal(config.boardName)) {
		if(_serialDebug == true){DEBUG_PRINTLN(" Not connected to WiFi but continuing anyway.");}
	}else{
		// If you get here you have connected to the WiFi
		if(_serialDebug == true){DEBUG_PRINTLN(" Connected... :-)");}
	}
	// Getting posted form values and overriding local variables parameters
	// Config file is written

	if(_serialDebug == true){DEBUG_PRINTLN(" ---------------------------");}
	
	//add all parameters here
	for(unsigned int i = 0; i < _nrXF; i++){
		strcpy((*fieldStruct[i].varPointer), parArray[i].getValue());
	}
	wifiManager.devPass.toCharArray(config.devPass,7);
	//DEBUG_PRINTLN(wifiManager.devPass);
	
	writeConfig(true);
	readFullConfiguration();  // read back to fill all variables
	//LEDswitch(None); // Turn LED off as we are not in configuration mode.
	espRestart('N', "Configuration finished"); //Normal Operation
}

//---------- MISC FUNCTIONS ----------
void IOTAppStory::espRestart(char mmode, char* message) {
	//LEDswitch(GreenFastBlink);
	while (digitalRead(_modeButton) == LOW) yield();    // wait till GPIOo released
	delay(500);
	
	rtcMem.boardMode = mmode;
	writeRTCmem();
	//system_rtc_mem_write(RTCMEMBEGIN + 100, &mmode, 1);
	//system_rtc_mem_read(RTCMEMBEGIN + 100, &boardMode, 1);
	
	if(_serialDebug == true){
		DEBUG_PRINTLN("");
		DEBUG_PRINTLN(message);
	}
	
	ESP.restart();
}

void IOTAppStory::eraseFlash(unsigned int eepFrom, unsigned int eepTo) {
	if(_serialDebug == true){
		DEBUG_PRINTLN(" Erasing Flash...");
		DEBUG_PRINT(" From ");
		DEBUG_PRINT(eepFrom);
		DEBUG_PRINT(" To ");
		DEBUG_PRINT(eepTo);
		DEBUG_PRINTLN(" ");
	}
	EEPROM.begin(EEPROM_SIZE);
	for (unsigned int t = eepFrom; t < eepTo; t++) EEPROM.write(t, 0);
	EEPROM.end();
}

//---------- CONFIGURATION PARAMETERS ----------
void IOTAppStory::writeConfig(bool wifiSave) {
	//if(_serialDebug == true){DEBUG_PRINTLN(" ------------------ Writing Config --------------------------------");}
	if (WiFi.psk() != "") {
		WiFi.SSID().toCharArray(config.ssid, STRUCT_CHAR_ARRAY_SIZE);
		WiFi.psk().toCharArray(config.password, STRUCT_CHAR_ARRAY_SIZE);
		/*
		if(_serialDebug == true){
			DEBUG_PRINT("Stored ");
			DEBUG_PRINT(config.ssid);
			DEBUG_PRINTLN("  ");
			//   DEBUG_PRINTLN(config.password);   devPass
		}
		*/
	}
	
	
	
	EEPROM.begin(EEPROM_SIZE);
	config.magicBytes[0] = MAGICBYTES[0];
	config.magicBytes[1] = MAGICBYTES[1];
	config.magicBytes[2] = MAGICBYTES[2];

	// WRITE CONFIG TO EEPROM
	for (unsigned int t = 0; t < sizeof(config); t++) EEPROM.write(t, *((char*)&config + t));
	EEPROM.commit();
	
	if(wifiSave == true && _nrXF > 0){
		// LOOP THREW ALL THE ADDED FIELDS, CHECK VALUES AND IF NECESSARY WRITE TO EEPROM
		for (unsigned int nr = 1; nr <= _nrXF; nr++){
			
			int prevTotLength = 0;
			for(unsigned int i = 0; i < (nr-1); i++){
				prevTotLength += fieldStruct[i].length;
			}
			const int sizeOfVal = fieldStruct[nr-1].length;
			const int sizeOfConfig = sizeof(config)+2;
			const int eeBeg = sizeOfConfig+prevTotLength+nr+((nr-1)*2);
			const int eeEnd = sizeOfConfig+(prevTotLength+sizeOfVal)+nr+1+((nr-1)*2);
			
			/*
			DEBUG_PRINT(" EEPROM space: ");
			DEBUG_PRINT(eeBeg);
			DEBUG_PRINT("  to ");
			DEBUG_PRINTLN(eeEnd);
			DEBUG_PRINTLN(" ");
			DEBUG_PRINT(" Size: ");
			DEBUG_PRINTLN(sizeOfVal);
			DEBUG_PRINTLN((*fieldStruct[nr-1].varPointer));
			*/
			
			char* tmpVal = new char[sizeOfVal + 1];
			for (int i = 0; i <= sizeOfVal; i++) {
				tmpVal[i] = 0;
			}
			if ((*fieldStruct[nr-1].varPointer) != NULL) {
				strncpy(tmpVal, (*fieldStruct[nr-1].varPointer), fieldStruct[nr-1].length);
			}

			//DEBUG_PRINTLN("^^");
			//DEBUG_PRINTLN(tmpVal);
			//DEBUG_PRINTLN(" ");
			
			// check for MAGICEEP
			if(EEPROM.read(eeBeg) == MAGICEEP[0] && EEPROM.read(eeEnd) == '^'){
				// add MAGICEEP to value and write to eeprom
				for (unsigned int t = eeBeg; t <= eeEnd; t++){
					if(t == eeBeg){
						EEPROM.put(t, MAGICEEP[0]);
						//DEBUG_PRINTLN(MAGICEEP[0]);
					}else if(t == eeEnd){
						EEPROM.put(t, '^');
						//DEBUG_PRINTLN('^');
					}else{
						EEPROM.put(t, *((char*)tmpVal + (t-eeBeg)-1));
						//DEBUG_PRINTLN(*((char*)tmpVal + (t-eeBeg)-1));
					}
				}
			}
			EEPROM.commit();
		}
	}
	
	EEPROM.end();
}

bool IOTAppStory::readConfig() {
	if(_serialDebug == true){DEBUG_PRINTLN(" Reading Config");}
	boolean ret = false;
	EEPROM.begin(EEPROM_SIZE);
	long magicBytesBegin = sizeof(config) - 4; 								// Magic bytes at the end of the structure

	if (EEPROM.read(magicBytesBegin) == MAGICBYTES[0] && EEPROM.read(magicBytesBegin + 1) == MAGICBYTES[1] && EEPROM.read(magicBytesBegin + 2) == MAGICBYTES[2]) {
		if(_serialDebug == true){DEBUG_PRINTLN(" EEPROM Configuration found");}
		for (unsigned int t = 0; t < sizeof(config); t++) *((char*)&config + t) = EEPROM.read(t);
		EEPROM.end();
		
		// Standard											// Is this part necessary? Maby for ram usage it is better to load this from eeprom only when needed....!?
		//boardName = String(config.boardName);
		//IOTappStory1 = String(config.IOTappStory1);
		//IOTappStoryPHP1 = String(config.IOTappStoryPHP1);
		//IOTappStory2 = String(config.IOTappStory2);
		//IOTappStoryPHP2 = String(config.IOTappStoryPHP2);
		ret = true;

	} else {
		if(_serialDebug == true){DEBUG_PRINTLN(" EEPROM Configurarion NOT FOUND!!!!");}
		writeConfig();
		//LEDswitch(RedFastBlink);
		ret = false;
	}
	return ret;
}


void IOTAppStory::routine() {
  unsigned long _buttonTime = -1;
  //pinMode(_modeButton, INPUT_PULLUP);     		// MODEBUTTON as input for Config mode selection
  
  int _buttonState = digitalRead(_modeButton);
    
  if (buttonStateOld != _buttonState) {
    Serial.println("* button changed *");
    delay(100);
    
    if (_buttonState == 0) {
      buttonEntry = millis();
    } else {
        _buttonTime = millis() - buttonEntry;
        buttonEntry = millis();
    }
    /*
	if (_serialDebug == true) {
       Serial.print("Time ");
       Serial.println(_buttonTime);
    }
*/
    if (_buttonTime > 4000 && _buttonTime < 10000) espRestart('C', "Going into Configuration Mode");     // long button press > 4sec
    if (_buttonTime >  500 && _buttonTime < 4000) callHome();         // long button press > 1sec
  }
  if (_serialDebug == true && millis() - debugEntry > 5000) {
     debugEntry = millis();
     //DEBUG_PRINT("inside loop()... ");
     //DEBUG_PRINT("Heap ");
     //DEBUG_PRINTLN(ESP.getFreeHeap());
     //sendDebugMessage();
  }
  buttonStateOld = _buttonState;
}

void IOTAppStory::saveConfigCallback () {        								// <<-- could be deleted ?
	writeConfig();
}
/*
void IOTAppStory::sendDebugMessage() {
	// ------- Syslog Message --------

	/* severity: 2 critical, 6 info, 7 debug
	facility: 1 user level
	String hostName: Board Name
	app: FIRMWARE
	procID: unddefined
	msgID: counter
	message: Your message
	

	sysMessage = "";
	long h1 = ESP.getFreeHeap();
	sysMessage += " Heap ";
	sysMessage += h1;
	//sendSysLogMessage(6, 1, config.boardName, FIRMWARE, 10, counter++, sysMessage);
}
*/
