/*                          =======================
============================   C/C++ SOURCE FILE   ============================
                            =======================                       *//**
  IOTAppStory.cpp

  Created by Onno Dirkzwager on 24.03.2017.
  Copyright (c) 2017 IOTAppStory. All rights reserved.

*///===========================================================================

/*---------------------------------------------------------------------------*/
/*                                INCLUDES                                   */
/*---------------------------------------------------------------------------*/

#include "IOTAppStory.h"

/*---------------------------------------------------------------------------*/
/*                        DEFINITIONS AND MACROS                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        TYPEDEFS AND STRUCTURES                            */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                                PROTOTYPES                                 */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                            LOCAL VARIABLES                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        FUNCTION IMPLEMENTATION                            */
/*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                        IOTAppStory constructor

*///---------------------------------------------------------------------------
IOTAppStory::IOTAppStory(const char *compDate, const int modeButton)
: _compDate(compDate), _modeButton(modeButton) {
    #if DEBUG_LVL >= 1
        Serial.begin(SERIAL_SPEED);
        while (!Serial){
            delay(10);
        }
        DEBUG_PRINT(F("\n\n\n\n\n"));
    #endif
}

/*-----------------------------------------------------------------------------
                        IOTAppStory firstBoot

    THIS ONLY RUNS ON THE FIRST BOOT OF A JUST INSTALLED APP (OR AFTER RESET TO
    DEFAULT SETTINGS)

*///---------------------------------------------------------------------------
void IOTAppStory::firstBoot() {

    #if DEBUG_LVL >= 2
        DEBUG_PRINTLN(F(" First boot sequence..."));
    #endif

    if (this->_firstBootCallback) {

        #if DEBUG_LVL >= 3
            DEBUG_PRINTLN(SER_CALLBACK_FIRST_BOOT);
        #endif
        this->_firstBootCallback();
        delay(100);
    }

    // get config from EEPROM
    ConfigStruct config;
    this->readConfig(config);

    // reset boardMode & bootTimes
    this->boardMode = 'N';
    this->bootTimes = 0;
    BoardInfo boardInfo(this->bootTimes, this->boardMode);
    boardInfo.write();

    // overwrite save compile date with the current compile date
    strcpy(config.compDate, this->_compDate);

    // write config to eeprom
    this->writeConfig(config);

    #if DEBUG_LVL >= 2
        DEBUG_PRINTLN(F(" Reset for fresh start!"));
        DEBUG_PRINTLN(FPSTR(SER_DEV));
    #endif
    ESP.restart();
}

/*-----------------------------------------------------------------------------
                        IOTAppStory preSetAppName

*///---------------------------------------------------------------------------
void IOTAppStory::preSetAppName(String appName) {
    // get config from EEPROM
    ConfigStruct config;
    this->readConfig(config);

    this->_setPreSet = false;
    this->SetConfigValueCharArray(config.appName, appName, 33, this->_setPreSet);

    if(this->_setPreSet) {
        // write config to EEPROM
        this->writeConfig(config);
    }
}

/*-----------------------------------------------------------------------------
                        IOTAppStory preSetAppVersion

*///---------------------------------------------------------------------------
void IOTAppStory::preSetAppVersion(String appVersion) {
    // get config from EEPROM
    ConfigStruct config;
    this->readConfig(config);

    this->_setPreSet = false;
    this->SetConfigValueCharArray(config.appVersion, appVersion, 11, this->_setPreSet);

    if(this->_setPreSet) {
        // write config to EEPROM
        this->writeConfig(config);
    }
}

/*-----------------------------------------------------------------------------
                        IOTAppStory preSetDeviceName

*///---------------------------------------------------------------------------
void IOTAppStory::preSetDeviceName(String deviceName) {
    // get config from EEPROM
    ConfigStruct config;
    this->readConfig(config);

    this->_setPreSet = false;
    this->SetConfigValueCharArray(config.deviceName, deviceName, STRUCT_BNAME_SIZE, this->_setPreSet);

    if(this->_setPreSet) {
        // write config to EEPROM
        this->writeConfig(config);
    }
}

