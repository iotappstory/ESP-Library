#ifndef IOTAppStory_h
	#include "IOTAppStory.h"
#endif

IOTAppStory::IOTAppStory(String appName, String appVersion, int modeButton){
	// initiating object
	_appName = appName;
	_appVersion = appVersion;
	_firmware = appName+appVersion;
	_modeButton = modeButton;
}

void IOTAppStory::serialdebug(bool onoff,int speed){
	_serialDebug = onoff;
	if(_serialDebug == true){
		Serial.begin(speed);
		for (int i = 0; i < 5; i++) Serial.println("");
	}
}


void IOTAppStory::preSetConfig(String boardName){
	boardName.toCharArray(config.boardName, STRUCT_CHAR_ARRAY_SIZE);
}
void IOTAppStory::preSetConfig(String ssid, String password){
	ssid.toCharArray(config.ssid, STRUCT_CHAR_ARRAY_SIZE);
	password.toCharArray(config.password, STRUCT_CHAR_ARRAY_SIZE);
}
void IOTAppStory::preSetConfig(String ssid, String password, bool automaticUpdate){
	preSetConfig(ssid,password);
	if(automaticUpdate == true){
		strcpy(config.automaticUpdate,"1");
	}else{
		strcpy(config.automaticUpdate,"0");
	}
}
void IOTAppStory::preSetConfig(String ssid, String password, String boardName){
	preSetConfig(ssid,password);
	boardName.toCharArray(config.boardName, STRUCT_CHAR_ARRAY_SIZE);
}
void IOTAppStory::preSetConfig(String ssid, String password, String boardName, bool automaticUpdate){
	preSetConfig(ssid,password,boardName);
	if(automaticUpdate == true){
		strcpy(config.automaticUpdate,"1");
	}else{
		strcpy(config.automaticUpdate,"0");
	}
}
void IOTAppStory::preSetConfig(String ssid, String password, String boardName, String IOTappStory1, String IOTappStoryPHP1, bool automaticUpdate){
	preSetConfig(ssid,password,boardName);
	IOTappStory1.toCharArray(config.IOTappStory1, STRUCT_CHAR_ARRAY_SIZE);
	IOTappStoryPHP1.toCharArray(config.IOTappStoryPHP1, STRUCT_CHAR_ARRAY_SIZE);
	if(automaticUpdate == true){
		strcpy(config.automaticUpdate,"1");
	}else{
		strcpy(config.automaticUpdate,"0");
	}
}


void IOTAppStory::begin(void(*ptr)(), int feedBackLed, bool bootstats){ 
	if(_serialDebug == true){
		Serial.println("*----------------------------------------------------------------------*");
		Serial.print("Start ");
		Serial.println(_firmware);
		Serial.println("*----------------------------------------------------------------------*");
		Serial.print("Mode select button: Gpio");
		Serial.println(_modeButton);
		Serial.println(config.boardName);
		Serial.println("*----------------------------------------------------------------------*");
	}

	// ----------- PINS ----------------
	pinMode(_modeButton, INPUT_PULLUP);     // MODEBUTTON as input for Config mode selection

	if(feedBackLed > -1){
		//pinMode(feedBackLed, OUTPUT);         // LEDgreen feedback
		//digitalWrite(feedBackLed, LEDOFF);
	}
	attachInterrupt(_modeButton, ptr, CHANGE);

	//------------- LED and DISPLAYS ------------------------
	//LEDswitch(GreenBlink);

	// --------- BOOT STATISTICS ------------------------
	// read and increase boot statistics (optional)
	if(bootstats == true){
		readRTCmem();
		rtcMem.bootTimes++;
		writeRTCmem();
		if(_serialDebug == true){
			printRTCmem();
		}
	}

	//---------- SELECT BOARD MODE -----------------------------
	system_rtc_mem_read(RTCMEMBEGIN + 100, &boardMode, 1);   // Read the "boardMode" flag RTC memory to decide, if to go to config
	if (boardMode == 'C') configESP();

	// --------- READ FULL CONFIG --------------------------
	readFullConfiguration();

	// --------- START WIFI --------------------------
	connectNetwork();

	//sendSysLogMessage(2, 1, config.boardName, _firmware, 10, counter++, "------------- Normal Mode -------------------");

	// --------- if automaticUpdate Update --------------------------
	if (atoi(config.automaticUpdate) == 1){callHome();}

	// ----------- END SPECIFIC SETUP CODE ----------------------------
	// LEDswitch(None);
	// sendSysLogMessage(7, 1, config.boardName, _firmware, 10, counter++, "Setup done");
}

