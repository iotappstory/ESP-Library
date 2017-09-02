#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
//#include <Ticker.h>
#include <EEPROM.h>
#include <pgmspace.h>
#include <ArduinoJson.h>
#include <FS.h>
#include "IOTAppStory.h"
#include "WiFiManagerMod.h"

#ifdef REMOTEDEBUGGING
	#include <WiFiUDP.h>
#endif

// used by the RTC memory read/write functions
extern "C" {
	#include "user_interface.h"
}

IOTAppStory::IOTAppStory(const char *appName, const char *appVersion, const char *compDate, const int modeButton){
	// initiating object
	//_appName = appName;			// may not be necessary
	//_appVersion = appVersion;		// may not be necessary
	_firmware = String(appName)+" "+String(appVersion);
	_compDate = compDate;
	_modeButton = modeButton;
	readConfig();
	
	// set appName as default boardName in case the app developer does not set it
	preSetConfig((String)appName, false);
}

void IOTAppStory::firstBoot(bool ea){
	DEBUG_PRINTF(" Running first boot sequence for %s\n", _firmware.c_str());

	// THIS ONLY RUNS ON THE FIRST BOOT OF A JUST INSTALLED APP (OR AFTER RESET TO DEFAULT SETTINGS) <-----------------------------------------------------------------------------------------------------------------
	
	// get json config......
	
	// erase eeprom after config (delete extra field data etc.)
	if(ea == true){
		DEBUG_PRINTLN(" Erasing full EEPROM");
		WiFi.disconnect(true); 							// Wipe out WiFi credentials.
		eraseFlash(0,EEPROM_SIZE);						// erase full eeprom
		
		
		String emty = "000000";
		emty.toCharArray(config.devPass, 7);
		emty = "";
		emty.toCharArray(config.ssid, STRUCT_CHAR_ARRAY_SIZE);
		emty.toCharArray(config.password, STRUCT_CHAR_ARRAY_SIZE);
	}else{
		DEBUG_PRINTLN(" Erasing EEPROM but leaving config settings");
		eraseFlash((sizeof(config)+2),EEPROM_SIZE);		// erase eeprom but leave the config settings
	}
	
	// update first boot config flag (date)
	//String tmpdate = _compDate;
	String(_compDate).toCharArray(config.compDate, 20);
	writeConfig();
	
	
	DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
}

void IOTAppStory::serialdebug(bool onoff,int speed){
	_serialDebug = onoff;
	if(_serialDebug == true){
		Serial.begin(speed);
		DEBUG_PRINT("\n\n\n\n\n");
	}
}

void IOTAppStory::preSetConfig(bool automaticUpdate){
	config.automaticUpdate = automaticUpdate;
	_setPreSet = true;
}

void IOTAppStory::preSetConfig(String boardName, bool automaticUpdate /*= false*/){
	boardName.toCharArray(config.boardName, STRUCT_CHAR_ARRAY_SIZE);
	config.automaticUpdate = automaticUpdate;
	_setPreSet = true;
}

void IOTAppStory::preSetConfig(String ssid, String password, bool automaticUpdate /*= false*/){
	ssid.toCharArray(config.ssid, STRUCT_CHAR_ARRAY_SIZE);
	password.toCharArray(config.password, STRUCT_CHAR_ARRAY_SIZE);
	config.automaticUpdate = automaticUpdate;
	_setPreSet = true;
}

void IOTAppStory::preSetConfig(String ssid, String password, String boardName, bool automaticUpdate /*= false*/){
	preSetConfig(ssid, password, automaticUpdate);
	boardName.toCharArray(config.boardName, STRUCT_CHAR_ARRAY_SIZE);
	_setPreSet = true;
}

void IOTAppStory::preSetConfig(String ssid, String password, String boardName, String IOTappStory1, String IOTappStoryPHP1, bool automaticUpdate /*= false*/) {
	preSetConfig(ssid, password, boardName, automaticUpdate);
	IOTappStory1.toCharArray(config.IOTappStory1, STRUCT_HOST_SIZE);
	IOTappStoryPHP1.toCharArray(config.IOTappStoryPHP1, STRUCT_FILE_SIZE);
	_setPreSet = true;
}

