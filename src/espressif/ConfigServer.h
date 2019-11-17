/*                          =======================
============================   C/C++ HEADER FILE   ============================
                            =======================                       *//**
  ConfigServer.h

  Created by Onno Dirkzwager on 22.11.2018.
  Copyright (c) 2018 IOTAppStory. All rights reserved.

*///===========================================================================

#if defined ESP8266 || defined ESP32

#ifndef __ConfigServer_h__
#define __ConfigServer_h__

/*---------------------------------------------------------------------------*/
/*                                    INCLUDES                               */
/*---------------------------------------------------------------------------*/

#include "IOTAppStory.h"

#ifdef ESP32
    #include <AsyncTCP.h> // https://github.com/me-no-dev/AsyncTCP
    #include <FS.h> // esp32 core SPIFFS library
    #include <SPIFFS.h>
#elif defined  ESP8266 // ESP32
    #include <ESPAsyncTCP.h> // https://github.com/me-no-dev/ESPAsyncTCP
    #include <FS.h> // esp8266 core SPIFFS library
    #define FILE_WRITE  "w"
    #define FILE_APPEND "a"
#endif // ESP8266

#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer

/*---------------------------------------------------------------------------*/
/*                            DEFINITIONS AND MACROS                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        TYPEDEFS, CLASSES AND STRUCTURES                   */
/*---------------------------------------------------------------------------*/

struct ConfigStruct;
class IOTAppStory;

/*                          =======================
============================   CLASS DEFINITION    ============================
                            =======================                       *//**
  ConfigServer.

*//*=========================================================================*/
class ConfigServer {
public:
    ConfigServer(IOTAppStory& ias, ConfigStruct& config);
    void run();

private:
    IOTAppStory* _ias;
    ConfigStruct* _config;
    std::unique_ptr<AsyncWebServer> server;

    bool _tryToConn         = false;        // is the wifi connector busy? (trying to connect)
    bool _connFail          = false;        // did the last connection attempt faile
    bool _connChangeMode    = false;        // flag to notify the loop to change from AP to STA mode

    void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void hdlReturn(AsyncWebServerRequest *request, int ret);
    void hdlReturn(AsyncWebServerRequest *request, String retHtml, String type = "text/html");

};

/*---------------------------------------------------------------------------*/
/*                                GLOBAL VARIABLES                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
#endif // __ConfigServer_h__
#endif // ESP8266 || defined ESP32
