/*                          =======================
============================   C/C++ HEADER FILE   ============================
                            =======================                       *//**
  config.h

  Created by Onno Dirkzwager on 05.08.2018.
  Copyright (c) 2018 IOTAppStory. All rights reserved.

*///===========================================================================

/*---------------------------------------------------------------------------*/
/*                                    INCLUDES                               */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                            DEFINITIONS AND MACROS                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        TYPEDEFS, CLASSES AND STRUCTURES                   */
/*---------------------------------------------------------------------------*/
/**
    ------ ------ ------ ------ ------ ------ user DEFINES for library ------ ------ ------ ------ ------ ------
*/
#include "serialFeedback_EN.h"                  // language file for serial feedback currently available EN & NL

#define DEBUG_LVL                   2           // Debug level: 0 - 3 | none - max
#define DEBUG_EEPROM_CONFIG         false
#define DEBUG_FREE_HEAP             false
#define SERIAL_SPEED                115200
#define BOOTSTATISTICS              true

// config
#define CFG_INCLUDE                 true        // include Config mode (Wifimanager!!!)
#define CFG_STORAGE                 ST_CLOUD    // ST_CLOUD / ST_SPIFSS / ST_HYBRID
#define CFG_AUTHENTICATE            false       // Set authentication
#define CFG_PASS                    "admin"     // initial authentication password. You edit & change this in config mode. | max 16 char
#define CFG_PAGE_INFO               true        // include the info page in Config mode
#define CFG_PAGE_IAS                true        // include the IAS page in Config mode
#define CFG_ANNOUNCE                true        // Announce to IAS on which ip this device is during Config mode.
#define CFG_TIMEOUT                 300000       // Exit Config mode after X time of inactivity.

// Wifi defines
#define WIFI_SMARTCONFIG            false       // Set to true to enable smartconfig by smartphone app "ESP Smart Config" or "ESP8266 SmartConfig" | This will add (+/- 2%) of program storage space and +/- 1%) of dynamic memory
#define WIFI_MULTI                  true        // false: only 1 ssid & pass will be used | true: 3 sets of ssid & pass will be used
#define WIFI_MULTI_MAX              3           // Max nr of addable WiFi AP's | If you go for more than 3 AP's make sure to change CFG_EEP_START_ADDR(line 94) & FIELD_EEP_START_ADDR(line 95)
#define WIFI_MULTI_FORCE_RECONN_ANY false       // By default wifi multi will only try to reconnect to the last AP it was connectected to. Setting this to true will force your esp to connect to any of the available AP's from the list.
#define WIFI_CONN_MAX_RETRIES       20          // sets the maximum number of retries when trying to connect to the wifi
#define WIFI_USE_MDNS               true        // include MDNS responder http://yourboard.local
#define WIFI_DHCP_ONLY              true        // true = DHCP only, false = 1 x Static ip address | WiFiMulti does not support STATIC ip addresses
#define WIFICONNECTOR_DEBUG         false

// Internal clock
#if defined  ESP8266
   #define SNTP_INT_CLOCK_UPD           true            // Synchronize the internal clock useing SNTP? BearSSL: This is necessary to verify that the TLS certificates offered by servers are currently valid.
   #define SNTP_INT_CLOCK_TIME_ZONE     TZ_Etc_GMT      // Ntp time zone
   #define SNTP_USE_STORED_UNTILL_SYNC  false           // Use the last stored timestamp untill time is synced with the NTP server (faster reboots..)(ESP8266 only)
#elif defined ESP32
   #define SNTP_INT_CLOCK_UPD           false           // The esp32 uses mbedTLS instead of BearSSL and does not need the time. Your welcome to turn it on for your own projects!
   #define SNTP_INT_CLOCK_TIME_ZONE     PSTR("GMT0")    // Ntp time zone
#endif


#define SNTP_INT_CLOCK_SERV_1       "pool.ntp.org"      // Ntp server 1
#define SNTP_INT_CLOCK_SERV_2       "time.nist.gov"     // Ntp server 2
#define SNTP_CONN_MAX_RETRIES       200                 // NTP sync is important! But we cannot wait forever...

// HTTPS defines
#define HTTPS                       true                // Use HTTPS for OTA updates
#define HTTPS_8266_TYPE             CERTIFICATE         // FNGPRINT / CERTIFICATE | ESP32 only accepts certificates | SET to FNGPRINT for backwards compatibility with 2.0.X (ESP8266)
#define HTTPS_CERT_STORAGE          ST_SPIFFS           // ST_SPIFFS / ST_PROGMEM | If you want to be able to update your certificates from config mode choose for ST_SPIFFS
#define HTTPS_FNGPRINT              "2b 14 1a f1 5e 54 87 fc 0d f4 6f 0e 01 1c 0d 77 25 28 5b 9e" // Initial fingerprint(ESP8266). You can edit & change this later in config mode.

// OTA defines
#define OTA_HOST                    "iotappstory.com"   // OTA update host
#define OTA_UPD_FILE                "/ota/updates"      // file at host that handles 8266 updates
#define OTA_LOG_FILE                "/ota/logs"         // file at host that handles 8266 updates
#define OTA_HOST_HTTP_PORT          80
#define OTA_HOST_HTTPS_PORT         443
#define OTA_LOCAL_UPDATE            false               // Update firmware by uploading a .bin file in config mode | Only when config is stored in SPIFFS. CFG_STORAGE (line 15)
#define OTA_UPD_CHECK_SPIFFS        true                // Do you want to OTA update SPIFFS? | true / false
#define OTA_UPD_CHECK_NEXTION       false               // Do you want to OTA update your Nextion display? | true / false

