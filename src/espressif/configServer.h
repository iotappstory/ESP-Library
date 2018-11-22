#if defined ESP8266 || defined ESP32
	#ifndef configServer_h
		#define configServer_h
		
		#include <IOTAppStory.h> 
		#ifdef ESP32
			#include <AsyncTCP.h>                    	// https://github.com/me-no-dev/AsyncTCP
			#include <FS.h>								// esp32 core SPIFFS library
			#include <SPIFFS.h>	
		#elif defined  ESP8266
			#include <ESPAsyncTCP.h>					// https://github.com/me-no-dev/ESPAsyncTCP
			#include <FS.h>								// esp8266 core SPIFFS library
		#endif
		#include <ESPAsyncWebServer.h>					// https://github.com/me-no-dev/ESPAsyncWebServer
		
		struct strConfig;
		class IOTAppStory;
		
		
		class configServer {
			public:
				configServer(IOTAppStory &ias);
				void run();
			
			private:
				IOTAppStory* _ias;
				
				void hdlReturn(AsyncWebServerRequest *request, String retHtml, String type = "text/html");
				
		};
	#endif
#endif
