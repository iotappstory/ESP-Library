#ifdef ESP8266
	#ifndef callServerBearSSL_h
		#define callServerBearSSL_h

		#include <IOTAppStory.h>
		#include <ESP8266WiFi.h>
		
		#if HTTPS == true
			#include <WiFiClientSecureBearSSL.h>
			#include <time.h>
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
					BearSSL::WiFiClientSecure _client;
				#else
					WiFiClient _client;
				#endif
				String *_statusMessage;
				
				strConfig* _config;
				int _command;
				const char* _callHost		= OTA_HOST;         // ota update host | Set in config.h
				const char* _callFile		= OTA_UPD_FILE; 	// file at host that handles esp updates
		};
	#endif
#endif
