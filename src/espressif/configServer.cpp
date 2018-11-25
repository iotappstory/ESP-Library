#if defined ESP8266 || defined ESP32
	#include "configServer.h"
	#include <IOTAppStory.h> 
	
	configServer::configServer(IOTAppStory &ias){
		_ias = &ias;
	}



/** config server */
void configServer::run() {
	//IOTAppStory IAS(COMPDATE, MODEBUTTON);                      // Initialize IOTAppStory
	
	bool exitConfig = false;

	#if CFG_STORAGE == SPIFSS
	if(!SPIFFS.begin()){
		#if DEBUG_LVL >= 1
			DEBUG_PRINT(F(" SPIFFS Mount Failed"));
		#endif
	}
	#endif

	#if DEBUG_LVL >= 1
		DEBUG_PRINT(SER_CONFIG_MODE);
	#endif
	
	AsyncWebServer server(80);
	
	if(WiFi.status() != WL_CONNECTED){
		
		// when there is no wifi setup server in AP mode
		IPAddress apIP(192, 168, 4, 1);
		//dnsServer.reset(new DNSServer());
		//server.reset(new AsyncWebServer(80));
		

		WiFi.mode(WIFI_AP_STA);
		#if SMARTCONFIG == true
			WiFi.beginSmartConfig();
		#endif
		WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
		WiFi.softAP(_ias->config.deviceName);
		
		//dnsServer->start(DNS_PORT, "*", apIP);
		
		#if DEBUG_LVL >= 2
			DEBUG_PRINTF_P(SER_CONFIG_AP_MODE, _ias->config.deviceName);
		#endif
		
	}else{
		
		// notifi IAS & enduser this device went to config mode (also sends localIP)
		//iasLog("1");
		
		// when there is wifi setup server in STA mode
		//server.reset(new AsyncWebServer(80));
		//AsyncWebServer server(80);
		WiFi.mode(WIFI_STA);
		
		#if DEBUG_LVL >= 2
			DEBUG_PRINT(SER_CONFIG_STA_MODE);
			DEBUG_PRINTLN(WiFi.localIP());
		#endif
		
	}
	

	
	
	#if CFG_STORAGE == SPIFSS
		// serv SPIFFS files from the /www/ directory
		server.serveStatic("/", SPIFFS, "/www/");
	#endif
	#if CFG_STORAGE == CLOUD || CFG_STORAGE == HYBRID
		// serv the index page or force wifi setup
		server.on("/", HTTP_GET, [&](AsyncWebServerRequest *request){ 		hdlReturn(request, _ias->servHdlRoot()); });
	#endif
	
	#if CFG_PAGE_INFO == true
		// serv this device information in json format (first page in config)
		server.on("/i", HTTP_GET, [&](AsyncWebServerRequest *request){ 	hdlReturn(request, _ias->servHdlDevInfo(), F("text/json")); });
	#endif
	
	// serv the wifi scan results
	server.on("/wsc", HTTP_GET, [&](AsyncWebServerRequest *request){ 	hdlReturn(request, _ias->strWifiScan(), F("text/json")); });

	// serv the app fields in json format
	server.on("/app", HTTP_GET, [&](AsyncWebServerRequest *request){ 	hdlReturn(request, _ias->servHdlAppInfo(), F("text/json")); });

	
	
	// save the received fingerprint and serv results
	#if defined  ESP8266
		server.on("/fp", HTTP_POST, [&](AsyncWebServerRequest *request){ hdlReturn(request, _ias->servHdlFngPrintSave(request->getParam("f", true)->value())); });
	#endif
	
	// save the received ssid & pass for the received APnr(i) ans serv results
	server.on("/wsa", HTTP_POST, [&](AsyncWebServerRequest *request){ 
		
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
	
	// save the received app fields and serv results
	server.on("/as", HTTP_POST, [&](AsyncWebServerRequest *request){ hdlReturn(request, _ias->servHdlAppSave(request)); });

	// save the received device activation code
	server.on("/ds", HTTP_POST, [&](AsyncWebServerRequest *request){ hdlReturn(request, _ias->servHdlactcodeSave(request->getParam("ac", true)->value())); });
	
	// close and exit the web server
	server.on("/close", HTTP_GET, [&](AsyncWebServerRequest *request){ exitConfig = true; });
	
	
	// serv 404 page
	server.onNotFound([](AsyncWebServerRequest *request){
		request->send(404);
	});
	
	
	
	// start the server
	server.begin();

	// server loop
	while(exitConfig == false){
		
		yield();
		
		if(WiFi.status() != WL_CONNECTED){
			
			//DNS
			//dnsServer->processNextRequest();
			
			// smartconfig default false / off
			#if SMARTCONFIG == true
				if(WiFi.smartConfigDone()){
					WiFi.mode(WIFI_AP_STA);
					isNetworkConnected();
				}
			#endif
			
			// wifi connect when asked
			if(_ias->_tryToConn == true){
				#if DEBUG_LVL >= 3
					DEBUG_PRINT(SER_REC_CREDENTIALS);
					DEBUG_PRINT(_ias->config.ssid[0]);
					DEBUG_PRINT(F(" - "));
					DEBUG_PRINTLN(_ias->config.password[0]);
				#endif
				

				WiFi.begin(_ias->config.ssid[0], _ias->config.password[0]);
				_ias->_connected = _ias->isNetworkConnected(false);
				yield();
				
				if(_ias->_connected){
					
					// Saving config to eeprom
					_ias->_writeConfig = true;
					
					#if DEBUG_LVL >= 3
						DEBUG_PRINT(SER_CONN_SAVE_EEPROM);
					#endif
				}else{
					_ias->readConfig();
					#if DEBUG_LVL >= 1
						DEBUG_PRINTLN(SER_FAILED_TRYAGAIN);
					#endif
					_ias->_tryToConnFail = true;
				}
				
				_ias->_tryToConn = false;
			}
			
		}else{

			// write EEPROM
			if(_ias->_writeConfig){
				_ias->writeConfig();
				yield();
				_ias->_writeConfig = false;
			}
			
			// when succesfully added wifi cred in AP mode change to STA mode
			if(_ias->_changeMode){
				delay(1000);
				WiFi.mode(WIFI_STA);
				delay(100);
				_ias->_changeMode = false;
				
				// notifi IAS & enduser this device went to config mode (also sends localIP)
				//iasLog("1");
				
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
	//iasLog("0");
				
	// Return to Normal Operation
	_ias->espRestart('N');

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