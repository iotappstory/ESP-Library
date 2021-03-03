/*                          =======================
============================   C/C++ HEADER FILE   ============================
                            =======================                       *//**
  serialFeedback_NL.h

  Created by Onno Dirkzwager on 10.08.2018.
  Copyright (c) 2018 IOTAppStory. All rights reserved.

*///===========================================================================

/*---------------------------------------------------------------------------*/
/*                                    INCLUDES                               */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                            DEFINITIONS AND MACROS                         */
/*---------------------------------------------------------------------------*/

#define SER_ERASE_FULL                  " EEPROM volledig wissen"
#define SER_ERASE_PART                  " EEPROM deels wissen"
#define SER_ERASE_PART_EXT              " EEPROM deels wissen, behoud config settings"
#define SER_ERASE_NONE                  " Laat EEPROM intact"
#define SER_ERASE_FLASH                 " Erasing Flash...\n From %4d to %4d\n"

#define SER_CALLBACK_FIRST_BOOT         " Voer first boot callback uit"
#define SER_START                       " Start "
#define SER_SAVE_CONFIG                 "Config instellingen opslaan...\n"
#define SER_MODE_SEL_BTN                " Mode select knop: GPIO%d\n Boardname: %s\n Update on boot: %d\n"

#define SER_BOOTTIMES_UPDATE            " bootTimes sinds laatste update: %d\n boardMode: %c\n"
#define SER_BOOTTIMES_POWERUP           " rtcMem\n bootTimes sinds powerup: %d\n boardMode: %c\n"

#define SER_UPDATE_IASLOG               " Verstuur log naar IAS"
#define SER_FAILED_COLON                " Mislukt: "
#define SER_FAILED_EXCL                 "\n Mislukt!"
#define SER_FAILED_TRYAGAIN             "\n Mislukt, Probeer nogmaals!!!"

#define SER_CONFIG_MODE                 "\n\n\n\n C O N F I G U R A T I E   M O D U S\n"
#define SER_CONFIG_AP_MODE              " AP mode. Verbinden met Wifi AP \"%s\"\n En open 192.168.4.1\n"
#define SER_CONFIG_STA_MODE             " STA mode. Open "
#define SER_CONFIG_STA_MODE_CHANGE      "\n Veranderd naar STA mode. Open "
#define SER_CONFIG_TIMEOUT              " Time-out vanwege inactiviteit"
#define SER_CONFIG_EXIT                 " Exit config"
#define SER_CONFIG_ENTER                " Start Configuratie Modus"

#define SER_REC_ACT_CODE                "\n Activatie code van browser ontvangen"
#define SER_REC_CREDENTIALS             "Credentials ontvangen: "

#define SER_CONN_SAVE_EEPROM            " Verbonden. Opslaan config naar eeprom"
#define SER_CONNECTING                  " Verbinden met WiFi AP"
#define SER_CONN_NONE_GO_CFG            "Geen verbinding. Ga in Configuratie Modus\n"
#define SER_CONN_NONE_CONTINU           "\n WiFi NIET verbonden. Toch verder\n"
#define SER_CONN_LOST_RECONN            " Verbinding verloren! Opnieuw verbinden"
#define SER_CONNECTED                   " WiFi verbonden\n"

#define SER_SNTP_SETUP                  " NTP server instellen\n"
#define SER_SNTP_SYNC_TIME              " Sync tijd met NTP server\n "
#define SER_SNTP_DISP_UTC               "\n UTC: "

#define SER_RETRY_LATER                 " Probeer later opnieuw"

#define SER_DEV_MAC                     " Apparaat MAC: "
#define SER_DEV_IP                      " Apparaat IP Address: "
#define SER_DEV_MDNS                    " MDNS responder gestart: http://"
#define SER_DEV_MDNS_INFO               "\n\n Om mDNS te gebruiken Instaleer host software:\n - Voor Linux, instaleer Avahi (http://avahi.org/)\n - Voor Windows, instaleer Bonjour (https://commaster.net/content/how-resolve-multicast-dns-windows)\n - Voor Mac OSX en iOS is al ondersteuning via Bonjour aanwezig"
#define SER_DEV_MDNS_FAIL               " MDNS responder start mislukt"

