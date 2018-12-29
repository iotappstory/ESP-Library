#ifdef  ESP32
	#include "otaUpdate.h"
	#include <IOTAppStory.h>								// IAS.com library
	#include <WiFiClientSecure.h>
	#include <StreamString.h>
	#include <Update.h>
	
	otaUpdate::otaUpdate(strConfig &config, int command){
		_config = &config;
		_command = command;
	}


	
	bool otaUpdate::getUpdate(){
		
		#if DEBUG_LVL >= 2
			#if HTTPS == true
				DEBUG_PRINT(F("https://"));
			#else
				DEBUG_PRINT(F("http://"));
			#endif
			DEBUG_PRINT(_updateHost);
			DEBUG_PRINTLN(_updateFile);
		#endif
		
		#if DEBUG_LVL == 1
			DEBUG_PRINTLN("");		
		#endif
		
		// set root certificate
		_client.setCACert(ROOT_CA);
		
		if (!_client.connect(_updateHost, 443)) {
			// Error: connection failed
			error = SER_CALLHOME_FAILED;
			return false;
		}
		
		/*
		// Verify validity of server's certificate
		if (!_client.verifyCertChain(_updateHost)) {
			// ERROR: certificate verification failed!
			error = F(" ERROR: certificate verification failed!");
			return false;
		}
		
		
		if(!_client.verify(_config->sha1, _updateHost)){
			//ERROR: certificate verification failed!
			error = SER_CALLHOME_CERT_FAILED;
			return false;
		}
		*/
		
		String mode, md5;
		if(_command == U_FLASH){
			mode = "sketch";
			md5 = ESP.getSketchMD5();
		}else if(_command == U_SPIFFS){
			mode = "spiffs";
			md5 = ESP.getSketchMD5();
		}else if(_command == U_NEXTION){
			mode = "nextion";
			if(_config->next_md5 == ""){
				md5 = "00000000000000000000000000000000";
			}else{
				md5 = _config->next_md5;
			}
		}	

		// This will send the request to the server
		_client.print(String("GET ") + _updateFile + F(" HTTP/1.1\r\n") +
				   F("Host: ") + _updateHost + F("\r\nUser-Agent: ESP-http-Update") +

				   F("\r\nx-ESP-STA-MAC: ") + WiFi.macAddress() +
				   F("\r\nx-ESP-ACT-ID: ") + _config->actCode +
				   F("\r\nx-ESP-LOCIP: ") + WiFi.localIP().toString() +
				   F("\r\nx-ESP-FREE-SPACE: ") + ESP.getFreeSketchSpace() +
				   F("\r\nx-ESP-SKETCH-SIZE: ") + ESP.getSketchSize() +


				   F("\r\nx-ESP-SKETCH-MD5: ") + md5 +
				   
				   //F("\r\nx-ESP-FLASHCHIP-ID: ") + ESP.getFlashChipId() +
				   //F("\r\nx-ESP-CHIP-ID: ") + ESP.getChipId() +
				   F("\r\nx-ESP-CORE-VERSION: ") + ESP.getSdkVersion() +
				   
				   F("\r\nx-ESP-FLASHCHIP-SIZE: ") + ESP.getFlashChipSize() +
				   F("\r\nx-ESP-VERSION: ") + _config->appName + " v" + _config->appVersion +

				   F("\r\nx-ESP-MODE: ") + mode +
				   
				   F("\r\nConnection: close\r\n\r\n"));

		
		unsigned long timeout = millis();
		while (_client.available() == 0) {
			if (millis() - timeout > 5000) {
				error = SER_CALLHOME_TIMEOUT;
				_client.stop();
				return false;
			}
		}
		
		
		
		_totalSize = 0;
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
					error = line;
					
					return false;
				}else{
					#if DEBUG_LVL >= 1
						DEBUG_PRINT(SER_DOWNLOADING);
					#endif
				}

			}else if(line.startsWith(F("Content-Length: "))) {
				line.remove(0, 16);
				_totalSize = line.toInt();

			} else if(line.startsWith(F("x-name: "))) {
				line.remove(0, 8);
				line.trim();
				_xname = line;

			} else if(line.startsWith(F("x-ver: "))) {
				line.remove(0, 7);
				line.trim();
				_xver = line;

			}else if(line.startsWith(F("x-MD5: "))) {
				line.remove(0, 7);
				line.trim();
				_xmd5 = line;

			}else if(line == "\r") {
				break;
			} 
		}
		/* */
		#if DEBUG_LVL >= 3
			DEBUG_PRINTLN(F("\n Extracted from header"));
			DEBUG_PRINTLN(F(" ---------------------"));
			DEBUG_PRINT(F(" Content-Length: "));
			DEBUG_PRINTLN(_totalSize);
			DEBUG_PRINT(F(" x-name: "));
			DEBUG_PRINTLN(_xname);
			DEBUG_PRINT(F(" x-ver: "));
			DEBUG_PRINTLN(_xver);
			DEBUG_PRINT(F(" x-md5: "));
			DEBUG_PRINTLN(_xmd5);
			DEBUG_PRINTLN(F(" ---------------------\n"));
		#endif
		
		if(code == 200 && _totalSize > 0 && _xname != "" && _xver != "" && _xmd5 != ""){
			return true;
		}else{
			error = SER_CALLHOME_HDR_FAILED;
			return false;
		}
	}


	
	bool otaUpdate::install(){
		
		DEBUG_PRINT(SER_INSTALLING);
		#if DEBUG_LVL == 3
		  StreamString stError;
		#endif

		/**
			Get free sketch / SPIFFS space
		*/
		int freeSpace;
		
		if(_command == U_FLASH){
			// current FreeSketchSpace
			freeSpace = ESP.getFreeSketchSpace();
			
			/**
				Check if there is enough free space for the received sketch / SPIFFS "file"
			*/
			if(_totalSize > freeSpace){
				#if DEBUG_LVL >= 3
					DEBUG_PRINTF_P(PSTR(" Error: Not enough space (%d) update size: %d"), freeSpace, _totalSize);
				#endif
				
				#if DEBUG_LVL >= 2
					error = F("Error: Not enough space");
				#endif
				return false;
			}
		}
		
		
		/* this will come with the next esp32 core release
		else if(_command == U_SPIFFS){ not yet supported for the ESP32 
			// current SPIFFS free space
			freeSpace = ((size_t) &_SPIFFS_end - (size_t) &_SPIFFS_start);
			
		}else if(_command == U_NEXTION){
			// mount SPIFFS and get free space
			if (!SPIFFS.begin()) {
				error = F("Error: Could not mount SPIFFS");
				return false;
			}
			
			freeSpace = SPIFFS.totalBytes() - SPIFFS.usedBytes();
		}
		*/

		#if NEXT_OTA == true
			if(_command == U_NEXTION){
				return installNEXTION();
			}else{
				return installESP();
			}
		#else
			return installESP();
		#endif
	}

	
	
	bool otaUpdate::installNEXTION(){
		
		ESPNexUpload nextion(NEXT_BAUD);
		
		// what to do during update progress *optional!
		nextion.setUpdateProgressCallback([](){
			#if DEBUG_LVL >= 1
				DEBUG_PRINT(F("."));
			#endif
		});

		// if nextion update failed return false & error
		if(!nextion.prepairUpload(_totalSize)){
			error = "Error: " + nextion.statusMessage;
	 
			return false;
		}

		// if nextion update failed return false & error
		if(!nextion.upload(_client)){
			error = "Error: " + nextion.statusMessage;
	 
			return false;
		}

		nextion.end();

		
		// on succesfull firmware installation
		#if DEBUG_LVL >= 2
			DEBUG_PRINT(F(" Updated Nextion to: "));
			DEBUG_PRINTLN(_xname+" v"+ _xver);
		#endif
		
		// update nextion md5
		_xmd5.toCharArray(_config->next_md5, 33);
		
		return true;
	}

	
	
	bool otaUpdate::installESP(){
		/**
			Initiate the esp update class
		*/
		if(!Update.begin(_totalSize, _command)) {
			#if DEBUG_LVL == 3
				Update.printError(stError);
				stError.trim();
				
				DEBUG_PRINT(F("\n Error: Update.begin failed! "));
				DEBUG_PRINTLN(stError.c_str());
			#endif

			#if DEBUG_LVL >= 2
				error = F(" Error: Update.begin failed! ");
			#endif
			
			return false;
		}
	  

		/**
			add and check the received md5 string
		*/
		if(_xmd5.length()) {
			
			if(!Update.setMD5(_xmd5.c_str())) {
				#if DEBUG_LVL == 3
					DEBUG_PRINTLN(" Error: Update.setMD5 failed! ");
					DEBUG_PRINT(" Received md5 string: ");
					DEBUG_PRINTLN(_xmd5.c_str());
				#endif
				
				#if DEBUG_LVL >= 2
					error = F("Error: Update.setMD5 failed!");
				#endif
			
				return false;
			}
		}
		

		/**
			write the received sketch / SPIFFS "file" to memory
		*/
		uint32_t written, totalDone = 0;
		
		while (!Update.isFinished()) {
			
			written = Update.writeStream(_client);
			
			if (written > 0) {
				totalDone += written;
				
				// progress callback
				/*
				if(_progress_callback) {
				  _progress_callback(totalDone, _totalSize);
				}
				*/
				#if DEBUG_LVL == 3
					int perc   = totalDone / (_totalSize / 100);
					
					DEBUG_PRINT(perc);
					DEBUG_PRINT(F("% | "));
					DEBUG_PRINT(totalDone);
					DEBUG_PRINT(F(" of "));
					DEBUG_PRINT(_totalSize);
					DEBUG_PRINTLN(F(" done\n"));
				#endif
			}else{
			  DEBUG_PRINT(F("."));
			}
		}


		/**
			End the esp update class
			If the update process end in an error print it
		*/
		if(!Update.end()) {
			
			// on failed firmware installation
			#if DEBUG_LVL == 3
			  Update.printError(stError);
			  stError.trim();
			  
			  DEBUG_PRINT(F(" Error: Update.end failed!\n"));
			  DEBUG_PRINTLN(stError.c_str());
			#endif
			
			#if DEBUG_LVL >= 2
				error = F("Error: Update.end failed!");
			#endif
			
			return false;
		}else{
			
			// on succesfull firmware installation
			#if DEBUG_LVL >= 2
				DEBUG_PRINT(F(" Updated to: "));
				DEBUG_PRINTLN(_xname+" v"+ _xver);
			#endif
			

			if(_command == U_FLASH){
				// store received appName & appVersion
				strcpy(_config->appName,  _xname.c_str());
				strcpy(_config->appVersion,  _xver.c_str());
			}
			
			_client.stop();
			return true;
		}
	}
#endif
