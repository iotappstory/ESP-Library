#if defined ESP8266 || defined ESP32
	#include "configServer.h"
	#include <IOTAppStory.h> 
	
	configServer::configServer(IOTAppStory &ias){
		_ias = &ias;
	}



/** config server */
void configServer::run(){
	
		bool exitConfig = false;
		
		#if CFG_STORAGE == ST_SPIFFS || CFG_STORAGE == ST_HYBRID
			if(!SPIFFS.begin()){
				#if DEBUG_LVL >= 1
					DEBUG_PRINT(F(" SPIFFS Mount Failed"));
				#endif
			}
		#endif

		#if DEBUG_LVL >= 1
			DEBUG_PRINT(SER_CONFIG_MODE);
		#endif
		
		// Start the AsyncWebServer | We use the std::unique_ptr below because the usual AsyncWebServer server(80); causes crashed on the esp32 when the run() method is finished
		server.reset(new AsyncWebServer(80));
		
		if(_ias->_connected){
			
			// when there is wifi setup server in STA mode
			WiFi.mode(WIFI_STA);
			
			#if DEBUG_LVL >= 2
				DEBUG_PRINT(SER_CONFIG_STA_MODE);
				DEBUG_PRINTLN(WiFi.localIP());
			#endif
			
		}else{
			
			// when there is no wifi setup server in AP mode
			IPAddress apIP(192, 168, 4, 1);
			
			WiFi.mode(WIFI_AP_STA);
			#if WIFI_SMARTCONFIG == true
				WiFi.beginSmartConfig();
			#endif
			WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
			WiFi.softAP(_ias->config.deviceName);
			
			#if DEBUG_LVL >= 2
				DEBUG_PRINTF_P(SER_CONFIG_AP_MODE, _ias->config.deviceName);
			#endif

		}
		

		#if CFG_STORAGE == ST_SPIFFS
			// serv SPIFFS files from the /www/ directory
			server->serveStatic("/", SPIFFS, "/www/");
		#endif
		#if CFG_STORAGE == ST_CLOUD || CFG_STORAGE == ST_HYBRID
			// serv the index page or force wifi setup
			server->on("/", HTTP_GET, [&](AsyncWebServerRequest *request){ 		hdlReturn(request, _ias->servHdlRoot()); });
		#endif
		#if CFG_PAGE_INFO == true
			// serv this device information in json format (first page in config)
			server->on("/i", HTTP_GET, [&](AsyncWebServerRequest *request){ 	hdlReturn(request, _ias->servHdlDevInfo(), F("text/json")); });
		#endif
		
		// serv the wifi scan results
		server->on("/wsc", HTTP_GET, [&](AsyncWebServerRequest *request){ 	hdlReturn(request, _ias->strWifiScan(), F("text/json")); });
		
		// save the received ssid & pass for the received APnr(i) ans serv results
		server->on("/wsa", HTTP_POST, [&](AsyncWebServerRequest *request){ 
			
			int postAPnr = 0;
			if(request->hasParam("i", true)){
				postAPnr = atoi(request->getParam("i", true)->value().c_str());
			}
			
			hdlReturn(
				request, 
				_ias->servHdlWifiSave(
					request->getParam("s", true)->value(), 
					request->getParam("p", true)->value(), 
					postAPnr
				)
			);
		});

		
		
		// save the received fingerprint and serv results
		/*#if defined  ESP8266
			server->on("/fp", HTTP_POST, [&](AsyncWebServerRequest *request){ hdlReturn(request, _ias->servHdlFngPrintSave(request->getParam("f", true)->value())); });
		#endif*/
		
		
		// serv cert scan in json format
		server->on("/csr", HTTP_GET, [&](AsyncWebServerRequest *request){
			if(request->hasParam("d")){
				hdlReturn(request, _ias->strCertScan(request->getParam("d")->value()), F("text/json")); 
			}else{
				hdlReturn(request, _ias->strCertScan(), F("text/json")); 
			}
		});
				
		// upload a file to /certupl
		server->on("/certupl", HTTP_POST, [&](AsyncWebServerRequest *request){
			request->send(200);
		}, 
			[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){

				File fsUploadFile;
				
				// First step: check file extension & open new SPIFFS file
				if(!index){
					/// Check if file has a valid extension .cer
					if(!filename.endsWith(".cer")){
						request->send(500, F("text/plain"), F("ONLY .cer files allowed\n"));
					}
					
					#if DEBUG_LVL >= 3
						DEBUG_PRINTF(" UploadStart: %s\n", filename.c_str());
					#endif
					
					/// open new SPIFFS file for writing data
					fsUploadFile = SPIFFS.open("/cert/" + filename, FILE_WRITE);            // Open the file for writing in SPIFFS (create if it doesn't exist)
					
				}else{
					/// open existing SPIFFS file for appending data
					fsUploadFile = SPIFFS.open("/cert/" + filename, FILE_APPEND);
				}
				
				// Second step: write received buffer to SPIFFS file
				if(len){
					#if DEBUG_LVL >= 3
						DEBUG_PRINT(F(" Writing:\t"));
						DEBUG_PRINTLN(len);
					#endif
					
					/// write data
					if(fsUploadFile.write(data, len) != len){
						#if DEBUG_LVL >= 3
							DEBUG_PRINTLN(F(" Write error!"));
						#endif
						
						/// if write failed return error
						request->send(500, F("text/plain"), F("Write error!\n"));
					}
				}
				
				// Last step: close file
				if(final){
					#if DEBUG_LVL >= 3
						DEBUG_PRINTF(" UploadEnd: %s, %u B\n", filename.c_str(), index+len);
					#endif
					
					/// close file
					fsUploadFile.close();
				}
			}
		);
		
		// serv the app fields in json format
		server->on("/app", HTTP_GET, [&](AsyncWebServerRequest *request){ 	hdlReturn(request, _ias->servHdlAppInfo(), F("text/json")); });

		// save the received app fields and serv results
		server->on("/as", HTTP_POST, [&](AsyncWebServerRequest *request){ hdlReturn(request, _ias->servHdlAppSave(request)); });

		// save the received device activation code
		server->on("/ds", HTTP_POST, [&](AsyncWebServerRequest *request){ hdlReturn(request, _ias->servHdlactcodeSave(request->getParam("ac", true)->value())); });
		
		// close and exit the web server
		server->on("/close", HTTP_GET, [&](AsyncWebServerRequest *request){ exitConfig = true; });
		
		
		// serv 404 page
		server->onNotFound([](AsyncWebServerRequest *request){
			request->send(404);
		});
		
		// start the server
		server->begin();

		// server loop
		while(exitConfig == false){
			
			yield();
			
			if(_ias->_connected){

				// if writeConfig bool is true write EEPROM (used by: saveWifi & saveApp
				if(_ias->_writeConfig){
					_ias->writeConfig(true);
					yield();
					_ias->_writeConfig = false;
				}
				
				// when succesfully added wifi cred in AP mode change to STA mode
				if(_ias->_connChangeMode){
					delay(1000);
					WiFi.mode(WIFI_STA);
					delay(100);
					_ias->_connChangeMode = false;
					
					#if DEBUG_LVL >= 2
						DEBUG_PRINTLN(SER_CONFIG_STA_MODE_CHANGE);
					#endif
				}
				
			}else{
				
				// smartconfig default false / off
				#if WIFI_SMARTCONFIG == true
					if(WiFi.smartConfigDone()){
						WiFi.mode(WIFI_AP_STA);
						_ias->WiFiConnectToAP();
					}
				#endif
				
				// wifi connect when asked
				if(_ias->_tryToConn == true){
						
					#if DEBUG_LVL >= 2
						DEBUG_PRINTLN(SER_CONN_REC_CRED);
					#endif
					#if DEBUG_LVL >= 3
						DEBUG_PRINTLN(F(""));
						DEBUG_PRINT(_ias->config.ssid[0]);
						DEBUG_PRINT(F(" - "));
						DEBUG_PRINTLN(_ias->config.password[0]);
					#endif
					

					WiFi.begin(_ias->config.ssid[0], _ias->config.password[0]);
					if(_ias->WiFiConnectToAP(false)){
						_ias->_connFail = false;
						
						#if DEBUG_LVL >= 3
							DEBUG_PRINT(SER_CONN_SAVE_EEPROM);
						#endif
					}else{
						_ias->_connFail = true;
						
						#if DEBUG_LVL >= 1
							DEBUG_PRINT(SER_FAILED_TRYAGAIN);
						#endif
						
					}
					
					_ias->_tryToConn = false;
					yield();
				}
			}
		}
		
		#if DEBUG_LVL >= 2
			DEBUG_PRINTLN(SER_CONFIG_EXIT);
		#endif
}



/** Handle uploads */
void configServer::onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
	if(!index){
		Serial.printf("UploadStart: %s\n", filename.c_str());
	}
	for(size_t i=0; i<len; i++){
		Serial.write(data[i]);
	}
	if(final){
		Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index+len);
	}
}



/** return page handler */
void configServer::hdlReturn(AsyncWebServerRequest *request, String retHtml, String type) {
	#if CFG_AUTHENTICATE == true
	if(!request->authenticate("admin", _ias->config.cfg_pass)){ 
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
#endif