void IOTAppStory::begin(bool bootstats, bool ea){
	DEBUG_PRINTLN("\n");

	if(_setPreSet == true){
		writeConfig();
		DEBUG_PRINTLN("Saving config presets...\n");
	}

	DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
	DEBUG_PRINTF(" Start %s\n", _firmware.c_str());
	DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
	DEBUG_PRINTF(" Mode select button: GPIO%d\n", _modeButton);
	DEBUG_PRINTF(" Boardname: %s\n", config.boardName);
	DEBUG_PRINTF(" Automatic update: %d\n", config.automaticUpdate);
	DEBUG_PRINTLN("*-------------------------------------------------------------------------*");

	// ----------- PINS ----------------
	pinMode(_modeButton, INPUT_PULLUP);     		// MODEBUTTON as input for Config mode selection

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
	readConfig();
	

	// --------- START WIFI --------------------------
	connectNetwork();


	// --------- if automaticUpdate Update --------------------------
	if(config.automaticUpdate == true){
		callHome();
	}
	buttonEntry = millis() + ENTER_CONFIG_MODE_TIME_MAX;    // make sure the timedifference during startup is bigger than 10 sec. Otherwise it will go either in config mode or calls home

	// ----------- END SPECIFIC SETUP CODE ----------------------------
	DEBUG_PRINT("\n\n\n\n\n");
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
	DEBUG_PRINTLN(" rtcMem");
	DEBUG_PRINTF(" markerFlag: %d\n", rtcMem.markerFlag);
	DEBUG_PRINTF(" bootTimes since powerup: %d\n", rtcMem.bootTimes);
	DEBUG_PRINTF(" boardMode: %c\n", rtcMem.boardMode);
	DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
}

void IOTAppStory::configESP() {
	readConfig();
	//connectNetwork();

	DEBUG_PRINT("\n\n\n\n");
	DEBUG_PRINTLN("C O N F I G U R A T I O N    M O D E");

	initWiFiManager();

	//--------------- buttonbuttonLoop ----------------------------------
	while (1) {
		//if ((*buttonChanged) && (*buttonTime) > 4000) espRestart('N', "Back to normal mode");  // long button press > 4sec
		yield();
		loopWiFiManager();
	}
} 


void IOTAppStory::connectNetwork() {
	DEBUG_PRINTLN(" Connecting to WiFi AP");

	WiFi.mode(WIFI_STA);
	if (!isNetworkConnected()) {
		DEBUG_PRINTLN("");
		DEBUG_PRINTLN(" No Connection. Try to connect with saved PW");

		WiFi.begin(config.ssid, config.password);  // if password forgotten by firmwware try again with stored PW
		if (!isNetworkConnected()) espRestart('C', "Going into Configuration Mode"); // still no success
	}
	DEBUG_PRINTLN(" ");
	DEBUG_PRINTLN(" WiFi connected");

	
	//getMACaddress();
	//printMacAddress();

	DEBUG_PRINT(" Device MAC: ");
	DEBUG_PRINTLN(WiFi.macAddress());

	DEBUG_PRINT(" Device IP Address: ");
	DEBUG_PRINTLN(WiFi.localIP());

	// Register host name in WiFi and mDNS
	String hostNameWifi = config.boardName;   // boardName is device name
	hostNameWifi.concat(".local");
	wifi_station_set_hostname(config.boardName);
	//   WiFi.hostname(hostNameWifi);
	if (MDNS.begin(config.boardName)) {
		DEBUG_PRINT(" MDNS responder started: http://");
		DEBUG_PRINT(hostNameWifi);
		DEBUG_PRINTLN("");
		DEBUG_PRINTLN("");
		DEBUG_PRINTLN(" To use MDNS Install host software:");
		DEBUG_PRINTLN(" - For Linux, install Avahi (http://avahi.org/)");
		DEBUG_PRINTLN(" - For Windows, install Bonjour (https://commaster.net/content/how-resolve-multicast-dns-windows)");
		DEBUG_PRINTLN(" - For Mac OSX and iOS support is built in through Bonjour already");
		DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
	} else {
		espRestart('N', "MDNS not started");
	}
}