#define SER_CALLING_HOME                " Calling Home"
#define SER_CALLHOME_FAILED             " Mislukt. Server onbereikbaar OF HTTPS probleem"
#define SER_CALLHOME_TIMEOUT            " Mislukt. Verbinding verbroken"
#define SER_CALLHOME_CERT_FAILED        " ERROR: certificaat verificatie mislukt!"
#define SER_CALLHOME_HDR_FAILED         " ERROR: bestand header verificatie mislukt!"
#define SER_RET_FROM_IAS                "\n Terug van IOTAppStory.com"
#define SER_CHECK_FOR                   " Checken voor "
#define SER_APP_SKETCH                  "App(Sketch)"
#define SER_SPIFFS                      "SPIFFS"
#define SER_NEXTION                     "Nextion scherm"
#define SER_UPDATES_FROM                " updates van: "
#define SER_UPDATES                     " updates"
#define SER_UPDATEDTO                   "\n Geupdate naar: "
#define SER_UPDATEDERROR                "\n Updaten mislukt: "

#define SER_DOWNLOADING                 " Downloaden..."
#define SER_INSTALLING                  " Installeren..."
#define SER_REBOOT_NEC                  " Reboot noodzakelijk!"

#define SER_PROC_ERROR                  "Teveel velden! Verander MAXNUMEXTRAFIELDS in config.h;"
#define SER_PROC_FIELDS                 " Verwerken toegevoegde velden"
#define SER_PROC_TBL_HDR                "\n ID | LABEL                          | LEN |  EEPROM HDR  |  EEPROM VAL  | DEFAULT WAARDE                  | HUIDIGE WAARDE                  | STATUS\n"
#define SER_PROC_TBL_OVRW               "%-30s | OVERSCHREVEN"
#define SER_PROC_TBL_DEF                "%-30s | DEFAULT"
#define SER_PROC_TBL_WRITE              "%-30s | SCHRIJVEN NAAR EEPROM"

#define SER_EEPROM_FOUND                " EEPROM Configuratie gevonden"
#define SER_EEPROM_NOT_FOUND            " EEPROM Configuratie NIET GEVONDEN!!!!"

#define SER_SPIFFS_MOUNTING             "Koppel SPIFFS..."
#define SER_SPIFFS_PART_NOT_FOUND       "SPIFFS Partitie Niet gevonden"
#define SER_CERTIFICATE_NOT_FOUND       "Certificaat Niet gevonden!"
#define SER_CERTIFICATE_NOT_LOADED      "Kon certificaat niet laden!"

#define SER_UPD_SPIFFS_TO_LARGE         "SPIFFS update te groot!"
#define SER_UPD_SKETCH_TO_LARGE         "Sketch update te groot!"
#define SER_UPD_BEGIN_FAILED            "Update.begin mislukt!"
#define SER_UPD_MD5_FAILED              "Update.setMD5 mislukt!"
#define SER_UPD_WRITE_FAILED            "Update.write mislukt!"
#define SER_UPD_END_FAILED              "Update.END mislukt!"

#define SER_SERV_DEV_INFO               " Serveer apparaat info"
#define SER_SERV_WIFI_SCAN_RES          " Serveer resultaat van Wifiscan"
#define SER_SERV_WIFI_CRED              " Serveer Wifi credentials"
#define SER_SERV_CERT_SCAN_RES          " Serveer resultaten certificate scan"
#define SER_SERV_APP_SETTINGS           " Serveer App Settings"

#define SER_SAVE_APP_SETTINGS           " Opslaan App Settings"
#define SER_SAVE_FINGERPRINT            " Opslaan Fingerprint"

#define SER_CONN_REC_CRED               "\n verbinden met ontvangen credentials"
#define SER_CONN_REC_CRED_DB3           " \n verbinden met ontvangen credentials: %s - %s\n"
#define SER_CONN_REC_CRED_PROC          "\n Verwerken van ontvangen credentials"
#define SER_CONN_ADDED_AP_CRED          "\n Wifi credentials toegevoegd voor AP%d\n"
#define SER_CONN_CRED_MISSING           "SSID of Wachtwoord is leeg"

/*---------------------------------------------------------------------------*/
/*                        TYPEDEFS, CLASSES AND STRUCTURES                   */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                                GLOBAL VARIABLES                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
