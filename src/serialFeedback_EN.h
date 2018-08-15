	/**
		------ ------ ------ ------ ------ ------ DEFINED SERIAL FEEDBACK ------ ------ ------ ------ ------ ------ 
	*/
	#define SER_ERASE_FULL							" Full erase of EEPROM"
	#define SER_ERASE_PART							" Partial erase of EEPROM"
	#define SER_ERASE_PART_EXT					" Partial erase of EEPROM but leaving config settings"
	#define SER_ERASE_NONE							" Leave EEPROM intact"
	#define SER_ERASE_FLASH							" Erasing Flash...\n From %4d to %4d\n"
	
	#define SER_CALLBACK_FIRST_BOOT			" Run first boot callback"
	#define SER_START										" Start "
	#define SER_SAVE_CONFIG							"Saving config presets...\n"
	#define SER_MODE_SEL_BTN						" Mode select button: GPIO%d\n Boardname: %s\n Update on boot: %d\n"
	
	#define SER_BOOTTIMES_UPDATE				" bootTimes since last update: %d\n boardMode: %c\n"
	#define SER_BOOTTIMES_POWERUP				" rtcMem\n bootTimes since powerup: %d\n boardMode: %c\n"
	
	#define SER_UPDATE_IASLOG						" update iasLog"
	#define SER_FAILED_COLON						" Failed: "
	#define SER_FAILED_TRYAGAIN					"\n Failed, try again!!!"
	
	#define SER_CONFIG_MODE							"\n\n\n\n C O N F I G U R A T I O N   M O D E\n"
	#define SER_CONFIG_AP_MODE					" AP mode. Connect to Wifi AP \"%s\"\n And open 192.168.4.1\n"
	#define SER_CONFIG_STA_MODE					" STA mode. Open "
	#define SER_CONFIG_STA_MODE_CHANGE	"\n Changed to STA mode. Open "
	#define SER_CONFIG_EXIT							" Exit config"
	#define SER_CONFIG_ENTER						" Entering in Configuration Mode"
	
	#define SER_REC_ACT_CODE						"\n Received activation code from browser"
	#define SER_REC_CREDENTIALS					"Received cred: "

	#define SER_CONN_SAVE_EEPROM				" Connected. Saving config to eeprom"
	#define SER_CONNECTING							" Connecting to WiFi AP"
	#define SER_CONN_NONE_GO_CFG				"\n No Connection. Going into Configuration Mode\n"
	#define SER_CONN_NONE_CONTINU				"\n WiFi NOT connected. Continuing anyway\n"
	#define SER_CONNECTED								"\n WiFi connected\n"
	
	#define SER_DEV_MAC									" Device MAC: "
	#define SER_DEV_IP									" Device IP Address: "
	#define SER_DEV_MDNS								" MDNS responder started: http://"
	#define SER_DEV_MDNS_INFO						"\n\n To use mDNS Install host software:\n - For Linux, install Avahi (http://avahi.org/)\n - For Windows, install Bonjour (https://commaster.net/content/how-resolve-multicast-dns-windows)\n - For Mac OSX and iOS support is built in through Bonjour already"
	#define SER_DEV_MDNS_FAIL						" MDNS responder failed to start"
	
	#define SER_CALLING_HOME						" Calling Home"
	#define SER_CALLHOME_FAILED					" Failed. Lost Wifi, Server down OR HTTPS problems"
	#define SER_RET_FROM_IAS						"\n Returning from IOTAppStory.com"
	#define SER_CHECK_FOR								" Checking for "
	#define SER_APP_SKETCH							"App(Sketch)"
	#define SER_SPIFFS									"SPIFFS"
	#define SER_UPDATES_FROM						" updates from: "
	#define SER_UPDATES									" updates"
	#define SER_DOWN_AND_PROC						" Download & Process update..."
	#define SER_REBOOT_NEC							" Reboot necessary!"
	
	#define SER_PROC_FIELDS							" Processing added fields"
	#define SER_PROC_TBL_HDR						"\n ID | LABEL                          | LEN |  EEPROM LOC  | DEFAULT VALUE                  | CURRENT VALUE                  | STATUS\n"
	#define SER_PROC_TBL_OVRW						"%-30s | OVERWRITTEN"
	#define SER_PROC_TBL_DEF						"%-30s | DEFAULT"
	#define SER_PROC_TBL_WRITE					"%-30s | WRITTING TO EEPROM"
	
	#define SER_EEPROM_FOUND						" EEPROM Configuration found"
	#define SER_EEPROM_NOT_FOUND				" EEPROM Configuration NOT FOUND!!!!"
	
	
	#define SER_SERV_DEV_INFO						" Serving device info"
	#define SER_SERV_WIFI_SCAN_RES			" Serving results of Wifiscan"
	#define SER_SERV_APP_SETTINGS				" Serving App Settings"
	
	#define SER_SAVE_APP_SETTINGS				" Saving App Settings"
	#define SER_SAVE_FINGERPRINT				" Saving Fingerprint"
	
	
	#define SER_CONN_REC_CRED						"\n Connect with received credentials"
	#define SER_CONN_REC_CRED_DB3				" \n Connect with received credentials: %s - %s\n"
	#define SER_CONN_REC_CRED_PROC			"\n Processing received credentials"
	#define SER_CONN_ADDED_AP_CRED			"\n Added wifi credentials for AP%d\n"
	#define SER_CONN_CRED_MISSING				"SSID or Password is missing"