//---------- RTC MEMORY FUNCTIONS ----------
bool IOTAppStory::readRTCmem() {
	bool ret = true;
	system_rtc_mem_read(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
	if (rtcMem.markerFlag != MAGICBYTE) {
		rtcMem.markerFlag = MAGICBYTE;
		rtcMem.bootTimes = 0;
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
	DEBUG_PRINTLN("");
	DEBUG_PRINTLN("rtcMem ");
	DEBUG_PRINT("markerFlag ");
	DEBUG_PRINTLN(rtcMem.markerFlag);
	DEBUG_PRINT("bootTimes ");
	DEBUG_PRINTLN(rtcMem.bootTimes);
}

void IOTAppStory::configESP() {
	//Serial.begin(115200);
	for (int i = 0; i < 5; i++) DEBUG_PRINTLN("");

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
		DEBUG_PRINTLN("------------- Configuration Mode -------------------");
	}
	//sendSysLogMessage(6, 1, config.boardName, _firmware, 10, counter++, "------------- Configuration Mode -------------------");
	initWiFiManager();

	//--------------- LOOP ----------------------------------
	while (1) {
		if (buttonChanged && buttonTime > 4000) espRestart('N', "Back to normal mode");  // long button press > 4sec
		yield();
		loopWiFiManager();
	}
}

void IOTAppStory::readFullConfiguration() {
	readConfig();  // configuration in EEPROM
	if(_serialDebug == true){
		DEBUG_PRINTLN("Exit config");
	}
}

void IOTAppStory::connectNetwork() {
	WiFi.mode(WIFI_STA);
	if (!isNetworkConnected()) {
		if(_serialDebug == true){
			DEBUG_PRINTLN("");
			DEBUG_PRINTLN("No Connection. Try to connect with saved PW");
		}
		WiFi.begin(config.ssid, config.password);  // if password forgotten by firmwware try again with stored PW
		if (!isNetworkConnected()) espRestart('C', "Going into Configuration Mode"); // still no success
	}
	if(_serialDebug == true){
		DEBUG_PRINTLN("");
		DEBUG_PRINTLN("WiFi connected");
	}
	
	getMACaddress();
	printMacAddress();
	
	if(_serialDebug == true){
		DEBUG_PRINT("IP Address: ");
		DEBUG_PRINTLN(WiFi.localIP());
	}

	// Register host name in WiFi and mDNS
	String hostNameWifi = config.boardName;   // boardName is device name
	hostNameWifi.concat(".local");
	wifi_station_set_hostname(config.boardName);
	//   WiFi.hostname(hostNameWifi);
	if (MDNS.begin(config.boardName)) {
		if(_serialDebug == true){
			DEBUG_PRINT("* MDNS responder started. http://");
			DEBUG_PRINTLN(hostNameWifi);
		}
	} else espRestart('N', "MDNS not started");
}

// Wait till networl is connected. Returns false if not connected after MAX_WIFI_RETRIES retries
bool IOTAppStory::isNetworkConnected() {
	int retries = MAX_WIFI_RETRIES;
	while (WiFi.status() != WL_CONNECTED && retries-- > 0 ) {
		delay(500);
		Serial.print(".");
	}
	if (retries <= 0) return false;
	else return true;
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
	Serial.print("MAC: ");
	for (int i = 0; i < 5; i++) {
		Serial.print(mac[i], HEX);
		Serial.print(":");
	}
	Serial.println(mac[5], HEX);
}

//---------- IOTappStory FUNCTIONS ----------
void IOTAppStory::callHome(bool spiffs) {
	// update from IOTappStory.com
	bool updateHappened=false;
	byte res1, res2;

	//sendSysLogMessage(7, 1, config.boardName, _firmware, 10, counter++, "------------- IOTappStory -------------------");
	//LEDswitch(GreenSlowBlink);

	getMACaddress();
	printMacAddress();
	DEBUG_PRINT("IP = ");
	DEBUG_PRINTLN(WiFi.localIP());
	DEBUG_PRINTLN("");
	DEBUG_PRINTLN("");

	ESPhttpUpdate.rebootOnUpdate(false);
	res1 = iotUpdaterSketch(config.IOTappStory1, config.IOTappStoryPHP1, _firmware, true);
	
	if (res1 == 'F') {
		String message = IOTappStory1 + ": Update not succesful";
		//sendSysLogMessage(2, 1, config.boardName, _firmware, 10, counter++, message);
		res2 = iotUpdaterSketch(config.IOTappStory2, config.IOTappStoryPHP2, _firmware, true) ;
		if (res2 == 'F') {
			message = IOTappStory2 + ": Update not succesful";
			//sendSysLogMessage(2, 1, config.boardName, _firmware, 10, counter++, message);
		} 
	}
	if (res1 == 'U' || res2 == 'U')  updateHappened = true;

	DEBUG_PRINTLN("");
	DEBUG_PRINTLN("");

	if (spiffs) {
		res1 = iotUpdaterSPIFFS(config.IOTappStory1, config.IOTappStoryPHP1, _firmware, true);
		if (res1 == 'F') {
			String message = IOTappStory1 + ": Update not succesful";
			//sendSysLogMessage(2, 1, config.boardName, _firmware, 10, counter++, message);
			res2 = iotUpdaterSPIFFS(config.IOTappStory2, config.IOTappStoryPHP2, _firmware, true);
			if (res2 == 'F') {
				message = IOTappStory2 + ": Update not succesful";
				//sendSysLogMessage(2, 1, config.boardName, _firmware, 10, counter++, message);
			}
		}
	} 
	if (res1 == 'U' || res2 == 'U')  updateHappened = true;

	DEBUG_PRINTLN("Returning from IOTAppstory");
	DEBUG_PRINTLN("");

	if (updateHappened) {
		initialize();
		boardMode = 'N';
		ESP.restart();
	}
}

void IOTAppStory::callHome() {
	callHome(true);
}

void IOTAppStory::initialize() {   // this function is called by callHome() before return. Here, you put a safe startup configuration

}

byte IOTAppStory::iotUpdaterSketch(String server, String url, String firmware, bool immediately) {
	byte retValue;
	DEBUG_PRINTLN("Updating Sketch from...");
	DEBUG_PRINT("Update_server ");
	DEBUG_PRINTLN(server);
	DEBUG_PRINT("UPDATE_URL ");
	DEBUG_PRINTLN(url);
	DEBUG_PRINT("FIRMWARE_VERSION ");
	DEBUG_PRINTLN(firmware);
	
	t_httpUpdate_return ret = ESPhttpUpdate.update(server, 80, url, firmware);
	switch (ret) {
		case HTTP_UPDATE_FAILED:
			#ifdef SERIALDEBUG
				Serial.printf("SKETCH_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
			#endif
			DEBUG_PRINTLN();
			retValue = 'F';
		break;
		case HTTP_UPDATE_NO_UPDATES:
			DEBUG_PRINTLN("---------- SKETCH_UPDATE_NO_UPDATES ------------------");
			retValue = 'A';
		break;
		case HTTP_UPDATE_OK:
			DEBUG_PRINTLN("SKETCH_UPDATE_OK");
			retValue = 'U';
		break;
	}
	return retValue;
}

byte IOTAppStory::iotUpdaterSPIFFS(String server, String url, String firmware, bool immediately) {
	byte retValue;
	DEBUG_PRINTLN("Updating SPIFFS from...");
	DEBUG_PRINT("Update_server ");
	DEBUG_PRINTLN(server);
	DEBUG_PRINT("UPDATE_URL ");
	DEBUG_PRINTLN(url);
	DEBUG_PRINT("FIRMWARE_VERSION ");
	DEBUG_PRINTLN(firmware);

	t_httpUpdate_return retspiffs = ESPhttpUpdate.updateSpiffs("http://" + String(server + url), firmware);
	switch (retspiffs) {
		case HTTP_UPDATE_FAILED:
			#ifdef SERIALDEBUG
				Serial.printf("SPIFFS_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
			#endif
			DEBUG_PRINTLN();
			retValue = 'F';
		break;
		case HTTP_UPDATE_NO_UPDATES:
			DEBUG_PRINTLN("---------- SPIFFS_UPDATE_NO_UPDATES ------------------");
			retValue = 'A';
		break;
		case HTTP_UPDATE_OK:
			DEBUG_PRINTLN("SPIFFS_UPDATE_OK");
			retValue = 'U';
		break;
	}
	return retValue;
}

//---------- WIFIMANAGER COMMON FUNCTIONS
void IOTAppStory::initWiFiManager() {
	WiFi.printDiag(Serial); //Remove this line if you do not want to see WiFi password printed

	if (WiFi.SSID() == "") {
		DEBUG_PRINTLN("We haven't got any access point credentials, so get them now");
	}else{
		WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
		unsigned long startedAt = millis();
		DEBUG_PRINT("After waiting ");
		int connRes = WiFi.waitForConnectResult();
		float waited = (millis() - startedAt);
		DEBUG_PRINT(waited / 1000);
		DEBUG_PRINT(" secs in setup() connection result is ");
		DEBUG_PRINTLN(connRes);
	}

	if (WiFi.status() != WL_CONNECTED) {
		DEBUG_PRINTLN("Failed to connect");
	}else{
		DEBUG_PRINT("Local ip: ");
		DEBUG_PRINTLN(WiFi.localIP());
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
	char* eepVal;
	
	EEPROM.begin(EEPROM_SIZE);
	const int sizeOfVal = length-1;
	const int sizeOfConfig = sizeof(config)+1;
	const int eeBeg = sizeOfConfig+((_nrXF-1) * sizeOfVal)+_nrXF;
	const int eeEnd = sizeOfConfig+(_nrXF * sizeOfVal)+_nrXF+1;

	// check for MAGICEEP
	if(EEPROM.read(eeBeg) == MAGICEEP[0] && EEPROM.read(length) == '\0'){
		// read eeprom
		for (unsigned int t = eeBeg; t <= eeEnd; t++){
			// start after MAGICEEP
			if(t != eeBeg){
				*((char*)&eepVal + (t-eeBeg)-1) = EEPROM.read(t);
			}
		}

		// if eeprom value is different update the ret value
		if(eepVal != defaultVal){
			defaultVal = eepVal;
		}
	}else{
		// add MAGICEEP to value and write to eeprom
		for (unsigned int t = eeBeg; t <= eeEnd; t++){
			if(t == eeBeg){
				EEPROM.put(t, MAGICEEP);
			}else{
				EEPROM.put(t, *((char*)&defaultVal + (t-eeBeg)-1));
			}
		}
	}
	EEPROM.end();
	
	fieldStruct[_nrXF-1].fieldIdName = fieldIdName;
	fieldStruct[_nrXF-1].fieldLabel = fieldLabel;
	fieldStruct[_nrXF-1].varPointer = defaultVal;
	fieldStruct[_nrXF-1].length = length;
	//Serial.println(fieldStruct[_nrXF-1].varPointer);
}

void IOTAppStory::loopWiFiManager() {
	WiFiManagerParameter parArray[_nrXF-1];
	unsigned int i;
	for(i = 0; i < _nrXF; i++){
		// add the WiFiManagerParameter to parArray so it can be referenced to later
		parArray[i] = WiFiManagerParameter(fieldStruct[i].fieldIdName, fieldStruct[i].fieldLabel, fieldStruct[i].varPointer, fieldStruct[i].length);

		/*
		Serial.println(fieldStruct[i].fieldIdName);
		Serial.println(fieldStruct[i].fieldLabel);
		Serial.println(fieldStruct[i].varPointer);
		Serial.println(fieldStruct[i].length);
		Serial.println("----/ parArray[] /---");
		Serial.println("");
		Serial.println("");
		*/
	}

	// Just a quick hint
	WiFiManagerParameter p_hint("<small>*Hint: if you want to reuse the currently active WiFi credentials, leave SSID and Password fields empty</small></br></br>");

	// Initialize WiFIManager
	WiFiManager wifiManager;
	wifiManager.addParameter(&p_hint);

	//add all parameters here
	for(i = 0; i < _nrXF; i++){
		wifiManager.addParameter(&parArray[i]);
	}

	// Sets timeout in seconds until configuration portal gets turned off.
	// If not specified device will remain in configuration mode until
	// switched off via webserver or device is restarted.
	wifiManager.setConfigPortalTimeout(600);

	// It starts an access point
	// and goes into a blocking loop awaiting configuration.
	// Once the user leaves the portal with the exit button
	// processing will continue
	if (!wifiManager.startConfigPortal(config.boardName)) {
		DEBUG_PRINTLN("Not connected to WiFi but continuing anyway.");
	}else{
		// If you get here you have connected to the WiFi
		DEBUG_PRINTLN("Connected... :-)");
	}
	// Getting posted form values and overriding local variables parameters
	// Config file is written

	//add all parameters here
	for(i = 0; i < _nrXF; i++){
		strcpy(fieldStruct[i].varPointer, parArray[0].getValue());
	}

	writeConfig();
	readFullConfiguration();  // read back to fill all variables
	//LEDswitch(None); // Turn LED off as we are not in configuration mode.
	espRestart('N', "Configuration finished"); //Normal Operation
}

//---------- MISC FUNCTIONS ----------
void IOTAppStory::espRestart(char mmode, char* message) {
	//LEDswitch(GreenFastBlink);
	DEBUG_PRINTLN(message);
	while (digitalRead(_modeButton) == LOW) yield();    // wait till GPIOo released
	delay(500);
	system_rtc_mem_write(RTCMEMBEGIN + 100, &mmode, 1);
	system_rtc_mem_read(RTCMEMBEGIN + 100, &boardMode, 1);
	ESP.restart();
}

void IOTAppStory::eraseFlash() {
	Serial.println("Erasing Flash...");
	EEPROM.begin(EEPROM_SIZE);
	for (unsigned int t = 0; t < EEPROM_SIZE; t++) EEPROM.write(t, 0);
	EEPROM.end();
}

//---------- CONFIGURATION PARAMETERS ----------
void IOTAppStory::writeConfig() {
	DEBUG_PRINTLN("------------------ Writing Config --------------------------------");
	if (WiFi.psk() != "") {
		WiFi.SSID().toCharArray(config.ssid, STRUCT_CHAR_ARRAY_SIZE);
		WiFi.psk().toCharArray(config.password, STRUCT_CHAR_ARRAY_SIZE);
		DEBUG_PRINT("Stored ");
		DEBUG_PRINT(config.ssid);
		DEBUG_PRINTLN(" ");
		//   DEBUG_PRINTLN(config.password);
	}

	EEPROM.begin(EEPROM_SIZE);
	config.magicBytes[0] = MAGICBYTES[0];
	config.magicBytes[1] = MAGICBYTES[1];
	config.magicBytes[2] = MAGICBYTES[2];

	for (unsigned int t = 0; t < sizeof(config); t++) EEPROM.write(t, *((char*)&config + t));
	EEPROM.end();
}

bool IOTAppStory::readConfig() {
	DEBUG_PRINTLN("Reading Config");
	boolean ret = false;
	EEPROM.begin(EEPROM_SIZE);
	long magicBytesBegin = sizeof(config) - 4; 													// Magic bytes at the end of the structure

	if (EEPROM.read(magicBytesBegin) == MAGICBYTES[0] && EEPROM.read(magicBytesBegin + 1) == MAGICBYTES[1] && EEPROM.read(magicBytesBegin + 2) == MAGICBYTES[2]) {
		DEBUG_PRINTLN("EEPROM Configuration found");
		for (unsigned int t = 0; t < sizeof(config); t++) *((char*)&config + t) = EEPROM.read(t);
		EEPROM.end();

		// Standard																					//	Is this part necessary? Maby for ram usage it is better to load this from eeprom only when needed....!?
		boardName = String(config.boardName);
		IOTappStory1 = String(config.IOTappStory1);
		IOTappStoryPHP1 = String(config.IOTappStoryPHP1);
		IOTappStory2 = String(config.IOTappStory2);
		IOTappStoryPHP2 = String(config.IOTappStoryPHP2);

		ret = true;

	} else {
		DEBUG_PRINTLN("EEPROM Configurarion NOT FOUND!!!!");
		writeConfig();
		//LEDswitch(RedFastBlink);
		ret = false;
	}
	return ret;
}

void IOTAppStory::routine(volatile unsigned long org_buttonEntry, unsigned long org_buttonTime, volatile bool org_buttonChanged) {
	buttonEntry = org_buttonEntry;
	buttonTime = org_buttonTime;
	buttonChanged = org_buttonChanged;

	if (buttonChanged && buttonTime > 3000) espRestart('C', "Going into Configuration Mode");  		// long button press > 4sec
	if (buttonChanged && buttonTime > 500 && buttonTime < 4000) callHome(); 						// long button press > 1sec
	buttonChanged = false;

	if(_serialDebug == true){
		if (millis() - debugEntry > 5000) { 														// Non-Blocking second counter
			debugEntry = millis();
			Serial.println("running...");
			sendDebugMessage();
		}
	}
}

void IOTAppStory::JSONerror(String err) {
	DEBUG_PRINTLN(err);
	DEBUG_PRINTLN("Restoring default values");
	writeConfig();
	//LEDswitch(RedFastBlink);
}

void IOTAppStory::saveConfigCallback () {        													// <<-- could be deleted ?
	writeConfig();
}

void IOTAppStory::sendDebugMessage() {
	// ------- Syslog Message --------

	/* severity: 2 critical, 6 info, 7 debug
	facility: 1 user level
	String hostName: Board Name
	app: FIRMWARE
	procID: unddefined
	msgID: counter
	message: Your message
	*/

	sysMessage = "";
	long h1 = ESP.getFreeHeap();
	sysMessage += " Heap ";
	sysMessage += h1;
	//sendSysLogMessage(6, 1, config.boardName, FIRMWARE, 10, counter++, sysMessage);
}
