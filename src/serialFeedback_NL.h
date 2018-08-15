	/**
		------ ------ ------ ------ ------ ------ DEFINED SERIAL FEEDBACK ------ ------ ------ ------ ------ ------ 
	*/
	#define SER_ERASE_FULL							" EEPROM volledig wissen"
	#define SER_ERASE_PART							" EEPROM deels wissen"
	#define SER_ERASE_PART_EXT					" EEPROM deels wissen, behoud config settings"
	#define SER_ERASE_NONE							" Laat EEPROM intact"
	#define SER_ERASE_FLASH							" Erasing Flash...\n From %4d to %4d\n"
	
	#define SER_CALLBACK_FIRST_BOOT			" Run first boot callback"
	#define SER_START										" Start "
	#define SER_SAVE_CONFIG							"Save config presets...\n"
	#define SER_MODE_SEL_BTN						" Mode select knop: GPIO%d\n Boardname: %s\n Update on boot: %d\n"
	
	#define SER_BOOTTIMES_UPDATE				" bootTimes sinds laatste update: %d\n boardMode: %c\n"
	#define SER_BOOTTIMES_POWERUP				" rtcMem\n bootTimes sinds powerup: %d\n boardMode: %c\n"
	
	#define SER_UPDATE_IASLOG						" update iasLog"
	#define SER_FAILED_COLON						" Mislukt: "
	#define SER_FAILED_TRYAGAIN					"\n Mislukt, Probeer nogmaals!!!"
	
	#define SER_CONFIG_MODE							"\n\n\n\n C O N F I G U R A T I E   M O D U S\n"
	#define SER_CONFIG_AP_MODE					" AP mode. Verbinden met Wifi AP \"%s\"\n En open 192.168.4.1\n"
	#define SER_CONFIG_STA_MODE					" STA mode. Open "
	#define SER_CONFIG_STA_MODE_CHANGE	"\n Veranderd naar STA mode. Open "
	#define SER_CONFIG_EXIT							" Exit config"
	#define SER_CONFIG_ENTER						" Start Configuratie Modus"   
	
	#define SER_REC_ACT_CODE						"\n Activatie code van browser ontvangen"
	#define SER_REC_CREDENTIALS					"Credentials ontvangen: "

	#define SER_CONN_SAVE_EEPROM				" Verbonden. Opslaan config naar eeprom"
	#define SER_CONNECTING							" Verbinden met WiFi AP"
	#define SER_CONN_NONE_GO_CFG				"\n Geen verbinding. Ga in Configuratie Modus\n"
	#define SER_CONN_NONE_CONTINU				"\n WiFi NIET verbonden. Toch verder\n" 
	#define SER_CONNECTED								"\n WiFi verbonden\n"
	
	#define SER_DEV_MAC									" Apparaat MAC: "
	#define SER_DEV_IP									" Apparaat IP Address: "
	#define SER_DEV_MDNS								" MDNS responder gestart: http://"
	#define SER_DEV_MDNS_INFO						"\n\n Om mDNS te gebruiken Instaleer host software:\n - Voor Linux, instaleer Avahi (http://avahi.org/)\n - Voor Windows, instaleer Bonjour (https://commaster.net/content/how-resolve-multicast-dns-windows)\n - Voor Mac OSX en iOS is al ondersteuning via Bonjour aanwezig"
	#define SER_DEV_MDNS_FAIL						" MDNS responder start mislukt"
	
	#define SER_CALLING_HOME						" Calling Home"  
	#define SER_CALLHOME_FAILED					" Mislukt. Server onbereikbaar OF HTTPS probleem"
	#define SER_RET_FROM_IAS						"\n Terug van IOTAppStory.com"
	#define SER_CHECK_FOR								" Checken voor "
	#define SER_APP_SKETCH							"App(Sketch)"
	#define SER_SPIFFS									"SPIFFS"
	#define SER_UPDATES_FROM						" updates van: "
	#define SER_UPDATES									" updates"
	#define SER_DOWN_AND_PROC						" Download & Process update..."
	#define SER_REBOOT_NEC							" Reboot noodzakelijk!"
	
	#define SER_PROC_FIELDS							" Verwerken toegevoegde velden"
	#define SER_PROC_TBL_HDR						"\n ID | LABEL                          | LEN |  EEPROM LOC  | DEFAULT WAARDE                  | HUIDIGE WAARDE                  | STATUS\n"
	#define SER_PROC_TBL_OVRW						"%-30s | OVERSCHREVEN"
	#define SER_PROC_TBL_DEF						"%-30s | DEFAULT"
	#define SER_PROC_TBL_WRITE					"%-30s | SCHRIJVEN NAAR EEPROM"
	
	#define SER_EEPROM_FOUND						" EEPROM Configuratie gevonden"
	#define SER_EEPROM_NOT_FOUND				" EEPROM Configuratie NIET GEVONDEN!!!!"
	
	
	#define SER_SERV_DEV_INFO						" Serveer apparaat info"
	#define SER_SERV_WIFI_SCAN_RES			" Serveer resultaat van Wifiscan"
	#define SER_SERV_APP_SETTINGS				" Serveer App Settings"
	
	#define SER_SAVE_APP_SETTINGS				" Opslaan App Settings"
	#define SER_SAVE_FINGERPRINT				" Opslaan Fingerprint"
	
	
	#define SER_CONN_REC_CRED						"\n verbinden met ontvangen credentials"
	#define SER_CONN_REC_CRED_DB3				" \n verbinden met ontvangen credentials: %s - %s\n"
	#define SER_CONN_REC_CRED_PROC			"\n Verwerken van ontvangen credentials"
	#define SER_CONN_ADDED_AP_CRED			"\n Wifi credentials toegevoegd voor AP%d\n"
	#define SER_CONN_CRED_MISSING				"SSID of Wachtwoord is leeg"
