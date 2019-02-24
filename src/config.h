	/**
		------ ------ ------ ------ ------ ------ DEFINES for library ------ ------ ------ ------ ------ ------ 
	*/
	#include "serialFeedback_EN.h"				// language file for serial feedback

	#ifndef DEBUG_LVL
	#define DEBUG_LVL 2  // Debug level: 0 - 3 | none - max
	#endif

	#ifndef SERIAL_SPEED
	#define SERIAL_SPEED 115200
	#endif

	#ifndef BOOTSTATISTICS
	#define BOOTSTATISTICS true
	#endif

	// config 
	#ifndef INC_CONFIG
	#define INC_CONFIG 							true  // include Config mode (Wifimanager!!!)
	#endif

	#ifndef CFG_AUTHENTICATE
	#define CFG_AUTHENTICATE				false	// Set authentication | Default: admin - admin | Password can be changed when in config mode | max 16 char
	#endif

	#ifndef CFG_PAGE_INFO
	#define CFG_PAGE_INFO						true	// include the info page in Config mode
	#endif

	#ifndef CFG_PAGE_IAS
	#define CFG_PAGE_IAS 						true  // include the IAS page in Config mode
	#endif

	// Wifi defines
	#ifndef SMARTCONFIG
	#define SMARTCONFIG							false	// Set to true to enable smartconfig by smartphone app "ESP Smart Config" or "ESP8266 SmartConfig" | This will add (2%) of program storage space and 1%) of dynamic memory
	#endif

	#ifndef WIFI_MULTI
	#define WIFI_MULTI							true	// false: only 1 ssid & pass will be used | true: 3 sets of ssid & pass will be used
	#endif

	#ifndef MAX_WIFI_RETRIES
	#define MAX_WIFI_RETRIES 				20		// sets the maximum number of retries when trying to connect to the wifi
	#endif

	#ifndef USEMDNS
	#define USEMDNS 								true  // include MDNS responder http://yourboard.local
	#endif

	#ifndef DNS_PORT
	#define DNS_PORT								53
	#endif

	#ifndef UDP_PORT
	#define UDP_PORT      					514
	#endif

	#ifndef HTTPS
	#define HTTPS         					true	// Use HTTPS for OTA updates
	#endif
	
	#if defined  ESP8266
		#define EEPROM_SIZE 					4096	// EEPROM_SIZE depending on device
		#define MAXNUMEXTRAFIELDS 		8			// wifimanger | max num of fields that can be added
	#elif defined ESP32
		#define EEPROM_SIZE		 				1984
		#define MAXNUMEXTRAFIELDS 		12
	#else
		#define EEPROM_SIZE 					1024
		#define MAXNUMEXTRAFIELDS 		8
	#endif

	#define MAGICBYTES    					"CFG" 
	#define MAGICEEP      					"%"
	#define MAGICBYTE     					85
	#define RTCMEMBEGIN   					68

	// length of config variables
	#define STRUCT_CHAR_ARRAY_SIZE  50
	#define STRUCT_PASSWORD_SIZE		64		// Thankyou reibuehl
	#define STRUCT_COMPDATE_SIZE    20
	#define STRUCT_BNAME_SIZE       30
	#define STRUCT_HOST_SIZE        24
	#define STRUCT_FILE_SIZE        31

	// constants used to define the status of the mode button based on the time it was pressed. (miliseconds)
	#define MODE_BUTTON_SHORT_PRESS       500
	#define MODE_BUTTON_LONG_PRESS        4000
	#define MODE_BUTTON_VERY_LONG_PRESS   10000