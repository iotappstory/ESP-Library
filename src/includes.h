/*                          =======================
============================   C/C++ HEADER FILE   ============================
                            =======================                       *//**
  includes.h

  Created by Onno Dirkzwager on 10.02.2019.
  Copyright (c) 2019 IOTAppStory. All rights reserved.

*///===========================================================================

/*---------------------------------------------------------------------------*/
/*                                    INCLUDES                               */
/*---------------------------------------------------------------------------*/

#ifdef ESP32
    #include <rom/spi_flash.h>                  // enable flash chip id from the SDK
    #include <WiFi.h>
    #include <WiFiMulti.h>
    #include "espressif/WiFiConnector.h"
    #include <ESPmDNS.h>

    #include "espressif/esp32/BoardInfo.h"
    #include "espressif/esp32/CallServer.h"
    #include "espressif/ConfigServer.h"

    #include "espressif/UpdateClassVirt.h"
    #include <Update.h>
    #include "espressif/UpdateESPClass.h"
    #if OTA_UPD_CHECK_NEXTION == true
        #include "espressif/UpdateNextionClass.h"
    #endif // OTA_UPD_CHECK_NEXTION

    #include <HTTPClient.h>
    #include <DNSServer.h>
    #include <EEPROM.h>
    #include <ESPAsyncWebServer.h>              // https://github.com/me-no-dev/ESPAsyncWebServer
#endif // ESP32

#ifdef  ESP8266
    #include <ESP8266WiFi.h>
    #include <ESP8266WiFiMulti.h>
    #include "espressif/WiFiConnector.h"
    #include <ESP8266mDNS.h>

    #include "espressif/esp8266/BoardInfo.h"
    #include "espressif/esp8266/CallServer.h"
    #include "espressif/ConfigServer.h"

    #include "espressif/UpdateClassVirt.h"
    #include "espressif/UpdateESPClass.h"
    #if OTA_UPD_CHECK_NEXTION == true
        #include "espressif/UpdateNextionClass.h"
    #endif // OTA_UPD_CHECK_NEXTION

    #include <ESP8266HTTPClient.h>
    #include <DNSServer.h>
    #include <EEPROM.h>
    #include <ESPAsyncWebServer.h>              // https://github.com/me-no-dev/ESPAsyncWebServer
#endif // ESP8266

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
#endif // ARDUINO_SAMD_VARIANT_COMPLIANCE

/*---------------------------------------------------------------------------*/
/*                            DEFINITIONS AND MACROS                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        TYPEDEFS, CLASSES AND STRUCTURES                   */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                                GLOBAL VARIABLES                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