// Wait until network is connected. Returns false if not connected after MAX_WIFI_RETRIES retries
bool IOTAppStory::isNetworkConnected() {
	int retries = MAX_WIFI_RETRIES;
	DEBUG_PRINT(" ");
	while (WiFi.status() != WL_CONNECTED && retries-- > 0 ) {
		delay(500);
		DEBUG_PRINT(".");
	}
	
	return (retries > 0);
}


//---------- IOTappStory FUNCTIONS ----------
bool IOTAppStory::callHome(bool spiffs /*= true*/) {
	// update from IOTappStory.com
	bool updateHappened=false;
	byte res1, res2;

	DEBUG_PRINTLN(" Calling Home");
	DEBUG_PRINTF(" Current App: %s\n\n", _firmware.c_str());

	ESPhttpUpdate.rebootOnUpdate(false);
	res1 = iotUpdater(0,config.IOTappStory1, config.IOTappStoryPHP1);
	
	if (res1 == 'F') {
		// if address 1 was unsuccesfull try address 2
		res2 = iotUpdater(0,config.IOTappStory2, config.IOTappStoryPHP2) ;
	}
	if (res1 == 'U' || res2 == 'U')  updateHappened = true;
	


	if (spiffs) {
		DEBUG_PRINTLN("");
		res1 = iotUpdater(1,config.IOTappStory1, config.IOTappStoryPHP1);
		if (res1 == 'F') {
			// if address 1 was unsuccesfull try address 2
			res2 = iotUpdater(1,config.IOTappStory2, config.IOTappStoryPHP2);
		}
	} 
	if (res1 == 'U' || res2 == 'U')  updateHappened = true;

	DEBUG_PRINTLN("");
	DEBUG_PRINTLN(" Returning from IOTAppStory.com");
	DEBUG_PRINTLN("*-------------------------------------------------------------------------*");

	
	
	if (updateHappened) {
		// set boardMode to normal and reboot
		rtcMem.boardMode = 'N';
		ESP.restart();
	}
	return updateHappened;
}

