/*                          =======================
============================   C/C++ HEADER FILE   ============================
                            =======================                       *//**
  WiFiConnector.h

  Created by Onno Dirkzwager on 05.06.2019.
  Copyright (c) 2019 IOTAppStory. All rights reserved.

*///===========================================================================

#ifndef __WiFiConnector_h__
#define __WiFiConnector_h__

/*---------------------------------------------------------------------------*/
/*                                    INCLUDES                               */
/*---------------------------------------------------------------------------*/

// INCLUDES
#include <IOTAppStory.h>
#include <EEPROM.h>

#ifdef ESP8266
    #include <ESP8266WiFi.h>
    #include <ESP8266WiFiMulti.h>
#elif defined ESP32
    #include <WiFi.h>
    #include <WiFiMulti.h>
#endif

/*---------------------------------------------------------------------------*/
/*                            DEFINITIONS AND MACROS                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        TYPEDEFS, CLASSES AND STRUCTURES                   */
/*---------------------------------------------------------------------------*/

/*                          =======================
============================   STRUCT DEFINITION   =============================
                            =======================                        *//**
  WiFiCredStruct.

*//*=========================================================================*/
struct WiFiCredStruct {
    char ssid[STRUCT_CHAR_ARRAY_SIZE] = "";
    char password[STRUCT_PASSWORD_SIZE] = "";
    #if WIFI_DHCP_ONLY == true
        bool dhcp = true;
    #else
        bool dhcp = false;
        IPAddress ip;
        IPAddress subnet;
        IPAddress gateway;
        IPAddress dnsserv;
    #endif

    const char magicBytes[4] = MAGICBYTES;
};

/*                          =======================
============================   CLASS DEFINITION    ============================
                            =======================                       *//**
  WiFiConnector.

*//*=========================================================================*/
class WiFiConnector {
public:
    //WiFiConnector();
    void getAPfromEEPROM(WiFiCredStruct &config, const int apNr);
    const char* getSSIDfromEEPROM(const int apNr);

    void addAPtoEEPROM(const char* ssid, const char* password, const int apNr = 0);
    void addAPtoEEPROM(const char* ssid, const char* password, IPAddress ip, IPAddress subnet, IPAddress gateway, IPAddress dnsserv);
    void addAPtoEEPROM(const char* ssid, const char* password, String ip, String subnet, String gateway, String dnsserv);

    void addAndShiftAPinEEPROM(const char* ssid, const char* password);

    bool setup();
    bool connectToAP(const char* waitChar = nullptr);
    bool connectLoop(const char* waitChar = nullptr);
    void disconnect();

    IPAddress ipFromString(String strIP);

    #if WIFI_MULTI == true
        #ifdef ESP32
            WiFiMulti wifiMulti;
        #elif defined ESP8266
            ESP8266WiFiMulti wifiMulti;
        #endif
    #endif

private:
    void WiFiCredStructToEEPROM(WiFiCredStruct config, const int apNr = 0);

    bool _connected = false; // wifi connection status bool
    // TODO: USELESS
    // bool _static = false;
    int _configCount = 0;
};

/*---------------------------------------------------------------------------*/
/*                                GLOBAL VARIABLES                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
#endif // __WiFiConnector_h__
