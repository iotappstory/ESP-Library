#ifdef ESP32
	#include <rom/spi_flash.h>					// enable flash chip id from the SDK
	#include <WiFi.h>
	#include <WiFiMulti.h>
	#include "espressif/WiFiConnector.h"
	#include <ESPmDNS.h>

	#include "espressif/esp32/boardInfo.h"
	#include "espressif/esp32/callServer_WiFiClientSecure.h"
	#include "espressif/configServer.h"
	
	#include "espressif/UpdateClassVirt.h"
	#include <Update.h>
	#include "espressif/updateESP.h"
	#if OTA_UPD_CHECK_NEXTION == true
		#include "espressif/updateNextion.h"
	#endif
	
	#include <HTTPClient.h>
	#include <DNSServer.h> 
	#include <EEPROM.h>
	#include <ESPAsyncWebServer.h>              // https://github.com/me-no-dev/ESPAsyncWebServer
#endif


#ifdef  ESP8266
	#include <ESP8266WiFi.h>
	#include <ESP8266WiFiMulti.h>
	#include "espressif/WiFiConnector.h"
	#include <ESP8266mDNS.h>

	#include "espressif/esp8266/boardInfo.h"
	#include "espressif/esp8266/callServer_WiFiClientSecure.h"
	#include "espressif/configServer.h"
	
	#include "espressif/UpdateClassVirt.h"
	#include "espressif/updateESP.h"
	#if OTA_UPD_CHECK_NEXTION == true
		#include "espressif/updateNextion.h"
	#endif
	
	#include <ESP8266HTTPClient.h>
	#include <DNSServer.h> 
	#include <EEPROM.h>
	#include <ESPAsyncWebServer.h>				// https://github.com/me-no-dev/ESPAsyncWebServer
#endif


#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
	//#WIFI_MULTI false
	#include <SPI.h>
	#include <WiFiNINA.h>
	//#include <WiFiMDNSResponder.h>
	//#include <stlport.h>
	//#include <type_traits>
	
	#include <stdio.h>
	
	//#include "ardmkr/boardInfo.h"
	//#include "ardmkr/otaUpdate.h"
	
	#include <FlashAsEEPROM.h>
#endif