byte IOTAppStory::iotUpdater(bool type, String server, String url) {
	byte retValue;

	DEBUG_PRINT(" Checking for ");
	if(type == 0){
		// type == sketch
		DEBUG_PRINT("App(Sketch)");
	}
	if(type == 1){
		// type == spiffs
		DEBUG_PRINT("SPIFFS");
	}
	DEBUG_PRINT(" updates from: ");
	DEBUG_PRINT(server);
	DEBUG_PRINTLN(url);
	
	t_httpUpdate_return ret;
	if(type == 0){
		// type == sketch
		ret = ESPhttpUpdate.update(server, 80, url, _firmware);
	}
	if(type == 1){
		// type == spiffs
		ret = ESPhttpUpdate.updateSpiffs("http://" + String(server + url), _firmware);
	}
	
	switch (ret) {
		case HTTP_UPDATE_FAILED:
			if(_serialDebug == true){
				Serial.printf(" Update Failed. Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
			}
			retValue = 'F';
		break;
		case HTTP_UPDATE_NO_UPDATES:
			DEBUG_PRINTLN(" No updates");
			retValue = 'A';
		break;
		case HTTP_UPDATE_OK:
			DEBUG_PRINTLN(" Received update");
			retValue = 'U';
		break;
	}
	return retValue;
}


//---------- WIFIMANAGER COMMON FUNCTIONS
void IOTAppStory::initWiFiManager() {
	if(_serialDebug == true){WiFi.printDiag(Serial);} //Remove this line if you do not want to see WiFi password printed

	if (WiFi.SSID() == "") {
		DEBUG_PRINTLN("We haven't got any access point credentials, so get them now");
	}else{
		WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
		unsigned long startedAt = millis();
		int connRes = WiFi.waitForConnectResult();
		float waited = (millis() - startedAt);

		DEBUG_PRINTF("After waiting %d secs in setup(), the connection result is %d\n", waited / 1000, connRes);
	}

	if (WiFi.status() != WL_CONNECTED) {
		DEBUG_PRINTLN("Failed to connect");
	}else{
		DEBUG_PRINT("Local ip: ");
		DEBUG_PRINTLN(WiFi.localIP());
	}
	
}


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
		DEBUG_PRINTLN(" Processing added fields");
		DEBUG_PRINTF(" ID | LABEL                          | LEN |  EEPROM LOC  | DEFAULT VALUE                  | CURRENT VALUE                  | STATUS\n");

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

			DEBUG_PRINTF(" %02d | %-30s | %03d | %04d to %04d | %-30s | ", nr, fieldStruct[nr-1].fieldLabel, fieldStruct[nr-1].length-1, eeBeg, eeEnd, (*fieldStruct[nr-1].varPointer));

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
					DEBUG_PRINTF("%-30s | OVERWRITTEN", eepVal);

					(*fieldStruct[nr-1].varPointer) = eepVal;
				}else{
					DEBUG_PRINTF("%-30s | DEFAULT", (*fieldStruct[nr-1].varPointer));
					//DEBUG_PRINTLN((*fieldStruct[nr-1].varPointer));
				}

			}else{
				//DEBUG_PRINTLN(" NO existing EEPROM value found.");
				DEBUG_PRINTF("%-30s | WRITTING TO EEPROM", (*fieldStruct[nr-1].varPointer));

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
			DEBUG_PRINTLN("");
		}
		EEPROM.end();
		DEBUG_PRINTLN("*-------------------------------------------------------------------------*");
	}
}
int IOTAppStory::dPinConv(String orgVal){
	#ifdef ARDUINO_ESP8266_ESP01  // Generic ESP's 

		//Serial.println("- Generic ESP's -");
		if      (orgVal == "D0"  || orgVal == "16")   return 16;
		else if (orgVal == "D1"  || orgVal == "5")    return 5;
		else if (orgVal == "D2"  || orgVal == "4")    return 4;
		else if (orgVal == "D4"  || orgVal == "2")    return 2;
		else if (orgVal == "D5"  || orgVal == "14")   return 14;
		else if (orgVal == "D6"  || orgVal == "12")   return 12;
		else if (orgVal == "D7"  || orgVal == "13")   return 13;
		else if (orgVal == "D8"  || orgVal == "15")   return 15;
		else if (orgVal == "D9"  || orgVal == "3")    return 3;
		else if (orgVal == "D10" || orgVal == "1")    return 1;
		else                                          return 16;

	#else

		//Serial.println("- Special ESP's -");
		if      (orgVal == "D0"  || orgVal == "16")   return D0;
		else if (orgVal == "D1"  || orgVal == "5")    return D1;
		else if (orgVal == "D2"  || orgVal == "4")    return D2;
		else if (orgVal == "D4"  || orgVal == "2")    return D4;
		else if (orgVal == "D5"  || orgVal == "14")   return D5;
		else if (orgVal == "D6"  || orgVal == "12")   return D6;
		else if (orgVal == "D7"  || orgVal == "13")   return D7;
		else if (orgVal == "D8"  || orgVal == "15")   return D7;
		else if (orgVal == "D9"  || orgVal == "3")    return D7;
		else if (orgVal == "D10" || orgVal == "1")    return D7;
		else                                          return D0;

	#endif
}

void IOTAppStory::loopWiFiManager() {
	WiFiManagerParameter parArray[MAXNUMEXTRAFIELDS];
	
	for(unsigned int i = 0; i < _nrXF; i++){
		// add the WiFiManagerParameter to parArray so it can be referenced to later
		parArray[i] = WiFiManagerParameter(fieldStruct[i].fieldIdName, fieldStruct[i].fieldLabel, (*fieldStruct[i].varPointer), fieldStruct[i].length);
	}

	// Initialize WiFIManager
	WiFiManager wifiManager;
	wifiManager.devPass = String(config.devPass);
	//wifiManager.addParameter(&p_hint);

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
		DEBUG_PRINTLN(" Not connected to WiFi but continuing anyway.");
	}else{
		// If you get here you have connected to the WiFi
		DEBUG_PRINTLN(" Connected... :-)");
	}
	// Getting posted form values and overriding local variables parameters
	// Config file is written

	DEBUG_PRINTLN(" ---------------------------");

	//add all parameters here
	for(unsigned int i = 0; i < _nrXF; i++){
		strcpy((*fieldStruct[i].varPointer), parArray[i].getValue());
	}
	wifiManager.devPass.toCharArray(config.devPass,7);
	//DEBUG_PRINTLN(wifiManager.devPass);
	
	writeConfig(true);
	readConfig();  // read back to fill all variables
	espRestart('N', "Configuration finished"); //Normal Operation
}

