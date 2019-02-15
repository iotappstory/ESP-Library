#ifdef  ESP32
	#include "callServer_WiFiClientSecure.h"
	
	callServer::callServer(strConfig &config, int command){
		_config = &config;
		_command = command;
	}


	
	Stream &callServer::getStream(firmwareStruct *firmwareStruct){

	
		if(!this->get(_updateFile, "")){
			(*firmwareStruct).success = false;
			return _client;
		}
		
		
		(*firmwareStruct).xlength = 0;
		int code;
		
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(F("\n Received http header"));
			DEBUG_PRINTLN(F(" ---------------------"));
		#endif
		while(_client.available()) {

			String line = _client.readStringUntil('\n');
			
			#if DEBUG_LVL >= 3
				// Read all the lines of the reply from server and print them to Serial
				DEBUG_PRINTLN(" " + line);
			#endif
			
			if(line.startsWith(F("HTTP/1.1 "))) {
				line.remove(0, 9);
				code = line.substring(0, 3).toInt();

				if(code != 200){
					line.remove(0, 4);
					(*_statusMessage) = line;
					
					//return false;
					(*firmwareStruct).success = false;
					return _client;
				}else{
					#if DEBUG_LVL >= 1
						DEBUG_PRINT(SER_DOWNLOADING);
					#endif
				}

			}else if(line.startsWith(F("Content-Length: "))) {
				line.remove(0, 16);
				(*firmwareStruct).xlength = line.toInt();

			} else if(line.startsWith(F("x-name: "))) {
				line.remove(0, 8);
				line.trim();
				(*firmwareStruct).xname = line;

			} else if(line.startsWith(F("x-ver: "))) {
				line.remove(0, 7);
				line.trim();
				(*firmwareStruct).xver = line;

			}else if(line.startsWith(F("x-MD5: "))) {
				line.remove(0, 7);
				line.trim();
				(*firmwareStruct).xmd5 = line;

			}else if(line == "\r") {
				break;
			} 
		}
		
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(F(" Extracted from header"));
			DEBUG_PRINTLN(F(" ---------------------"));
			DEBUG_PRINT(F(" Content-Length: "));
			DEBUG_PRINTLN((*firmwareStruct).xlength);
			DEBUG_PRINT(F(" x-name: "));
			DEBUG_PRINTLN((*firmwareStruct).xname);
			DEBUG_PRINT(F(" x-ver: "));
			DEBUG_PRINTLN((*firmwareStruct).xver);
			DEBUG_PRINT(F(" x-md5: "));
			DEBUG_PRINTLN((*firmwareStruct).xmd5);
			DEBUG_PRINTLN(F(" ---------------------\n"));
		#endif
		
		if(code == 200 && (*firmwareStruct).xlength > 0 && (*firmwareStruct).xname != "" && (*firmwareStruct).xver != "" && (*firmwareStruct).xmd5 != ""){
			(*firmwareStruct).success = true;
			return _client;
		}else{
			(*_statusMessage) = SER_CALLHOME_HDR_FAILED;
			(*firmwareStruct).success = false;
			return _client;
		}
	}


	
	bool callServer::get(const char* url, String args){

		#if DEBUG_LVL >= 2
			#if HTTPS == true
				DEBUG_PRINT(F("https://"));
			#else
				DEBUG_PRINT(F("http://"));
			#endif
			DEBUG_PRINT(_updateHost);
			DEBUG_PRINTLN(url);
		#endif
		
		#if DEBUG_LVL == 1
			DEBUG_PRINTLN("");
		#endif

		#if HTTPS == true
			
			#if CERT_STORAGE == ST_SPIFFS
				#if DEBUG_LVL >= 3
					DEBUG_PRINT(SER_SPIFFS_MOUNTING);
				#endif
				
				// Start SPIFFS and load partition
				if(!SPIFFS.begin()){
				   (*_statusMessage) = SER_SPIFFS_PART_NOT_FOUND;
				   return false;
				} 

				// Get root certificate file from SPIFFS
				File file = SPIFFS.open("/iasRootCa.cer", "r");
				if(!file){
				   (*_statusMessage) = SER_CERTIFICATE_NOT_FOUND;
					return false;
				}
				
				#if DEBUG_LVL >= 3
					DEBUG_PRINT("File size: ");
					DEBUG_PRINTLN(file.size());
				#endif

				// Load root certificate (from SPIFFS)
				_client.loadCACert(file, file.size());
			
			#else
				// Set root certificate (from const char ROOT_CA[] PROGMEM)
				_client.setCACert(ROOT_CA);
			#endif
			
			
			// connect to host
			if (!_client.connect(_updateHost, 443)) {
		#else
			if (!_client.connect(_updateHost, 80)) {
		#endif
		
				// Error: connection failed
				(*_statusMessage) = SER_CALLHOME_FAILED;
				return false;
			}
		
		/*
		// Verify validity of server's certificate
		if (!_client.verifyCertChain(_updateHost)) {
			// (*_statusMessage): certificate verification failed!
			(*_statusMessage) = F(" Certificate verification failed!");
			return false;
		}
		*/
		
		String mode, md5;
		if(_command == U_LOGGER){
			mode = F("log");
			md5 = ESP.getSketchMD5();
		}else if(_command == U_FLASH){
			mode = F("sketch");
			md5 = ESP.getSketchMD5();
		}else if(_command == U_SPIFFS){
			mode = F("spiffs");
			md5 = ESP.getSketchMD5();
		}
		#if NEXT_OTA == true
			else if(_command == U_NEXTION){
				mode = F("nextion");
				md5 = _config->next_md5;
			}
		#endif
	
		
		// This will send the request to the server
		_client.print(String("GET ") + url + "?" + args + F(" HTTP/1.1\r\n") +
				   F("Host: ") + _updateHost + 
				   
				   F("\r\nUser-Agent: ESP-http-Update") +

				   F("\r\nx-ESP-STA-MAC: ") + WiFi.macAddress() +
				   F("\r\nx-ESP-ACT-ID: ") + _config->actCode +
				   F("\r\nx-ESP-LOCIP: ") + WiFi.localIP().toString() +
				   F("\r\nx-ESP-FREE-SPACE: ") + ESP.getFreeSketchSpace() +
				   F("\r\nx-ESP-SKETCH-SIZE: ") + ESP.getSketchSize() +


				   F("\r\nx-ESP-SKETCH-MD5: ") + md5 +
				   //F("\r\nx-ESP-FLASHCHIP-ID: ") + ESP.getFlashChipId() +			// <--- not available yet for ESP32
				   //F("\r\nx-ESP-CHIP-ID: ") + ESP.getChipId() +
				   F("\r\nx-ESP-CORE-VERSION: ") + ESP.getSdkVersion() +
				   
				   F("\r\nx-ESP-FLASHCHIP-SIZE: ") + ESP.getFlashChipSize() +
				   F("\r\nx-ESP-VERSION: ") + _config->appName + " v" + _config->appVersion +

				   F("\r\nx-ESP-MODE: ") + mode +
				   
				   F("\r\nConnection: close\r\n\r\n"));

		
		unsigned long timeout = millis();
		while (_client.available() == 0) {
			if (millis() - timeout > 5000) {
				(*_statusMessage) = SER_CALLHOME_TIMEOUT;
				_client.stop();
				
				return false;
			}
		}
		
		return true;
	}


	
	void callServer::sm(String *statusMessage){
		_statusMessage = statusMessage;
	}
#endif