/*-----------------------------------------------------------------------------
                        IOTAppStory preSetAutoUpdate

*///---------------------------------------------------------------------------
void IOTAppStory::preSetAutoUpdate(bool automaticUpdate) {
    this->_updateOnBoot = automaticUpdate;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory preSetAutoConfig

*///---------------------------------------------------------------------------
void IOTAppStory::preSetAutoConfig(bool automaticConfig) {
    this->_automaticConfig = automaticConfig;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory preSetWifiConnectOnBoot

*///---------------------------------------------------------------------------
void IOTAppStory::preSetConnWifiOnBoot(bool automaticWifiConnectOnBoot) {
    this->_automaticWifiConnectOnBoot = automaticWifiConnectOnBoot;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory preSetWifi

*///---------------------------------------------------------------------------
void IOTAppStory::preSetWifi(const char* ssid, const char* password) {
    // Save Wifi presets if newer
    WiFiConnector WiFiConn;
    if(WiFiConn.getSSIDfromEEPROM(1) != String(ssid)) {
        WiFiConn.addAPtoEEPROM(ssid, password, 1);
    }
}

/*-----------------------------------------------------------------------------
                        IOTAppStory setCallHome

*///---------------------------------------------------------------------------
void IOTAppStory::setCallHome(bool callHome) {} // <----- deprecated left for compatibility. Remove with version 3.0.0

/*-----------------------------------------------------------------------------
                        IOTAppStory setCallHomeInterval

*///---------------------------------------------------------------------------
void IOTAppStory::setCallHomeInterval(unsigned long interval) {
    this->_callHomeInterval = interval * 1000; //Convert to millis so users can pass seconds to this function
}

/*-----------------------------------------------------------------------------
                        IOTAppStory begin

*///---------------------------------------------------------------------------
void IOTAppStory::begin(const char ea) { // <----- deprecated left for compatibility. This will be removed with version 3.0.0
    this->begin();
}

/*-----------------------------------------------------------------------------
                        IOTAppStory begin

*///---------------------------------------------------------------------------
void IOTAppStory::begin() {
    {

        // get config from EEPROM
        ConfigStruct config;
        this->readConfig(config);

        // on first boot of the app run the firstBoot() function
        if(strcmp(config.compDate, this->_compDate) != 0) {
            this->firstBoot();
        }

        #if DEBUG_LVL >= 1
            DEBUG_PRINTLN(FPSTR(SER_DEV));

            DEBUG_PRINT(SER_START);
            DEBUG_PRINT(config.appName);
            DEBUG_PRINT(F(" v"));
            DEBUG_PRINTLN(config.appVersion);

            #if DEBUG_LVL >= 2
                DEBUG_PRINTLN(FPSTR(SER_DEV));
                DEBUG_PRINTF_P(SER_MODE_SEL_BTN, this->_modeButton, config.deviceName, this->_updateOnBoot);
            #endif

            DEBUG_PRINTLN(FPSTR(SER_DEV));
        #endif

        // set the input pin for Config/Update mode selection
        pinMode(this->_modeButton, INPUT_PULLUP);

        // set the "hard" reset(power) pin for the Nextion display
        // and turn the display on
        #if OTA_UPD_CHECK_NEXTION == true
            pinMode(NEXT_RES, OUTPUT);
            digitalWrite(NEXT_RES, HIGH);
        #endif

        {
        // Read the "bootTime" & "boardMode" from the Non-volatile storage on ESP32 processor
        BoardInfo boardInfo(this->bootTimes, this->boardMode);
        boardInfo.read();

        // BOOT STATISTICS read and increase boot statistics (optional)
        #if BOOTSTATISTICS == true && DEBUG_LVL >= 1
            this->bootTimes++;
            boardInfo.write();

            #if DEBUG_LVL >= 1
                this->printBoardInfo();
            #endif
        #endif
        }

        // --------- START WIFI --------------------------
		if((_automaticWifiConnectOnBoot || this->boardMode == 'C') && !this->WiFiConnected){
			// Setup wifi with cred etc connect to AP
			this->WiFiSetupAndConnect();
		}

        //---------- SELECT BOARD MODE -----------------------------
        #if CFG_INCLUDE == true
            if(this->boardMode == 'C') {
                {
                    // callback entered config mode
                    if(this->_configModeCallback) {
                        this->_configModeCallback();
                    }

                    // notifi IAS & enduser this device went to config mode (also sends localIP)
                    #if CFG_ANNOUNCE == true
                        if(this->WiFiConnected) {
							
                            #if SNTP_INT_CLOCK_UPD == true
                              if(!this->_timeSet){
                                this->ntpWaitForSync();
                              }
                            #endif
							
                            this->iasLog("1");
                        }
                    #endif
                #ifdef ESP32    //<<--- this is to prevent nasty async tcp errors
                }
                #endif
                    // run config server
                    ConfigServer configServer(*this, config);
                    configServer.run();
                #ifdef ESP8266
                }
                #endif
                delay(100);

                // notifi IAS & enduser this device has left config mode (also sends localIP)
                #if CFG_ANNOUNCE == true
                    if(this->WiFiConnected) {
                        this->iasLog("0");
						delay(100);
                    }
                #endif

                // Restart & return to Normal Operation
                this->espRestart('N');
            }
        #endif
    }

    // --------- if connection & automaticUpdate Update --------------------------
    if(this->WiFiConnected && this->_updateOnBoot == true) {
        this->callHome();
    }

    this->_buttonEntry = millis() + MODE_BUTTON_VERY_LONG_PRESS;    // make sure the timedifference during startup is bigger than 10 sec. Otherwise it will go either in config mode or calls home
    this->_appState = AppStateNoPress;

    #if DEBUG_FREE_HEAP == true
        DEBUG_PRINTLN(" end of IAS::begin");
        DEBUG_PRINTF(" Free heap: %u\n", ESP.getFreeHeap());
    #endif

    #if DEBUG_LVL >= 1
        DEBUG_PRINT(F("\n\n\n\n\n"));
    #endif
}

/*-----------------------------------------------------------------------------
                        IOTAppStory printBoardInfo

    Print BoardInfo

*///---------------------------------------------------------------------------
#if DEBUG_LVL >= 1
void IOTAppStory::printBoardInfo() {
    DEBUG_PRINTF_P(SER_BOOTTIMES_UPDATE, this->bootTimes, this->boardMode);
    DEBUG_PRINTLN(FPSTR(SER_DEV));
}
#endif

/*-----------------------------------------------------------------------------
                        IOTAppStory iasLog

    Send msg to iasLog

*///---------------------------------------------------------------------------
void IOTAppStory::iasLog(String msg) {

    // get config from EEPROM
    ConfigStruct config;
    this->readConfig(config);

    // notifi IAS & enduser about the localIP
    CallServer callServer(config, U_LOGGER);
    callServer.sm(&this->statusMessage);
    msg.replace(" ", "_");
    msg = "msg="+msg;

    #if DEBUG_LVL >= 3
        DEBUG_PRINT(SER_UPDATE_IASLOG);
        if(!callServer.get(OTA_LOG_FILE, msg)) {
            DEBUG_PRINTLN(SER_FAILED_COLON);
            DEBUG_PRINTLN(" " + this->statusMessage);
        }
    #else
        callServer.get(OTA_LOG_FILE, msg);
    #endif
}

/*-----------------------------------------------------------------------------
                        IOTAppStory WiFiSetupAndConnect

    Connect to Wifi AP

*///---------------------------------------------------------------------------
void IOTAppStory::WiFiSetupAndConnect() {
	// Synchronize time useing SNTP. This is necessary to verify that
	// the TLS certificates offered by servers are currently valid.
	// ESP8266 setClock BEFORE wifi for faster sync
	#if SNTP_INT_CLOCK_UPD == true && defined ESP8266
		this->setClock();
	#endif
	
	
    #if DEBUG_LVL >= 1
        DEBUG_PRINTLN(SER_CONNECTING);
    #endif

    // setup wifi credentials
    WiFiConnector WiFiConn;
    WiFiConn.setup();

    #if DEBUG_LVL >= 1
        DEBUG_PRINT(" ");
    #endif

    // connect to access point
    if(!WiFiConn.connectToAP(".")) {
        this->WiFiConnected = false;
        // FAILED
        // if conditions are met, set to config mode (C)
        if(this->_automaticConfig || this->boardMode == 'C') {

            if(this->boardMode == 'N') {
                this->boardMode = 'C';
                BoardInfo boardInfo(this->bootTimes, this->boardMode);
                boardInfo.write();
            }

            #if DEBUG_LVL >= 1
                DEBUG_PRINT(SER_CONN_NONE_GO_CFG);
            #endif

        } else {

            #if DEBUG_LVL >= 1
                // this point is only reached if _automaticConfig = false
                DEBUG_PRINT(SER_CONN_NONE_CONTINU);
            #endif
        }

    } else {
        this->WiFiConnected = true;
        // SUCCES
        // Show connection details if debug level is set
        #if DEBUG_LVL >= 1
            DEBUG_PRINTLN(SER_CONNECTED);
            DEBUG_PRINT(SER_DEV_IP);
            DEBUG_PRINTLN(WiFi.localIP());
        #endif
        #if DEBUG_LVL >= 2
            DEBUG_PRINT(SER_DEV_MAC);
            DEBUG_PRINTLN(WiFi.macAddress());
        #endif

        // Register host name in WiFi and mDNS
        #if WIFI_USE_MDNS == true
		
            // get config from EEPROM
            ConfigStruct config;
            this->readConfig(config);

            if(MDNS.begin(config.deviceName)) {

                #if DEBUG_LVL >= 1
                    DEBUG_PRINT(SER_DEV_MDNS);
                    DEBUG_PRINT(config.deviceName);
                    DEBUG_PRINT(".local");
                #endif

                #if DEBUG_LVL >= 3
                    DEBUG_PRINTLN(SER_DEV_MDNS_INFO);
                #endif
                #if DEBUG_LVL == 1 || DEBUG_LVL == 2
                    DEBUG_PRINTLN(F(""));
                #endif

            } else {
                #if DEBUG_LVL >= 1
                    DEBUG_PRINTLN(SER_DEV_MDNS_FAIL);
                #endif
            }
        #endif
    }

    #if DEBUG_LVL >= 1
        DEBUG_PRINTLN(FPSTR(SER_DEV));
    #endif
	
	// Synchronize time useing SNTP. This is necessary to verify that
	// the TLS certificates offered by servers are currently valid.
	// ESP32 setClock AFTER wifi as stated in the docs & to prevent crashes
	#if SNTP_INT_CLOCK_UPD == true && defined ESP32
		this->setClock();
	#endif
}

/*-----------------------------------------------------------------------------
                        IOTAppStory WiFiConnect

    Connect to last used wifi Acces Point

*///---------------------------------------------------------------------------
void IOTAppStory::WiFiConnect(){
    WiFiConnector WiFiConn;
	WiFiConn.setup();
	
    // connect to access point
    if(!WiFiConn.connectToAP(".")){
        this->WiFiConnected = false;
        #if DEBUG_LVL >= 2
            DEBUG_PRINTLN(F(" WiFi connection failed!"));
            DEBUG_PRINTLN(FPSTR(SER_DEV));
        #endif
    }else{
        this->WiFiConnected = true;
        #if DEBUG_LVL >= 2
            DEBUG_PRINTLN(F(" WiFi connected!"));
            DEBUG_PRINTLN(FPSTR(SER_DEV));
        #endif
    }
}

/*-----------------------------------------------------------------------------
                        IOTAppStory WiFiDisconnect

    Disconnect wifi

*///---------------------------------------------------------------------------
void IOTAppStory::WiFiDisconnect() {
    WiFi.disconnect();
    this->WiFiConnected = false;
    #if DEBUG_LVL >= 2
        DEBUG_PRINTLN(F(" WiFi disconnected!"));
        DEBUG_PRINTLN(FPSTR(SER_DEV));
    #endif
}




/*-----------------------------------------------------------------------------
                        IOTAppStory setClock

    Set time via NTP, as required for x.509 validation

*///---------------------------------------------------------------------------
void IOTAppStory::setClock() {

    NtpHelper.ServerSetup(SNTP_INT_CLOCK_TIME_ZONE, SNTP_INT_CLOCK_SERV_1, SNTP_INT_CLOCK_SERV_2);

    #if DEBUG_LVL >= 2
		  DEBUG_PRINT(SER_SNTP_SETUP);
    #endif
	
    #if DEBUG_LVL >= 3  //<!-- will be debug level 3 when released
		  DEBUG_PRINT(F(" - "));
		  DEBUG_PRINTLN(SNTP_INT_CLOCK_TIME_ZONE);
		  DEBUG_PRINT(F(" - "));
		  DEBUG_PRINTLN(SNTP_INT_CLOCK_SERV_1);
		  DEBUG_PRINT(F(" - "));
		  DEBUG_PRINTLN(SNTP_INT_CLOCK_SERV_2);
    #endif

    #if DEBUG_LVL >= 2
		  DEBUG_PRINTLN(FPSTR(SER_DEV));
    #endif
}


bool IOTAppStory::ntpWaitForSync(int retries) {
	#if DEBUG_LVL >= 2
		DEBUG_PRINT(SER_SNTP_SYNC_TIME);
	#endif
	
	if(!NtpHelper.WaitForSync(retries, ".")){
		return false;
	}
	this->_timeSet = true;
	time_t now = time(nullptr);

	#if DEBUG_LVL >= 3
		DEBUG_PRINT(F("\n "));
		DEBUG_PRINT(SNTP_INT_CLOCK_TIME_ZONE);
		DEBUG_PRINT(F(": "));
		DEBUG_PRINT(ctime(&now));
	#endif
	
	return true;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory callHome

    Call home and check for updates

*///---------------------------------------------------------------------------
void IOTAppStory::callHome(bool spiffs /*= true*/) {

    // update from IOTappStory.com
    #if DEBUG_LVL >= 2
        DEBUG_PRINTLN(SER_CALLING_HOME);
    #endif

    if (this->_firmwareUpdateCheckCallback) {
        this->_firmwareUpdateCheckCallback();
    }
	
	if(!this->WiFiConnected){
		#if DEBUG_LVL >= 2
			DEBUG_PRINTLN(F(" Failed: no wifi connection"));
		#endif		
		return;
	}

	#if SNTP_INT_CLOCK_UPD == true
	if(!this->_timeSet){
		this->ntpWaitForSync();
	}
	#endif
	

    // try to update sketch from IOTAppStory
    this->iotUpdater();

    // try to update spiffs from IOTAppStory
    #if OTA_UPD_CHECK_SPIFFS == true
        if(spiffs) {
            this->iotUpdater(U_SPIFFS);
        }
    #endif

    #if OTA_UPD_CHECK_NEXTION == true
        this->iotUpdater(U_NEXTION);
    #endif

    #if DEBUG_LVL >= 2
        DEBUG_PRINTLN(SER_RET_FROM_IAS);
    #endif
    #if DEBUG_LVL >= 1
        DEBUG_PRINTLN(FPSTR(SER_DEV));
    #endif

    // update last time called home
    this->_lastCallHomeTime = millis();
}

/*-----------------------------------------------------------------------------
                        IOTAppStory iotUpdater

    IOT updater

*///---------------------------------------------------------------------------
bool IOTAppStory::iotUpdater(int command) {

    // get config from EEPROM
    ConfigStruct config;
    this->readConfig(config);
    yield();

    bool result = false;
    #if DEBUG_LVL >= 2
        DEBUG_PRINT(F("\n"));
    #endif
    #if DEBUG_LVL >= 1
        DEBUG_PRINT(SER_CHECK_FOR);
    #endif
    #if DEBUG_LVL >= 1
        if(command == U_FLASH) {
            DEBUG_PRINT(SER_APP_SKETCH);

        } else if(command == U_SPIFFS) {
            DEBUG_PRINT(SER_SPIFFS);

        }
        #if OTA_UPD_CHECK_NEXTION == true
            else if(command == U_NEXTION) {
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

    FirmwareStruct  firmwareStruct;
    CallServer callServer(config, command);
    callServer.sm(&this->statusMessage);

    Stream &clientStream = callServer.getStream(&firmwareStruct);
    yield();

    if(!firmwareStruct.success) {
        #if DEBUG_LVL >= 2
            DEBUG_PRINTLN(" " + this->statusMessage);
        #endif
        if(this->_firmwareNoUpdateCallback) {
            this->_firmwareNoUpdateCallback(this->statusMessage);
        }

        return false;
    }

    if (this->_firmwareUpdateDownloadCallback) {
        this->_firmwareUpdateDownloadCallback();
    }

    if(command == U_FLASH || command == U_SPIFFS) {
        // sketch / spiffs
        result = this->espInstaller(clientStream, &firmwareStruct, UpdateESP, command);
    }
    #if OTA_UPD_CHECK_NEXTION == true
        if(command == U_NEXTION) {
            // nextion display
            this->espInstaller(clientStream, &firmwareStruct, UpdateNextion, command);
        }
    #endif

    if(result && (command == U_FLASH || command == U_NEXTION)) {

        // succesfull update
        #if DEBUG_LVL >= 1
            DEBUG_PRINTLN(SER_REBOOT_NEC);
        #endif

        // reboot to start the new updated firmware
        ESP.restart();
    }
    return true;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory espInstaller

    espInstaller

*///---------------------------------------------------------------------------
bool IOTAppStory::espInstaller(Stream &streamPtr, FirmwareStruct *firmwareStruct, UpdateClassVirt& devObj, int command) {
    devObj.sm(&this->statusMessage);
    bool result = devObj.prepareUpdate((*firmwareStruct).xlength, (*firmwareStruct).xmd5, command);

    if(!result) {
        #if DEBUG_LVL >= 2
            DEBUG_PRINTLN(this->statusMessage);
        #endif
    } else {

        #if DEBUG_LVL >= 2
            DEBUG_PRINT(SER_INSTALLING);
        #endif

        yield();

        // Write the buffered bytes to the esp. If this fails, return false.
        {
            // create buffer for read
            uint8_t buff[OTA_BUFFER] = { 0 };

            // to do counter
            int updTodo = (*firmwareStruct).xlength;

            // Upload the received byte Stream to the device
            while(updTodo > 0 || updTodo == -1) {

                // get available data size
                size_t size = streamPtr.available();

                if(size) {
                    // read up to 2048 byte into the buffer
                    size_t c = streamPtr.readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                    // Write the buffered bytes to the esp. If this fails, return false.
                    result = devObj.update(buff, c);

                    if(updTodo > 0) {
                        updTodo -= c;
                    }

                    if (this->_firmwareUpdateProgressCallback) {
                        this->_firmwareUpdateProgressCallback((*firmwareStruct).xlength - updTodo, (*firmwareStruct).xlength);
                        //DEBUG_PRINTF(" Free heap: %u\n", ESP.getFreeHeap());
                    }
                }
                delay(1);
            }
        }

        if(!result) {
            #if DEBUG_LVL >= 2
                DEBUG_PRINT(SER_UPDATEDERROR);
                DEBUG_PRINTLN(this->statusMessage);
            #endif
        } else {

            // end: wait(delay) for the nextion to finish the update process, send nextion reset command and end the serial connection to the nextion
            result = devObj.end();

            if(result) {
                // on succesfull firmware installation
                #if DEBUG_LVL >= 2
                    DEBUG_PRINT(SER_UPDATEDTO);
                    DEBUG_PRINTLN((*firmwareStruct).xname+" v"+ (*firmwareStruct).xver);
                #endif

                // get config from EEPROM
                ConfigStruct config;
                this->readConfig(config);

                if(command == U_FLASH) {
                    // write received appName & appVersion to config
                    (*firmwareStruct).xname.toCharArray(config.appName, 33);
                    (*firmwareStruct).xver.toCharArray(config.appVersion, 12);
                }

                #if OTA_UPD_CHECK_NEXTION == true
                    if(command == U_NEXTION) {
                        // update nextion md5
                        (*firmwareStruct).xmd5.toCharArray(config.next_md5, 33);
                    }
                #endif

                // write config to EEPROM
                this->writeConfig(config);


                if (this->_firmwareUpdateSuccessCallback) {
                    this->_firmwareUpdateSuccessCallback();
                }

            }else{
                // update failed
                #if DEBUG_LVL >= 2
                    DEBUG_PRINTLN(" " + this->statusMessage);
                #endif
                if(this->_firmwareUpdateErrorCallback) {
                    this->_firmwareUpdateErrorCallback(this->statusMessage);
                }
            }
        }
    }
    return result;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory addField

    Add fields to the fieldStruct

*///---------------------------------------------------------------------------
#if(EEPROM_STORAGE_STYLE == EEP_OLD)
	void IOTAppStory::addField(char* &defaultVal, const char *fieldLabel, const int length, const char type) {
		// get config from EEPROM
		ConfigStruct config;
		this->readConfig(config);
		if(strcmp(config.compDate, this->_compDate) == 0) {
			if(this->_nrXF >= MAXNUMEXTRAFIELDS) {
				// if MAXNUMEXTRAFIELDS is reached return an error
				#if DEBUG_LVL >= 1
					DEBUG_PRINTLN(SER_PROC_ERROR);
				#endif
			} else {
				#if DEBUG_LVL >= 1
					// if this is the first field being processed display header
					if(this->_nrXF == 0){

						#if DEBUG_LVL >= 2
							DEBUG_PRINTLN(FPSTR(SER_DEV));
						#endif

						DEBUG_PRINT(SER_PROC_FIELDS);
						#if DEBUG_LVL == 1
							DEBUG_PRINTLN(F(""));
						#endif

						#if DEBUG_LVL >= 2
							DEBUG_PRINTLN(SER_PROC_TBL_HDR);
						#endif
					}
				#endif

				// init fieldStruct
				AddFieldStruct fieldStruct;

				// calculate EEPROM addresses
				const int eepStartAddress = FIELD_EEP_START_ADDR + (this->_nrXF * sizeof(fieldStruct));
				const int eepEndAddress = eepStartAddress + sizeof(fieldStruct);
				const int magicBytesBegin = eepEndAddress - 3;
				int eepFieldStart;

				if(this->_nrXF == 0) {
					eepFieldStart = FIELD_EEP_START_ADDR + (MAXNUMEXTRAFIELDS * sizeof(fieldStruct)) + this->_nrXFlastAdd;
				} else {
					eepFieldStart = this->_nrXFlastAdd;
				}
				this->_nrXFlastAdd = eepFieldStart + length + 1;


				#if DEBUG_LVL >= 2
					DEBUG_PRINTF_P(PSTR(" %02d | %-30s | %03d | %04d to %04d | %-30s | "), this->_nrXF+1, fieldLabel, length, eepFieldStart, this->_nrXFlastAdd, defaultVal);
				#endif

				// EEPROM begin
				EEPROM.begin(EEPROM_SIZE);

				// check for MAGICEEP to confirm the this fieldStruct is stored in EEPROM
				if(EEPROM.read(magicBytesBegin) != MAGICEEP[0]) {
					#if DEBUG_LVL >= 2
						DEBUG_PRINTF_P(SER_PROC_TBL_WRITE, defaultVal);
					#endif

					// add values to the fieldstruct
					fieldStruct.fieldLabel  = fieldLabel;
					fieldStruct.length      = length;
					fieldStruct.type        = type;

					// put the fieldStruct to EEPROM
					EEPROM.put(eepStartAddress, fieldStruct);

					// temp val buffer
					char eepVal[length+1];
					strcpy(eepVal, defaultVal);

					// put the field value to EEPROM
					unsigned int ee = 0;
					for(unsigned int e=eepFieldStart; e < this->_nrXFlastAdd; e++) {
						EEPROM.write(e, eepVal[ee]);
						ee++;
					}

				} else {
					// get the fieldStruct from EEPROM
					EEPROM.get(eepStartAddress, fieldStruct);

					// temp val buffer
					char eepVal[length+1];

					// read field value from EEPROM and store it in eepVal buffer
					unsigned int ee = 0;
					for(unsigned int e=eepFieldStart; e < this->_nrXFlastAdd; e++) {
						eepVal[ee] = EEPROM.read(e);
						ee++;
					}

					// compair EEPROM value with the defaultVal
					if(strcmp(eepVal, defaultVal) == 0) {
						// EEPROM value is the same als the default value
						#if DEBUG_LVL >= 2
							DEBUG_PRINTF_P(SER_PROC_TBL_DEF, defaultVal);
						#endif
					} else {

						// EEPROM value is NOT the same als the default value
						#if DEBUG_LVL >= 2
							DEBUG_PRINTF_P(SER_PROC_TBL_OVRW, eepVal);
						#endif

						// workaround to prevent jiberish chars | move addField char* to char[] with v3!
						defaultVal = new char[length+1];

						// update the default value with the value from EEPROM
						strcpy(defaultVal, eepVal);
					}

					bool putfieldStruct = false;

					// compair EEPROM fieldLabel with the current fieldLabel
					if(fieldStruct.fieldLabel != fieldLabel) {
						// EEPROM value is NOT the same als the default value
						#if DEBUG_LVL >= 2
							DEBUG_PRINTLN("Overwritting label");
						#endif
						// update the default value with the value from EEPROM
						fieldStruct.fieldLabel  = fieldLabel;
						putfieldStruct = true;
					}

					// compair EEPROM fieldLabel with the current fieldLabel
					if(fieldStruct.length != length) {
						// EEPROM value is NOT the same als the default value
						#if DEBUG_LVL >= 2
							DEBUG_PRINTLN("Overwritting length");
						#endif
						// update the default value with the value from EEPROM
						fieldStruct.length      = length;
						putfieldStruct = true;
					}

					// compair EEPROM fieldLabel with the current fieldLabel
					if(fieldStruct.type != type) {
						// EEPROM value is NOT the same als the default value
						#if DEBUG_LVL >= 2
							DEBUG_PRINTLN("Overwritting type");
						#endif
						// update the default value with the value from EEPROM
						fieldStruct.type        = type;
						putfieldStruct = true;
					}

					if(putfieldStruct) {
						// put the fieldStruct to EEPROM
						EEPROM.put(eepStartAddress, fieldStruct);
					}
				}

				// EEPROM end
				EEPROM.end();

				#if DEBUG_LVL >= 1
					DEBUG_PRINTLN("");
				#endif

				delay(200);

				// increase added xtra field count
				this->_nrXF++;
				this->eepFreeFrom = this->_nrXFlastAdd;
			}
		}
	}
#else
	void IOTAppStory::addField(char* &defaultVal, const char *fieldLabel, const int length, const char type) {
		// get config from EEPROM
		ConfigStruct config;
		this->readConfig(config);
		
		if(strcmp(config.compDate, this->_compDate) == 0) {

			#if DEBUG_LVL >= 1
				// if this is the first field being processed display header
				if(this->_nrXF == 0){

					#if DEBUG_LVL >= 2
						DEBUG_PRINTLN(FPSTR(SER_DEV));
					#endif
					DEBUG_PRINT(SER_PROC_FIELDS);
					#if DEBUG_LVL == 1
						DEBUG_PRINTLN(F(""));
					#endif
					#if DEBUG_LVL >= 2
						DEBUG_PRINTLN(SER_PROC_TBL_HDR);
					#endif
				}
			#endif

			// init fieldStruct
			AddFieldStruct fieldStruct;

			// calculate EEPROM addresses
			int eepFieldHdrStart = this->_nrXFlastAdd;
			if(eepFieldHdrStart == 0) {
				eepFieldHdrStart 		= FIELD_EEP_START_ADDR;
			}
			
			const int eepFieldHdrEnd 	= eepFieldHdrStart + sizeof(fieldStruct);
			const int magicBytesBegin 	= eepFieldHdrEnd - 3;
			
			const int eepFieldValStart	= eepFieldHdrEnd;
			const int eepFieldValEnd	= eepFieldValStart + length + 1;

			// check if the last field address fits EEPROM size
			if(eepFieldValEnd > EEPROM_SIZE){
				
				#if DEBUG_LVL >= 1
					DEBUG_PRINTLN(FPSTR(SER_DEV));
					DEBUG_PRINTLN(SER_PROC_ERROR);
				#endif
				#if DEBUG_LVL >= 2
					DEBUG_PRINT(SER_LAST_FIELD_ENDS_AT);
					DEBUG_PRINT(eepFieldValEnd);
					DEBUG_PRINT(" |");
					DEBUG_PRINT(SER_EEPROM_SIZE);
					DEBUG_PRINTLN(EEPROM_SIZE);
				#endif

			}else{

				#if DEBUG_LVL >= 2
					DEBUG_PRINTF_P(PSTR(" %02d | %-30s | %03d | %04d to %04d | %04d to %04d | %-30s | "), this->_nrXF+1, fieldLabel, length, eepFieldHdrStart, eepFieldHdrEnd, eepFieldValStart, eepFieldValEnd, defaultVal);
				#endif

				// EEPROM begin
				EEPROM.begin(EEPROM_SIZE);

				// check for MAGICEEP to confirm the this fieldStruct is stored in EEPROM
				if(EEPROM.read(magicBytesBegin) != MAGICEEP[0]) {
					#if DEBUG_LVL >= 2
						DEBUG_PRINTF_P(SER_PROC_TBL_WRITE, defaultVal);
					#endif

					// add values to the fieldstruct
					fieldStruct.fieldLabel  = fieldLabel;
					fieldStruct.length      = length;
					fieldStruct.type        = type;

					// put the fieldStruct to EEPROM
					EEPROM.put(eepFieldHdrStart, fieldStruct);

					// temp val buffer
					char eepVal[length+1];
					strcpy(eepVal, defaultVal);

					// put the field value to EEPROM
					unsigned int ee = 0;
					for(unsigned int e=eepFieldValStart; e < eepFieldValEnd; e++) {
						EEPROM.write(e, eepVal[ee]);
						ee++;
					}

				} else {
					// get the fieldStruct from EEPROM
					EEPROM.get(eepFieldHdrStart, fieldStruct);

					// temp val buffer
					char eepVal[length+1];

					// read field value from EEPROM and store it in eepVal buffer
					unsigned int ee = 0;
					for(unsigned int e=eepFieldValStart; e < eepFieldValEnd; e++) {
						eepVal[ee] = EEPROM.read(e);
						ee++;
					}

					// compair EEPROM value with the defaultVal
					if(strcmp(eepVal, defaultVal) == 0) {
						// EEPROM value is the same als the default value
						#if DEBUG_LVL >= 2
							DEBUG_PRINTF_P(SER_PROC_TBL_DEF, defaultVal);
						#endif
					} else {

						// EEPROM value is NOT the same als the default value
						#if DEBUG_LVL >= 2
							DEBUG_PRINTF_P(SER_PROC_TBL_OVRW, eepVal);
						#endif

						// workaround to prevent jiberish chars | move addField char* to char[] with v3!
						defaultVal = new char[length+1];

						// update the default value with the value from EEPROM
						strcpy(defaultVal, eepVal);
					}

					bool putfieldStruct = false;

					// compair EEPROM fieldLabel with the current fieldLabel
					if(fieldStruct.fieldLabel != fieldLabel) {
						// EEPROM value is NOT the same als the default value
						#if DEBUG_LVL >= 2
							DEBUG_PRINTLN("Overwritting label");
						#endif
						// update the default value with the value from EEPROM
						fieldStruct.fieldLabel  = fieldLabel;
						putfieldStruct = true;
					}

					// compair EEPROM fieldLabel with the current fieldLabel
					if(fieldStruct.length != length) {
						// EEPROM value is NOT the same als the default value
						#if DEBUG_LVL >= 2
							DEBUG_PRINTLN("Overwritting length");
						#endif
						// update the default value with the value from EEPROM
						fieldStruct.length      = length;
						putfieldStruct = true;
					}

					// compair EEPROM fieldLabel with the current fieldLabel
					if(fieldStruct.type != type) {
						// EEPROM value is NOT the same als the default value
						#if DEBUG_LVL >= 2
							DEBUG_PRINTLN("Overwritting type");
						#endif
						// update the default value with the value from EEPROM
						fieldStruct.type        = type;
						putfieldStruct = true;
					}

					if(putfieldStruct) {
						// put the fieldStruct to EEPROM
						EEPROM.put(eepFieldHdrStart, fieldStruct);
					}
				}

				// EEPROM end
				EEPROM.end();

				#if DEBUG_LVL >= 1
					DEBUG_PRINTLN("");
				#endif

				delay(200);

				// increase added xtra field count
				this->_nrXF++;
				this->_nrXFlastAdd 	= eepFieldValEnd;
				this->eepFreeFrom 	= this->_nrXFlastAdd;
			}
		}
	}
#endif



bool IOTAppStory::eepFieldsConvertOldToNew(){
	
	if(EEPROM_STORAGE_STYLE == EEP_OLD){
		#if DEBUG_LVL >= 1
			DEBUG_PRINTLN(" ERROR: eepFieldsConvertOldToNew() will only work if config.h EEPROM_STORAGE_STYLE is set to EEP_NEW");
		#endif
		
		return false;
	}
	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN("\n Running: eepFieldsConvertOldToNew()\n\n Scanning storage style");
	#endif
	
	// EEPROM begin
	EEPROM.begin(EEPROM_SIZE);
	
	unsigned int eepCurrAt 		= 0;
	unsigned int eepValFound 	= 0;
	AddFieldStruct eepHdrArray[MAXNUMEXTRAFIELDS];
	String eepValArray[MAXNUMEXTRAFIELDS];
	
	{
		// check if current eeprom layout is in the old style
		// and thus can be converted to the new style
		if(EEPROM.read(FIELD_EEP_START_ADDR + sizeof(eepHdrArray[0]) - 3) != MAGICEEP[0] && EEPROM.read(FIELD_EEP_START_ADDR + sizeof(eepHdrArray[0]) - 2) != MAGICEEP[1]) {
			#if DEBUG_LVL >= 1
				DEBUG_PRINTLN("\n Did not detect stored fields");
			#endif
			
			return false;
		}
		
		if(EEPROM.read(FIELD_EEP_START_ADDR + (sizeof(eepHdrArray[0]) * 2) - 3) == MAGICEEP[0] && EEPROM.read(FIELD_EEP_START_ADDR + (sizeof(eepHdrArray[0]) * 2) - 2) == MAGICEEP[1]) {
			#if DEBUG_LVL >= 1
				DEBUG_PRINTLN("\n - old style detected\n Continue conversion.");
			#endif
		}else{
			#if DEBUG_LVL >= 1
				DEBUG_PRINTLN("\n - new style detected\n Ending conversion.");
			#endif
			
			return false;
		}
		
		
		
		for(unsigned int i = 0; i < MAXNUMEXTRAFIELDS; ++i) {

			// calculate EEPROM addresses
			const int eepStartAddress 		= FIELD_EEP_START_ADDR + (eepValFound * sizeof(eepHdrArray[eepValFound]));
			const int magicBytesBegin 		= eepStartAddress + sizeof(eepHdrArray[eepValFound]) - 3;
			const int secondMagicBytesBegin = eepStartAddress + (sizeof(eepHdrArray[eepValFound]) *2 ) - 3;
			int eepFieldStart;
			
			if(EEPROM.read(magicBytesBegin) == MAGICEEP[0]) {

				// get the fieldStruct from EEPROM
				EEPROM.get(eepStartAddress, eepHdrArray[eepValFound]);

				if(eepValFound == 0) {
					eepFieldStart = FIELD_EEP_START_ADDR + (MAXNUMEXTRAFIELDS * sizeof(eepHdrArray[eepValFound])) + eepCurrAt;
				} else {
					eepFieldStart = eepCurrAt;
				}
				eepCurrAt = eepFieldStart + eepHdrArray[eepValFound].length + 1;

				// temp buffer
				char eepVal[eepHdrArray[eepValFound].length + 1];

				// read field value from EEPROM and store it in eepVal buffer
				unsigned int ee = 0;
				for(unsigned int e=eepFieldStart; e < eepCurrAt; e++) {
					eepVal[ee] = EEPROM.read(e);
					ee++;
				}
				
				// update eepValArray
				eepValArray[eepValFound] = eepVal;
				eepValFound++;
			}
		}
		
		#if DEBUG_LVL >= 1
			DEBUG_PRINT("\n Found field values: ");
			DEBUG_PRINT(eepValFound);
			DEBUG_PRINT("\n");
		#endif
		
		#if DEBUG_LVL >= 3
			for(unsigned int i = 0; i < eepValFound; ++i) {
				DEBUG_PRINTLN(String(eepValArray[i]));
			}
		#endif
	}
	#if DEBUG_LVL >= 1
		DEBUG_PRINTLN("\n Write found headers & values to the new style");
	#endif
	unsigned int eepLastWritten = 0;
	
	for(unsigned int i = 0; i < eepValFound; ++i) {
		
		// calculate EEPROM addresses
		int eepFieldHdrStart = eepLastWritten;
		if(eepFieldHdrStart == 0) {
			eepFieldHdrStart 		= FIELD_EEP_START_ADDR;
		}
		
		const int eepFieldHdrEnd 	= eepFieldHdrStart + sizeof(eepHdrArray[i]);
		const int magicBytesBegin 	= eepFieldHdrEnd - 3;
		const int eepFieldValStart	= eepFieldHdrEnd;
		

		// put the fieldStruct to EEPROM
		EEPROM.put(eepFieldHdrStart, eepHdrArray[i]);

		const int eepFieldValEnd	= eepFieldValStart + eepHdrArray[i].length + 1;
		
		// temp buffer
		char eepVal[eepHdrArray[i].length + 1];
		
		// write temp stored string to char array
		eepValArray[i].toCharArray(eepVal, eepHdrArray[i].length+1);
		
		
		#if DEBUG_LVL >= 2
			DEBUG_PRINTF_P(PSTR(" %02d | %03d | %04d to %04d | %04d to %04d | %-30s | "), i+1, eepHdrArray[i].length, eepFieldHdrStart, eepFieldHdrEnd, eepFieldValStart, eepFieldValEnd, eepVal);
			DEBUG_PRINTLN("");
		#endif
		
		// put the field value to EEPROM
		unsigned int ee = 0;
		for(unsigned int e=eepFieldValStart; e < eepFieldValEnd; e++) {
			EEPROM.write(e, eepVal[ee]);
			ee++;
		}
		
		
		eepLastWritten = eepFieldValEnd;
	}
	
	// EEPROM end
	EEPROM.end();
	delay(500);
	DEBUG_PRINTLN("\n Done!");
}






/*-----------------------------------------------------------------------------
                        IOTAppStory dPinConv

    Convert dpins to int

*///---------------------------------------------------------------------------
int IOTAppStory::dPinConv(String orgVal) {
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

/*-----------------------------------------------------------------------------
                        IOTAppStory espRestart

    Set mode and reboot

*///---------------------------------------------------------------------------
void IOTAppStory::espRestart(char mmode) {
    //while (this->isModeButtonPressed()) yield();    // wait till GPIOo released
    delay(500);
    this->boardMode = mmode;
    BoardInfo boardInfo(this->bootTimes, this->boardMode);
    boardInfo.write();
    ESP.restart();
}

/*-----------------------------------------------------------------------------
                        IOTAppStory eraseEEPROM

    Erase EEPROM from till

*///---------------------------------------------------------------------------
void IOTAppStory::eraseEEPROM(int eepFrom, int eepTo) {
    #if DEBUG_LVL >= 2 || DEBUG_EEPROM_CONFIG == true
        DEBUG_PRINTF_P(SER_ERASE_FLASH, eepFrom, eepTo);
    #endif
    EEPROM.begin(EEPROM_SIZE);
    for (int t = eepFrom; t < eepTo; t++) EEPROM.write(t, 0);
    EEPROM.end();
}

/*-----------------------------------------------------------------------------
                        IOTAppStory eraseEEPROM

    Erase EEPROM (F)ull or (P)artial

*///---------------------------------------------------------------------------
void IOTAppStory::eraseEEPROM(const char ea) {
    // erase eeprom after config (delete extra field data etc.)
    if(ea == 'F') {
        #if DEBUG_LVL >= 1
            DEBUG_PRINTLN(SER_ERASE_FULL);
        #endif
        // Wipe out WiFi credentials.
        WiFi.disconnect();
        delay(200);
        // erase full eeprom
        this->eraseEEPROM(0, EEPROM_SIZE);
    } else if(ea == 'P') {
        #if DEBUG_LVL == 1
            DEBUG_PRINTLN(SER_ERASE_PART);
        #endif
        #if DEBUG_LVL >= 2
            DEBUG_PRINTLN(SER_ERASE_PART_EXT);
        #endif

        // erase eeprom but leave the config settings
        this->eraseEEPROM(FIELD_EEP_START_ADDR, EEPROM_SIZE);
    }
}

/*-----------------------------------------------------------------------------
                        IOTAppStory writeConfig

    Write the config struct to EEPROM

*///---------------------------------------------------------------------------
void IOTAppStory::writeConfig(ConfigStruct &config) {
    #if DEBUG_EEPROM_CONFIG == true
        DEBUG_PRINTLN("DEBUG_EEPROM\t| running writeConfig(...)");
    #endif
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(CFG_EEP_START_ADDR, config);
    EEPROM.end();
}

/*-----------------------------------------------------------------------------
                        IOTAppStory readConfig

    Read the config struct from EEPROM

*///---------------------------------------------------------------------------
void IOTAppStory::readConfig(ConfigStruct& config) {
    #if DEBUG_EEPROM_CONFIG == true
        DEBUG_PRINTLN("DEBUG_EEPROM\t| running readConfig()");
    #endif
    EEPROM.begin(EEPROM_SIZE);
    const int magicBytesBegin = CFG_EEP_START_ADDR + sizeof(config) - 4;

    #if DEBUG_EEPROM_CONFIG == true
        DEBUG_PRINT("DEBUG_EEPROM\t| config start: ");
        DEBUG_PRINTLN(CFG_EEP_START_ADDR);
        DEBUG_PRINT("DEBUG_EEPROM\t| config end: ");
        DEBUG_PRINTLN(CFG_EEP_START_ADDR + sizeof(config));
        DEBUG_PRINT("DEBUG_EEPROM\t| Searching for config MAGICBYTES at: ");
        DEBUG_PRINTLN(magicBytesBegin);
    #endif
    if(EEPROM.read(magicBytesBegin) == MAGICBYTES[0] && EEPROM.read(magicBytesBegin + 1) == MAGICBYTES[1] && EEPROM.read(magicBytesBegin + 2) == MAGICBYTES[2]) {
        EEPROM.get(CFG_EEP_START_ADDR, config);
        #if DEBUG_EEPROM_CONFIG == true
            DEBUG_PRINTLN("DEBUG_EEPROM\t| Found! Succesfully read config from EEPROM");
        #endif
    } else {
        EEPROM.put(CFG_EEP_START_ADDR, config);
        #if DEBUG_EEPROM_CONFIG == true
            DEBUG_PRINTLN("DEBUG_EEPROM\t| Failed! Writing config to EEPROM and return new config");
        #endif
    }
    EEPROM.end();
}

/*-----------------------------------------------------------------------------
                        IOTAppStory loop

*///---------------------------------------------------------------------------
void IOTAppStory::loop() {
    // wifi connector
    #if WIFI_MULTI_FORCE_RECONN_ANY == true
    if(WiFi.status() == WL_NO_SSID_AVAIL) {
        this->WiFiConnected = false;
        WiFi.disconnect(false);
        delay(10);
        #if DEBUG_LVL >= 1
            DEBUG_PRINTLN(SER_CONN_LOST_RECONN);
        #endif
        WiFiConnector WiFiConn;
        WiFiConn.connectToAP(".");
        #if DEBUG_LVL >= 1
            DEBUG_PRINTLN(F(""));
            DEBUG_PRINTLN(FPSTR(SER_DEV));
        #endif
    }
    #endif

    if(WiFi.status() == WL_CONNECTED) {
        this->WiFiConnected = true;
    }else{
        this->WiFiConnected = false;
    }

    // Call home and check for updates every _callHomeInterval
    if(this->WiFiConnected && this->_callHomeInterval > 0 && millis() - this->_lastCallHomeTime > this->_callHomeInterval) {
        this->callHome();
    }

    // handle button presses: short, long, xlong
    this->getModeButtonState();

    #if DEBUG_FREE_HEAP == true
        DEBUG_PRINTLN(" end of IAS::loop");
        DEBUG_PRINTF(" Free heap: %u\n", ESP.getFreeHeap());
    #endif
}

/*-----------------------------------------------------------------------------
                        IOTAppStory buttonLoop

*///---------------------------------------------------------------------------
// ModeButtonState IOTAppStory::buttonLoop() {
//     return this->getModeButtonState();
// }

/*-----------------------------------------------------------------------------
                        IOTAppStory isModeButtonPressed

*///---------------------------------------------------------------------------
bool IOTAppStory::isModeButtonPressed() {
    return digitalRead(this->_modeButton) == LOW; // LOW means flash button IS pressed
}

/*-----------------------------------------------------------------------------
                        IOTAppStory getModeButtonState

*///---------------------------------------------------------------------------
ModeButtonState IOTAppStory::getModeButtonState() {
    while(true) {
        unsigned long buttonTime = millis() - this->_buttonEntry;
        switch(this->_appState) {
        case AppStateNoPress:
            if(this->isModeButtonPressed()) {
                this->_buttonEntry = millis();
                this->_appState = AppStateWaitPress;
                continue;
            }
            return ModeButtonNoPress;

        case AppStateWaitPress:
            if(buttonTime > MODE_BUTTON_SHORT_PRESS) {
                this->_appState = AppStateShortPress;
                if(this->_shortPressCallback) {
                    this->_shortPressCallback();
                }
                continue;
            }
            if(!this->isModeButtonPressed()) {
                this->_appState = AppStateNoPress;
            }
            return ModeButtonNoPress;

        case AppStateShortPress:
            if(buttonTime > MODE_BUTTON_LONG_PRESS) {
                this->_appState = AppStateLongPress;
                if(this->_longPressCallback) {
                    this->_longPressCallback();
                }
                continue;
            }
            if(!this->isModeButtonPressed()) {
                this->_appState = AppStateFirmwareUpdate;
                continue;
            }
            return ModeButtonShortPress;

        case AppStateLongPress:
            if(buttonTime > MODE_BUTTON_VERY_LONG_PRESS) {
                this->_appState = AppStateVeryLongPress;
                if (this->_veryLongPressCallback) {
                    this->_veryLongPressCallback();
                }
                continue;
            }
        #if CFG_INCLUDE == true
            if(!this->isModeButtonPressed()) {
                this->_appState = AppStateConfigMode;
                continue;
            }
        #endif
            return ModeButtonLongPress;

        case AppStateVeryLongPress:
            if(!this->isModeButtonPressed()) {
                this->_appState = AppStateNoPress;
                if(this->_noPressCallback) {
                    this->_noPressCallback();
                }
                continue;
            }
            return ModeButtonVeryLongPress;

        case AppStateFirmwareUpdate:
            this->_appState = AppStateNoPress;
            this->callHome();
            continue;
        #if CFG_INCLUDE == true
        case AppStateConfigMode:
            this->_appState = AppStateNoPress;
            #if DEBUG_LVL >= 1
                DEBUG_PRINTLN(SER_CONFIG_ENTER);
            #endif
            this->espRestart('C');
            continue;
        #endif
        }
    }

    return ModeButtonNoPress; // will never reach here (used just to avoid compiler warnings)
}

/*-----------------------------------------------------------------------------
                        IOTAppStory onFirstBoot

*///---------------------------------------------------------------------------
void IOTAppStory::onFirstBoot(THandlerFunction value) {
    this->_firstBootCallback = value;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory onModeButtonNoPress

*///---------------------------------------------------------------------------
void IOTAppStory::onModeButtonNoPress(THandlerFunction value) {
    this->_noPressCallback = value;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory onModeButtonShortPress

*///---------------------------------------------------------------------------
void IOTAppStory::onModeButtonShortPress(THandlerFunction value) {
    this->_shortPressCallback = value;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory onModeButtonLongPress

*///---------------------------------------------------------------------------
void IOTAppStory::onModeButtonLongPress(THandlerFunction value) {
    this->_longPressCallback = value;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory onModeButtonVeryLongPress

*///---------------------------------------------------------------------------
void IOTAppStory::onModeButtonVeryLongPress(THandlerFunction value) {
    this->_veryLongPressCallback = value;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory onFirmwareUpdateCheck

*///---------------------------------------------------------------------------
void IOTAppStory::onFirmwareUpdateCheck(THandlerFunction value) {
    this->_firmwareUpdateCheckCallback = value;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory onFirmwareNoUpdate

*///---------------------------------------------------------------------------
void IOTAppStory::onFirmwareNoUpdate(THandlerFunctionStr value) {
    this->_firmwareNoUpdateCallback = value;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory onFirmwareUpdateDownload

*///---------------------------------------------------------------------------
void IOTAppStory::onFirmwareUpdateDownload(THandlerFunction value) {
    this->_firmwareUpdateDownloadCallback = value;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory onFirmwareUpdateProgress

*///---------------------------------------------------------------------------
void IOTAppStory::onFirmwareUpdateProgress(THandlerFunctionArg value) {
    this->_firmwareUpdateProgressCallback = value;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory onFirmwareUpdateError

*///---------------------------------------------------------------------------
void IOTAppStory::onFirmwareUpdateError(THandlerFunctionStr value) {
    this->_firmwareUpdateErrorCallback = value;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory onFirmwareUpdateSuccess

*///---------------------------------------------------------------------------
void IOTAppStory::onFirmwareUpdateSuccess(THandlerFunction value) {
    this->_firmwareUpdateSuccessCallback = value;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory onConfigMode

*///---------------------------------------------------------------------------
void IOTAppStory::onConfigMode(THandlerFunction value) {
    this->_configModeCallback = value;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory strRetHtmlRoot

    Handle root

*///---------------------------------------------------------------------------
String IOTAppStory::strRetHtmlRoot() {

    String retHtml;
    retHtml += FPSTR(HTTP_TEMP_START);

    if(this->WiFiConnected) {
        retHtml.replace("{h}", FPSTR(HTTP_STA_JS));
    } else {

        retHtml.replace("{h}", FPSTR(HTTP_AP_CSS));
        retHtml += FPSTR(HTTP_WIFI_FORM);
        retHtml.replace("{r}", this->strRetWifiScan());
        retHtml += FPSTR(HTTP_AP_JS);
    }

    retHtml += FPSTR(HTTP_TEMP_END);
    return retHtml;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory strRetDevInfo

    Handle device information

*///---------------------------------------------------------------------------
String IOTAppStory::strRetDevInfo() {
    #if DEBUG_LVL >= 3
        DEBUG_PRINTLN(SER_SERV_DEV_INFO);
    #endif

    // get config from EEPROM
    ConfigStruct config;
    this->readConfig(config);

    String retHtml;
    retHtml += FPSTR(HTTP_DEV_INFO);
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
    retHtml.replace(F("{xf}"), String(this->_nrXF));

    if(String(config.actCode) == "000000" || String(config.actCode) == "") {
        retHtml.replace(F("{ac}"), "0");
    } else {
        retHtml.replace(F("{ac}"), "1");
    }

    return retHtml;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory strRetWifiScan

    Handle wifi scan

*///---------------------------------------------------------------------------
String IOTAppStory::strRetWifiScan() {

    #if DEBUG_LVL >= 3
        DEBUG_PRINTLN(SER_SERV_WIFI_SCAN_RES);
    #endif

    // WiFi.scanNetworks will return the number of networks found
    String retHtml;
    int n = WiFi.scanComplete();
    if(n == -2) {
        WiFi.scanNetworks(true);
    } else if(n) {
        /**
            All credits for the "sort networks" & "RSSI SORT" code below goes to tzapu!
        */

        // sort networks
        int indices[n];
        for(int i = 0; i < n; i++) {
            indices[i] = i;
        }

        // RSSI SORT
        for(int i = 0; i < n; i++) {
            for(int j = i + 1; j < n; j++) {
                if(WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
                    std::swap(indices[i], indices[j]);
                }
            }
        }
        /**
            All credits for the "sort networks" & "RSSI SORT" code above goes to tzapu!
        */

        for(int i = 0; i < n; i++) {
            // return html results from the wifi scan
            retHtml += FPSTR(HTTP_WIFI_SCAN);
            retHtml.replace(F("{s}"), WiFi.SSID(indices[i]));
            retHtml.replace(F("{q}"), String(WiFi.RSSI(indices[i])));
            retHtml.replace(F("{e}"), String(WiFi.encryptionType(indices[i])));
            delay(10);
        }

        WiFi.scanDelete();
        if(WiFi.scanComplete() == -2) {
            WiFi.scanNetworks(true);
        }
    }
    return retHtml;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory strRetWifiCred

    Handle wifi scan

*///---------------------------------------------------------------------------
String IOTAppStory::strRetWifiCred() {

    #if DEBUG_LVL >= 3
        DEBUG_PRINTLN(SER_SERV_WIFI_CRED);
    #endif

    WiFiConnector WiFiConn;

    String retHtml = F("[");

    for(int i = 1; i <= WIFI_MULTI_MAX; i++) {
        if(i > 1) {
            retHtml += F(",");
        }

        // create json string {\"s\":\"ssid1\"}
        retHtml += F("{\"s\":\"");

        #if WIFI_DHCP_ONLY == true
            retHtml += WiFiConn.getSSIDfromEEPROM(i);
        #else
            WiFiCredStruct config;
            WiFiConn.getAPfromEEPROM(config, i);

            retHtml += config.ssid;
            retHtml += F("\",\"si\":\"");
            retHtml += config.ip.toString();
            retHtml += F("\",\"ss\":\"");
            retHtml += config.subnet.toString();
            retHtml += F("\",\"sg\":\"");
            retHtml += config.gateway.toString();
            retHtml += F("\",\"sd\":\"");
            retHtml += config.dnsserv.toString();
        #endif

        retHtml += F("\"}");
    }

    retHtml += F("]");
    #if DEBUG_LVL >= 3
        DEBUG_PRINTLN(retHtml);
    #endif
    return retHtml;
}

/*-----------------------------------------------------------------------------
                        IOTAppStory servSaveWifiCred

    Handle save wifi credentials

*///---------------------------------------------------------------------------
bool IOTAppStory::servSaveWifiCred(const char* newSSID, const char* newPass, const int apNr) {

    WiFiConnector WiFiConn;
    if(apNr==0) {
        WiFiConn.addAndShiftAPinEEPROM(newSSID, newPass);
    } else {
        WiFiConn.addAPtoEEPROM(newSSID, newPass, apNr);
    }

    return true;        // ok
}

/*-----------------------------------------------------------------------------
                        IOTAppStory servSaveWifiCred

    Handle save wifi credentials

*///---------------------------------------------------------------------------
bool IOTAppStory::servSaveWifiCred(const char* newSSID, const char* newPass, String ip, String subnet, String gateway, String dnsserv) {

    WiFiConnector WiFiConn;
    if((ip+subnet+gateway+dnsserv) == "") {
        WiFiConn.addAPtoEEPROM(newSSID, newPass, 1);
    } else {
        WiFiConn.addAPtoEEPROM(newSSID, newPass, ip, subnet, gateway, dnsserv);
    }

    return true;        // ok
}

/*-----------------------------------------------------------------------------
                        IOTAppStory strRetAppInfo

    Handle app / firmware information

*///---------------------------------------------------------------------------
#if(EEPROM_STORAGE_STYLE == EEP_OLD)
	String IOTAppStory::strRetAppInfo() {
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(SER_SERV_APP_SETTINGS);
		#endif

		// EEPROM begin
		EEPROM.begin(EEPROM_SIZE);

		this->_nrXFlastAdd = 0;

		String retHtml = F("[");
		for(unsigned int i = 0; i < this->_nrXF; ++i) {

			// return html results from the wifi scan
			if(i > 0) {
				retHtml += F(",");
			}

			// init fieldStruct
			AddFieldStruct fieldStruct;

			// calculate EEPROM addresses
			const int eepStartAddress = FIELD_EEP_START_ADDR + (i * sizeof(AddFieldStruct));
			int eepFieldStart;

			// get the fieldStruct from EEPROM
			EEPROM.get(eepStartAddress, fieldStruct);

			if(i == 0) {
				eepFieldStart = FIELD_EEP_START_ADDR + (MAXNUMEXTRAFIELDS * sizeof(AddFieldStruct)) + this->_nrXFlastAdd;
			} else {
				eepFieldStart = this->_nrXFlastAdd;
			}
			this->_nrXFlastAdd = eepFieldStart + fieldStruct.length + 1;

			// temp buffer
			char eepVal[fieldStruct.length + 1];

			// read field value from EEPROM and store it in eepVal buffer
			unsigned int ee = 0;
			for(unsigned int e=eepFieldStart; e < this->_nrXFlastAdd; e++) {
				eepVal[ee] = EEPROM.read(e);
				ee++;
			}

			// add slashed to values where necessary to prevent the json repsons from being broken
			String value = eepVal;
			value.replace("\\", "\\\\");
			value.replace("\"", "\\\"");
			value.replace("\n", "\\n");
			value.replace("\r", "\\r");
			value.replace("\t", "\\t");
			value.replace("\b", "\\b");
			value.replace("\f", "\\f");

			// get PROGMEM json string and replace {*} with values
			retHtml += FPSTR(HTTP_APP_INFO);
			retHtml.replace(F("{l}"), String(fieldStruct.fieldLabel));
			retHtml.replace(F("{v}"), value);
			retHtml.replace(F("{n}"), String(i));
			retHtml.replace(F("{m}"), String(fieldStruct.length));
			retHtml.replace(F("{t}"), String(fieldStruct.type));
			delay(10);
		}
		retHtml += F("]");

		// EEPROM end
		EEPROM.end();
		delay(500);

		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(retHtml);
		#endif

		return retHtml;
	}
#else
	String IOTAppStory::strRetAppInfo() {
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(SER_SERV_APP_SETTINGS);
		#endif

		// EEPROM begin
		EEPROM.begin(EEPROM_SIZE);

		this->_nrXFlastAdd 	= 0;
		String retHtml 		= F("[");
		
		for(unsigned int i = 0; i < this->_nrXF; ++i) {

			// return html results from the wifi scan
			if(i > 0) {
				retHtml += F(",");
			}

			// init fieldStruct
			AddFieldStruct fieldStruct;

			// calculate EEPROM addresses
			int eepFieldHdrStart = this->_nrXFlastAdd;
			if(eepFieldHdrStart == 0) {
				eepFieldHdrStart 		= FIELD_EEP_START_ADDR;
			}
			
			const int eepFieldHdrEnd 	= eepFieldHdrStart + sizeof(fieldStruct);
			const int magicBytesBegin 	= eepFieldHdrEnd - 3;
			const int eepFieldValStart	= eepFieldHdrEnd;
			

			// get the fieldStruct from EEPROM
			EEPROM.get(eepFieldHdrStart, fieldStruct);
			
			const int eepFieldValEnd	= eepFieldValStart + fieldStruct.length + 1;
			
			// temp buffer
			char eepVal[fieldStruct.length + 1];

			// read field value from EEPROM and store it in eepVal buffer
			unsigned int ee = 0;
			for(unsigned int e=eepFieldValStart; e < eepFieldValEnd; e++) {      //this->_nrXFlastAdd
				eepVal[ee] = EEPROM.read(e);
				ee++;
			}

			// add slashed to values where necessary to prevent the json repsons from being broken
			String value = eepVal;
			value.replace("\\", "\\\\");
			value.replace("\"", "\\\"");
			value.replace("\n", "\\n");
			value.replace("\r", "\\r");
			value.replace("\t", "\\t");
			value.replace("\b", "\\b");
			value.replace("\f", "\\f");

			// get PROGMEM json string and replace {*} with values
			retHtml += FPSTR(HTTP_APP_INFO);
			retHtml.replace(F("{l}"), String(fieldStruct.fieldLabel));
			retHtml.replace(F("{v}"), value);
			retHtml.replace(F("{n}"), String(i));
			retHtml.replace(F("{m}"), String(fieldStruct.length));
			retHtml.replace(F("{t}"), String(fieldStruct.type));
			delay(10);
			
			this->_nrXFlastAdd = eepFieldValEnd;
		}
		retHtml += F("]");

		

		// EEPROM end
		EEPROM.end();
		delay(500);

		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(retHtml);
		#endif

		return retHtml;
	}
#endif
/*-----------------------------------------------------------------------------
                        IOTAppStory servSaveFngPrint

    Handle app / firmware information

*///---------------------------------------------------------------------------
#if defined  ESP8266 && HTTPS_8266_TYPE == FNGPRINT
bool IOTAppStory::servSaveFngPrint(String fngprint) {

    // get config from EEPROM
    ConfigStruct config;
    this->readConfig(config);

    #if DEBUG_LVL >= 3
        DEBUG_PRINTLN(SER_SAVE_FINGERPRINT);
        DEBUG_PRINT("Received fingerprint: ");
        DEBUG_PRINTLN(fngprint);
        DEBUG_PRINT("Current  fingerprint: ");
        DEBUG_PRINTLN(config.sha1);
    #endif

    fngprint.toCharArray(config.sha1, 60);

    // write config to EEPROM
    this->writeConfig(config);

    return true;
}
#endif

/*-----------------------------------------------------------------------------
                        IOTAppStory strRetCertScan

    Get all root certificates

*///---------------------------------------------------------------------------
#if defined  ESP32
String IOTAppStory::strRetCertScan(String path) {

    #if DEBUG_LVL >= 3
        DEBUG_PRINTLN(SER_SERV_CERT_SCAN_RES);
    #endif

    // open SPIFFS certificate directory
    File root = SPIFFS.open("/cert");
    if(!root || !root.isDirectory()) {
        #if DEBUG_LVL >= 3
            DEBUG_PRINTLN(" Failed to open directory");
        #endif

        return "0";
    }

    // delete requested file
    if(path != "") {
        if(!SPIFFS.remove(path)) {
            #if DEBUG_LVL >= 3
                DEBUG_PRINTLN(" Failed to delete file!");
            #endif
        }
    }

    // return all the files found in this directory and return them as a json string
    File file = root.openNextFile();
    String retHtml = "[";
    bool pastOne = false;

    while(file) {
        if(!file.isDirectory()) {
            if(pastOne == true) {
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

String IOTAppStory::strRetCertScan(String path) {

    #if DEBUG_LVL >= 3
        DEBUG_PRINTLN(SER_SERV_CERT_SCAN_RES);
    #endif

    //Initialize File System
    if(!ESP_SPIFFSBEGIN) {
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
    while(dir.next()) {
        if(dir.fileSize()) {
            File file = dir.openFile("r");
            if(retHtml != "[") {
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

/*-----------------------------------------------------------------------------
                        IOTAppStory servSaveAppInfo

    Save App Settings

*///---------------------------------------------------------------------------
#ifndef ARDUINO_SAMD_VARIANT_COMPLIANCE
	#if(EEPROM_STORAGE_STYLE == EEP_OLD)
		bool IOTAppStory::servSaveAppInfo(AsyncWebServerRequest *request) {
			#if DEBUG_LVL >= 3
				DEBUG_PRINTLN(SER_SAVE_APP_SETTINGS);
			#endif

			if(this->_nrXF) {
				// EEPROM begin
				EEPROM.begin(EEPROM_SIZE);
				this->_nrXFlastAdd = 0;
				// init fieldStruct
				AddFieldStruct fieldStruct;

				for(unsigned int i = 0; i < this->_nrXF; i++) {
					if(request->hasParam(String(i), true)) {
						// calculate EEPROM addresses
						const int eepStartAddress = FIELD_EEP_START_ADDR + (i * sizeof(fieldStruct));
						int eepFieldStart;
						// get the fieldStruct from EEPROM
						EEPROM.get(eepStartAddress, fieldStruct);

						if(i == 0) {
							eepFieldStart = FIELD_EEP_START_ADDR + (MAXNUMEXTRAFIELDS * sizeof(fieldStruct)) + this->_nrXFlastAdd;
						} else {
							eepFieldStart = this->_nrXFlastAdd;
						}
						this->_nrXFlastAdd = eepFieldStart + fieldStruct.length + 1;
						char eepVal[fieldStruct.length + 1];
						// read field value from EEPROM and store it in eepVal buffer
						unsigned int ee = 0;
						for(unsigned int e=eepFieldStart; e < this->_nrXFlastAdd; e++) {
							eepVal[ee] = EEPROM.read(e);
							ee++;
						}

						if(strcmp(eepVal, request->getParam(String(i), true)->value().c_str()) != 0) {
							char saveEepVal[fieldStruct.length+1];
							// overwrite current value with the saved value
							request->getParam(String(i), true)->value().toCharArray(saveEepVal, fieldStruct.length+1);
							// write the field value to EEPROM
							unsigned int ee = 0;
							for(unsigned int e=eepFieldStart; e < this->_nrXFlastAdd; e++) {
								EEPROM.write(e, saveEepVal[ee]);
								ee++;
							}

						#if DEBUG_LVL >= 3
							DEBUG_PRINT("\nOverwrite with new value: ");
							DEBUG_PRINTLN(saveEepVal);
							DEBUG_PRINT("EEPROM from: ");
							DEBUG_PRINT(eepFieldStart);
							DEBUG_PRINT(" to ");
							DEBUG_PRINTLN(this->_nrXFlastAdd);
						} else {
							DEBUG_PRINTLN("No need to overwrite current value");
						#endif
						}
					}
				}
				// EEPROM end
				EEPROM.end();
				delay(200);
				return true;
			}

			return false;
		}
	#else
		bool IOTAppStory::servSaveAppInfo(AsyncWebServerRequest *request) {
			#if DEBUG_LVL >= 3
				DEBUG_PRINTLN(SER_SAVE_APP_SETTINGS);
			#endif

			if(this->_nrXF) {
				// EEPROM begin
				EEPROM.begin(EEPROM_SIZE);
				this->_nrXFlastAdd = 0;
				// init fieldStruct
				AddFieldStruct fieldStruct;

				for(unsigned int i = 0; i < this->_nrXF; i++) {
					if(request->hasParam(String(i), true)) {
						
						// calculate EEPROM addresses
						int eepFieldHdrStart = this->_nrXFlastAdd;
						if(eepFieldHdrStart == 0) {
							eepFieldHdrStart 		= FIELD_EEP_START_ADDR;
						}
						
						const int eepFieldHdrEnd 	= eepFieldHdrStart + sizeof(fieldStruct);
						const int magicBytesBegin 	= eepFieldHdrEnd - 3;
						const int eepFieldValStart	= eepFieldHdrEnd;
						

						// get the fieldStruct from EEPROM
						EEPROM.get(eepFieldHdrStart, fieldStruct);
						
						const int eepFieldValEnd	= eepFieldValStart + fieldStruct.length + 1;

						// temp buffer
						char eepVal[fieldStruct.length + 1];
						
						// read field value from EEPROM and store it in eepVal buffer
						unsigned int ee = 0;
						for(unsigned int e=eepFieldValStart; e < eepFieldValEnd; e++) {
							eepVal[ee] = EEPROM.read(e);
							ee++;
						}

						if(strcmp(eepVal, request->getParam(String(i), true)->value().c_str()) != 0) {
							char saveEepVal[fieldStruct.length+1];
							// overwrite current value with the saved value
							request->getParam(String(i), true)->value().toCharArray(saveEepVal, fieldStruct.length+1);
							// write the field value to EEPROM
							unsigned int ee = 0;
							for(unsigned int e=eepFieldValStart; e < eepFieldValEnd; e++) {
								EEPROM.write(e, saveEepVal[ee]);
								ee++;
							}

						#if DEBUG_LVL >= 3
							DEBUG_PRINT("\nOverwrite with new value: ");
							DEBUG_PRINTLN(saveEepVal);
							DEBUG_PRINT("EEPROM from: ");
							DEBUG_PRINT(eepFieldValStart);
							DEBUG_PRINT(" to ");
							DEBUG_PRINTLN(eepFieldValEnd);
						} else {
							DEBUG_PRINTLN("No need to overwrite current value");
						#endif
						}
					
						this->_nrXFlastAdd = eepFieldValEnd;
					}
				}
				// EEPROM end
				EEPROM.end();
				delay(200);
				return true;
			}

			return false;
		}
	#endif
#endif

/*-----------------------------------------------------------------------------
                        IOTAppStory servSaveActcode

    Save activation code

*///---------------------------------------------------------------------------
bool IOTAppStory::servSaveActcode(String actcode) {
    #if DEBUG_LVL >= 3
        DEBUG_PRINT(SER_REC_ACT_CODE);
        DEBUG_PRINTLN(actcode);
    #endif

    if(actcode != "") {
        // get config from EEPROM
        ConfigStruct config;
        this->readConfig(config);
        actcode.toCharArray(config.actCode, 7);
        // write config to EEPROM
        this->writeConfig(config);
        return true;
    }

    return false;
}

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