//---------- MISC FUNCTIONS ----------
void IOTAppStory::espRestart(char mmode, char* message) {
	while (digitalRead(_modeButton) == LOW) yield();    // wait till GPIOo released
	delay(500);
	
	rtcMem.boardMode = mmode;
	writeRTCmem();
	//system_rtc_mem_write(RTCMEMBEGIN + 100, &mmode, 1);
	//system_rtc_mem_read(RTCMEMBEGIN + 100, &boardMode, 1);

	DEBUG_PRINTLN("");
	DEBUG_PRINTLN(message);

	ESP.restart();
}

void IOTAppStory::eraseFlash(unsigned int eepFrom, unsigned int eepTo) {
	DEBUG_PRINTLN(" Erasing Flash...");
	DEBUG_PRINTF(" From %4d to %4d\n", eepFrom, eepTo);

	EEPROM.begin(EEPROM_SIZE);
	for (unsigned int t = eepFrom; t < eepTo; t++) EEPROM.write(t, 0);
	EEPROM.end();
}

//---------- CONFIGURATION PARAMETERS ----------
void IOTAppStory::writeConfig(bool wifiSave) {
	//DEBUG_PRINTLN(" ------------------ Writing Config --------------------------------");
	if (WiFi.psk() != "") {
		WiFi.SSID().toCharArray(config.ssid, STRUCT_CHAR_ARRAY_SIZE);
		WiFi.psk().toCharArray(config.password, STRUCT_CHAR_ARRAY_SIZE);
		//DEBUG_PRINT("Stored ");
		//DEBUG_PRINT(config.ssid);
		//DEBUG_PRINTLN("  ");
		//DEBUG_PRINTLN(config.password);   // devPass
	}
	
	
	
	EEPROM.begin(EEPROM_SIZE);
	config.magicBytes[0] = MAGICBYTES[0];
	config.magicBytes[1] = MAGICBYTES[1];
	config.magicBytes[2] = MAGICBYTES[2];

	// WRITE CONFIG TO EEPROM
	for (unsigned int t = 0; t < sizeof(config); t++) EEPROM.write(t, *((char*)&config + t));
	EEPROM.commit();
	
	if(wifiSave == true && _nrXF > 0){
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
	DEBUG_PRINTLN(" Reading Config");
	boolean ret = false;
	EEPROM.begin(EEPROM_SIZE);
	long magicBytesBegin = sizeof(config) - 4; 								// Magic bytes at the end of the structure

	if (EEPROM.read(magicBytesBegin) == MAGICBYTES[0] && EEPROM.read(magicBytesBegin + 1) == MAGICBYTES[1] && EEPROM.read(magicBytesBegin + 2) == MAGICBYTES[2]) {
		DEBUG_PRINTLN(" EEPROM Configuration found");
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
		DEBUG_PRINTLN(" EEPROM Configuration NOT FOUND!!!!");
		writeConfig();
		ret = false;
	}
	return ret;
}


void IOTAppStory::buttonLoop() {
  unsigned long _buttonTime = -1;
  // pinMode(_modeButton, INPUT_PULLUP);     		// MODEBUTTON as input for Config mode selection
  
  int _buttonState = digitalRead(_modeButton);
  yield(); 
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
    if (_buttonTime >= ENTER_CONFIG_MODE_TIME_MIN && _buttonTime < ENTER_CONFIG_MODE_TIME_MAX)
    	espRestart('C', "Going into Configuration Mode");     // long button press > 4sec
    if (_buttonTime >= ENTER_CHECK_FIRMWARE_TIME_MIN && _buttonTime < ENTER_CHECK_FIRMWARE_TIME_MAX) 
	    callHome();         // long button press > 1sec
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
