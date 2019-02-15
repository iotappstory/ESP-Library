#ifdef  ESP32
	#ifndef callServer_h
		#define callServer_h

		#include <IOTAppStory.h>
		
		#if HTTPS == true
			#include <WiFiClientSecure.h>
		#endif

		#if NEXT_OTA == true
			#define U_NEXTION 300
		#endif
		
		#define U_FLASH		0
		#define U_SPIFFS	100
		#define U_AUTH		200
		#define U_LOGGER	400
		
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
				int _command;												// command sent
				const char* _updateHost		= "iotappstory.com";         	// ota update host
				const char* _updateFile		= "/ota/esp32-v2.php"; 			// file at host that handles esp updates
		};
	#endif
#endif
