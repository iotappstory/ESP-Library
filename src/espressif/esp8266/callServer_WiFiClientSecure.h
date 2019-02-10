#ifdef  ESP8266
	#ifndef callServer_h
		#define callServer_h

		#include <IOTAppStory.h>
		#include <ESP8266WiFi.h>
		
		#if HTTPS == true
			#include <WiFiClientSecureBearSSL.h>
		#endif

		#if NEXT_OTA == true
			#define U_NEXTION 300
		#endif
		
		#define U_LOGGER 400

		struct strConfig;
		struct firmwareStruct;
		
		
		class callServer{
			
			public:
				callServer(strConfig &config, int command = U_FLASH);
				Stream &getStream(firmwareStruct *firmwareStruct);
				
				bool get(const char* url, String args);
				
				void sm(String *statusMessage);
				
			private:
				#if HTTPS == true
					WiFiClientSecure _client;
				#else
					WiFiClient _client;
				#endif
				String *_statusMessage;
				
				strConfig* _config;
				int _command;													// total size of received update
				const char* _updateHost		= "iotappstory.com";         	// ota update host
				const char* _updateFile		= "/ota/esp8266-v2.1.0.php"; 	// file at host that handles 8266 updates
		};
	#endif
#endif