#if defined  ESP8266
    #define OTA_BUFFER              1024
    #define NEXT_RES                5       // Nextion reset pin | Default 5 / D1 | Use this pin to control a transistor or relay to "hard" reset your display(power) after updates
    #define NEXT_RX                 14      // Nextion RX pin | Default 14 / D5
    #define NEXT_TX                 12      // Nextion TX pin | Default 12 / D6
    #define NEXT_BAUD               38400   // Nextion baudrate | 115200 / 57600 / 38400 / 19200
#elif defined ESP32
    #define OTA_BUFFER              2048
    #define NEXT_RES                5       // Nextion reset pin | Default 5 / D1 | Use this pin to control a transistor or relay to "hard" reset your display(power) after updates
    #define NEXT_RX                 16      // Nextion RX pin | Default 16
    #define NEXT_TX                 17      // Nextion TX pin | Default 17
    #define NEXT_BAUD               115200  // Nextion baudrate | 115200 / 57600
#endif

// EERPOM & max nr of addable fields
#define EEPROM_STORAGE_STYLE        EEP_NEW	// EEP_OLD / EEP_NEW

#if defined  ESP8266
    #define EEPROM_SIZE             4096    // EEPROM_SIZE depending on device
    #define MAXNUMEXTRAFIELDS       8       // wifimanger | max num of fields that can be added
#elif defined ESP32
    #define EEPROM_SIZE             1984
    #define MAXNUMEXTRAFIELDS       12
#else
    #define EEPROM_SIZE             1024
    #define MAXNUMEXTRAFIELDS       8
#endif

// EEPROM start addresses
// This is setup to let wifi & config strucs grow / shrink without effecting config & added fields by leaving space beteen structs.
// For a tighter & dynamic EEPROM layout use the commented out formulas behind the static values.
#define WIFI_EEP_START_ADDR         0
#define CFG_EEP_START_ADDR          500     // WIFI_EEP_START_ADDR + ((sizeof(WiFiCredStruct) * WIFI_MULTI_MAX) + 2)
#define FIELD_EEP_START_ADDR        720     // CFG_EEP_START_ADDR + sizeof(ConfigStruct) + 2

/**
    ------ ------ ------ ------ ------ ------ internal DEFINES for library ------ ------ ------ ------ ------ ------
*/

// length of config variables
#define STRUCT_CHAR_ARRAY_SIZE      50
#define STRUCT_PASSWORD_SIZE        64      // Thankyou reibuehl
#define STRUCT_COMPDATE_SIZE        20
#define STRUCT_BNAME_SIZE           30
#define STRUCT_HOST_SIZE            24
#define STRUCT_FILE_SIZE            31

// constants used to define the status of the mode button based on the time it was pressed. (miliseconds)
#define MODE_BUTTON_SHORT_PRESS       500
#define MODE_BUTTON_LONG_PRESS        4000
#define MODE_BUTTON_VERY_LONG_PRESS   10000

// define storage types (internal use)
#define ST_SPIFFS   0
#define ST_SD       1
#define ST_PROGMEM  2
#define ST_CLOUD    3
#define ST_HYBRID   4

#define FNGPRINT    0
#define CERTIFICATE 1
#define IASLIB      "2.1.0-RC5.1"

// define storage types (internal use)
#define EEP_OLD     0
#define EEP_NEW     1

// used for storing the config struct to eeprom
#define MAGICBYTES  "CFG"
#define MAGICEEP    "%"

#if WIFI_DHCP_ONLY == false && WIFI_MULTI == true
    #undef WIFI_MULTI
    #define WIFI_MULTI false
#endif

#if WIFI_MULTI == false || WIFI_DHCP_ONLY == false
    #define WIFIBEGIN(x,y) WiFi.begin(x,y)          // add single credential
    #define WIFIBEGINDEBUG "WiFi.begin"
    #define WIFIRUN WiFi.status()
    #define WIFIRUNDEBUG "WiFi.status()"
#else
    #define WIFIBEGIN(x,y) wifiMulti.addAP(x,y)     // add multiple credentials
    #define WIFIBEGINDEBUG "wifiMulti.addAP"
    #define WIFIRUN wifiMulti.run()
    #define WIFIRUNDEBUG "wifiMulti.run()"
#endif

#if defined ESP8266
    #define ESP_GETCHIPID ESP.getChipId()               // define for get chip id
    #define ESP_GETFLASHCHIPID ESP.getFlashChipId()     // define for get flash chip id
    #define ESP_SPIFFSBEGIN SPIFFS.begin()              // define for SPIFFS.begin()
#elif defined ESP32
    #define ESP_GETCHIPID (uint32_t)ESP.getEfuseMac()
    #define ESP_GETFLASHCHIPID (uint32_t)g_rom_flashchip.device_id
    #define ESP_SPIFFSBEGIN SPIFFS.begin(true)
#endif

/*---------------------------------------------------------------------------*/
/*                                GLOBAL VARIABLES                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
