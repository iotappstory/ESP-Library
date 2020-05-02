/*                          =======================
============================   C/C++ HEADER FILE   ============================
                            =======================                       *//**
  serialFeedback_EN.h

  Created by Onno Dirkzwager on 05.08.2018.
  Copyright (c) 2018 IOTAppStory. All rights reserved.

*///===========================================================================

/*---------------------------------------------------------------------------*/
/*                                    INCLUDES                               */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                            DEFINITIONS AND MACROS                         */
/*---------------------------------------------------------------------------*/

#define SER_ERASE_FULL                  " (F)ull erase of EEPROM"
#define SER_ERASE_PART                  " (P)artial erase of EEPROM"
#define SER_ERASE_PART_EXT              " Partial erase of EEPROM but leaving config settings"
#define SER_ERASE_NONE                  " (L)eave EEPROM intact"
#define SER_ERASE_FLASH                 " Erasing EEPROM...\n From %4d to %4d\n"

#define SER_CALLBACK_FIRST_BOOT         " Run first boot callback"
#define SER_START                       " Start "
#define SER_SAVE_CONFIG                 "Saving config presets...\n"
#define SER_MODE_SEL_BTN                " Mode select button: GPIO%d\n Boardname: %s\n Update on boot: %d\n"

#define SER_BOOTTIMES_UPDATE            " bootTimes since last update: %d\n boardMode: %c\n"
#define SER_BOOTTIMES_POWERUP           " rtcMem\n bootTimes since powerup: %d\n boardMode: %c\n"

#define SER_UPDATE_IASLOG               " Send log to IAS"
#define SER_FAILED_COLON                " Failed: "
#define SER_FAILED_EXCL                 "\n Failed!"
#define SER_FAILED_TRYAGAIN             " try again!!!"

#define SER_CONFIG_MODE                 "\n\n\n\n C O N F I G U R A T I O N   M O D E\n"
#define SER_CONFIG_AP_MODE              " AP mode. Connect to Wifi AP \"%s\"\n And open 192.168.4.1\n"
#define SER_CONFIG_STA_MODE             " STA mode. Open "
#define SER_CONFIG_STA_MODE_CHANGE      " Changed to STA mode. Reconnect to: "
#define SER_CONFIG_EXIT                 " Exit config"
#define SER_CONFIG_ENTER                " Entering in Configuration Mode"

#define SER_REC_ACT_CODE                "\n Received activation code from browser"

#define SER_CONN_SAVE_EEPROM            " Connected. Saving config to eeprom"
#define SER_CONNECTING                  " Connecting to WiFi AP"
#define SER_CONN_NONE_GO_CFG            " Going into Configuration Mode\n"
#define SER_CONN_NONE_CONTINU           "\n Continu without WiFi\n"
#define SER_CONN_LOST_RECONN            " Connection lost! Reconnecting"
#define SER_CONNECTED                   " WiFi connected\n"
#define SER_SYNC_TIME_NTP               " Synchronising time with NTP server\n "

#define SER_DEV_MAC                     " Device MAC: "
#define SER_DEV_IP                      " IP Address: "
#define SER_DEV_MDNS                    " MDNS responder started: http://"
#define SER_DEV_MDNS_INFO               "\n\n To use mDNS Install host software:\n - For Linux, install Avahi (http://avahi.org/)\n - For Windows, install Bonjour (https://commaster.net/content/how-resolve-multicast-dns-windows)\n - For Mac OSX and iOS support is built in through Bonjour already"
#define SER_DEV_MDNS_FAIL               " MDNS responder failed to start"

#define SER_CALLING_HOME                " Calling Home"
#define SER_CALLHOME_FAILED             "Error: connection failed"
#define SER_CALLHOME_TIMEOUT            "Error: connection timeout"
#define SER_CALLHOME_CERT_FAILED        " ERROR: certificate verification failed!"
#define SER_CALLHOME_HDR_FAILED         " ERROR: file header verification failed!"
#define SER_RET_FROM_IAS                "\n Returning from IOTAppStory.com"
#define SER_CHECK_FOR                   " Checking for "
#define SER_APP_SKETCH                  "App(Sketch)"
#define SER_SPIFFS                      "SPIFFS"
#define SER_NEXTION                     "Nextion screen"
#define SER_UPDATES_FROM                " updates from: "
#define SER_UPDATES                     " updates"
#define SER_UPDATEDTO                   "\n Updated to: "
#define SER_UPDATEDERROR                "\n Error updating device: "

#define SER_DOWNLOADING                 " Downloading..."
#define SER_INSTALLING                  " Installing..."
#define SER_REBOOT_NEC                  " Reboot necessary!"

#define SER_PROC_ERROR                  "To many fields! Change MAXNUMEXTRAFIELDS in config.h;"
#define SER_PROC_FIELDS                 " Processing added fields"
#define SER_PROC_TBL_HDR                "\n ID | LABEL                          | LEN |  EEPROM LOC  | DEFAULT VALUE                  | CURRENT VALUE                  | STATUS\n"
#define SER_PROC_TBL_OVRW               "%-30s | OVERWRITTEN"
#define SER_PROC_TBL_DEF                "%-30s | DEFAULT"
#define SER_PROC_TBL_WRITE              "%-30s | WRITTING TO EEPROM"

#define SER_EEPROM_FOUND                " EEPROM Configuration found"
#define SER_EEPROM_NOT_FOUND            " EEPROM Configuration NOT FOUND!!!!"

#define SER_SPIFFS_MOUNTING             "Mounting SPIFFS..."
#define SER_SPIFFS_PART_NOT_FOUND       "SPIFFS Partition Not Found"
#define SER_CERTIFICATE_NOT_FOUND       "Certificate Not Found!"
#define SER_CERTIFICATE_NOT_LOADED      "Could not load Certificate!"

#define SER_UPD_SPIFFS_TO_LARGE         "SPIFFS update to large!"
#define SER_UPD_SKETCH_TO_LARGE         "Sketch update to large!"
#define SER_UPD_BEGIN_FAILED            "Update.begin failed!"
#define SER_UPD_MD5_FAILED              "Update.setMD5 failed!"
#define SER_UPD_WRITE_FAILED            "Update.write failed!"
#define SER_UPD_END_FAILED              "Update.END failed!"

#define SER_SERV_DEV_INFO               " Serving device info"
#define SER_SERV_WIFI_SCAN_RES          " Serving results of Wifiscan"
#define SER_SERV_WIFI_CRED              " Serving Wifi credentials"
#define SER_SERV_CERT_SCAN_RES          " Serving results of certificate scan"
#define SER_SERV_APP_SETTINGS           " Serving App Settings"

#define SER_SAVE_APP_SETTINGS           " Saving App Settings"
#define SER_SAVE_FINGERPRINT            " Saving Fingerprint"

#define SER_CONN_REC_CRED               "\n Connect with received credentials"
#define SER_CONN_REC_CRED_DB3           "\n Connect with received credentials: %s - %s\n"
#define SER_CONN_REC_CRED_PROC          "\n Processing received credentials"
#define SER_CONN_ADDED_AP_CRED          "\n Added wifi credentials for AP%d\n"
#define SER_CONN_CRED_MISSING           " SSID or Password is missing"


/*---------------------------------------------------------------------------*/
/*                        TYPEDEFS, CLASSES AND STRUCTURES                   */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                                GLOBAL VARIABLES                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
