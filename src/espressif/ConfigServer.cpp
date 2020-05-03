/*                          =======================
============================   C/C++ SOURCE FILE   ============================
                            =======================                       *//**
  ConfigServer.cpp

  Created by Onno Dirkzwager on 22.11.2018.
  Copyright (c) 2018 IOTAppStory. All rights reserved.

*///===========================================================================

#if defined ESP8266 || defined ESP32

/*---------------------------------------------------------------------------*/
/*                                INCLUDES                                   */
/*---------------------------------------------------------------------------*/

#include "ConfigServer.h"
#include "IOTAppStory.h"

/*---------------------------------------------------------------------------*/
/*                        DEFINITIONS AND MACROS                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        TYPEDEFS AND STRUCTURES                            */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                                PROTOTYPES                                 */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                            LOCAL VARIABLES                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        FUNCTION IMPLEMENTATION                            */
/*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                        ConfigServer constructor

    @param ias IOTAppStory &
    @param config ConfigStruct &

*///---------------------------------------------------------------------------
ConfigServer::ConfigServer(IOTAppStory& ias, ConfigStruct& config) {
    this->_ias = &ias;
    this->_config = &config;
}

/*-----------------------------------------------------------------------------
                        ConfigServer run

*///---------------------------------------------------------------------------
void ConfigServer::run() {
        bool exitConfig = false;

        #if CFG_STORAGE == ST_SPIFFS || CFG_STORAGE == ST_HYBRID || defined ESP32
            if(!ESP_SPIFFSBEGIN) {
                #if DEBUG_LVL >= 1
                    DEBUG_PRINT(F(" SPIFFS Mount Failed"));
                #endif
            }
        #endif

        #if DEBUG_LVL >= 1
            DEBUG_PRINT(SER_CONFIG_MODE);
        #endif

        // Start the AsyncWebServer | We use the std::unique_ptr below because the usual AsyncWebServer server(80); causes crashed on the esp32 when the run() method is finished
        this->server.reset(new AsyncWebServer(80));

        if(this->_ias->_connected) {
            // when there is wifi setup server in STA mode
            WiFi.mode(WIFI_STA);
            #if DEBUG_LVL >= 2
                DEBUG_PRINT(SER_CONFIG_STA_MODE);
                DEBUG_PRINTLN(WiFi.localIP());
            #endif

        } else {
            #if WIFI_DHCP_ONLY == false
                WiFi.disconnect();
            #endif

            // when there is no wifi setup server in AP mode
            IPAddress apIP(192, 168, 4, 1);

            WiFi.mode(WIFI_AP_STA);
            #if WIFI_SMARTCONFIG == true
                WiFi.beginSmartConfig();
            #endif
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
            WiFi.softAP(this->_config->deviceName);

            #if DEBUG_LVL >= 2
                DEBUG_PRINTF_P(SER_CONFIG_AP_MODE, this->_config->deviceName);
            #endif

            WiFi.scanNetworks(true);
        }

        #if CFG_STORAGE == ST_SPIFFS
            // serv SPIFFS files from the /www/ directory
            this->server->serveStatic("/", SPIFFS, "/www/");
        #endif
        #if CFG_STORAGE == ST_CLOUD || CFG_STORAGE == ST_HYBRID
            // serv the index page or force wifi setup
            this->server->on("/", HTTP_GET, [&](AsyncWebServerRequest *request) { hdlReturn(request, this->_ias->strRetHtmlRoot()); });
        #endif
        #if CFG_PAGE_INFO == true
            // serv this device information in json format (first page in config)
            this->server->on("/i", HTTP_GET, [&](AsyncWebServerRequest *request) { hdlReturn(request, this->_ias->strRetDevInfo(), F("text/json")); });
        #endif

        // serv the wifi scan results
        this->server->on("/wsc", HTTP_GET, [&](AsyncWebServerRequest *request) { hdlReturn(request, this->_ias->strRetWifiScan(), F("text/json")); });

        // serv the wifi credentials
        this->server->on("/wc", HTTP_GET, [&](AsyncWebServerRequest *request) { hdlReturn(request, this->_ias->strRetWifiCred(), F("text/json")); });

        // save the received ssid & pass for the received APnr(i) ans serv results
        this->server->on("/wsa", HTTP_POST, [&](AsyncWebServerRequest *request) {
            #if DEBUG_LVL >= 3
                DEBUG_PRINTLN(request->getParam("s", true)->value());
                DEBUG_PRINTLN(request->getParam("p", true)->value());
            #endif

            int postAPnr = 0;
            if(request->hasParam("i", true)) {
                postAPnr = atoi(request->getParam("i", true)->value().c_str());
            }

            int len_char = request->getParam("s", true)->value().length() + 1;
            char charSSID[len_char];
            request->getParam("s", true)->value().toCharArray(charSSID, len_char);
            len_char = request->getParam("p", true)->value().length() + 1;
            char charPass[len_char];
            request->getParam("p", true)->value().toCharArray(charPass, len_char);

            if(postAPnr > 0) {
                hdlReturn(
                    request,
                    this->_ias->servSaveWifiCred(
                        charSSID,
                        charPass,
                        #if WIFI_DHCP_ONLY == true
                            postAPnr
                        #else
                            request->getParam("sip", true)->value(),
                            request->getParam("ssn", true)->value(),
                            request->getParam("sgw", true)->value(),
                            request->getParam("sds", true)->value()
                        #endif
                    )
                );
            } else {
                if(_tryToConn == false) {
                    WiFi.begin(charSSID, charPass);
                    _tryToConn = true;
                    hdlReturn(request, "2");    // busy
                    #if DEBUG_LVL >= 3
                        DEBUG_PRINTLN("configServer debug:\tbusy");
                    #endif
                } else {
                    String retHtml;
                    if(_connFail) {
                        _connFail = false;
                        retHtml = F("3");       // return html Failed

                        #if DEBUG_LVL >= 3
                            DEBUG_PRINTLN("configServer debug:\tTrying to connect: failed");
                        #endif

                    } else if(this->_ias->_connected) {
                        _connChangeMode = true;
                        _tryToConn = false;

                        this->_ias->servSaveWifiCred(
                            charSSID,
                            charPass
                            #if WIFI_DHCP_ONLY == false
                                ,request->getParam("sip", true)->value(),
                                request->getParam("ssn", true)->value(),
                                request->getParam("sgw", true)->value(),
                                request->getParam("sds", true)->value()
                            #endif
                        );

                        retHtml = F("1:");      // ok:ip
                        retHtml += WiFi.localIP().toString();

                        #if DEBUG_LVL >= 3
                            DEBUG_PRINTLN("configServer debug:\tTrying to connect: connected! send ip");
                        #endif

                    } else {
                        retHtml = F("2");       // still busy
                        #if DEBUG_LVL >= 3
                            DEBUG_PRINTLN("configServer debug:\tstill busy!");
                        #endif
                    }
                    hdlReturn(request, retHtml);
                }
            }
        });

        // save the received fingerprint and serv results
        #if defined  ESP8266 && HTTPS_8266_TYPE == FNGPRINT
            this->server->on("/fp", HTTP_POST, [&](AsyncWebServerRequest *request) { hdlReturn(request, this->_ias->servSaveFngPrint(request->getParam("f", true)->value())); });
        #endif

        // serv cert scan in json format
        this->server->on("/csr", HTTP_GET, [&](AsyncWebServerRequest *request) {
            if(request->hasParam("d")) {
                hdlReturn(request, this->_ias->strRetCertScan(request->getParam("d")->value()), F("text/json"));
            } else {
                hdlReturn(request, this->_ias->strRetCertScan(), F("text/json"));
            }
        });

        // upload a file to /certupl
        this->server->on("/certupl", HTTP_POST, [](AsyncWebServerRequest *request) {
            request->send(200);
        },
            [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

                File fsUploadFile;

                // First step: check file extension & open new SPIFFS file
                if(!index) {
                    /// Check if file has a valid extension .cer
                    if(!filename.endsWith(".cer")) {
                        request->send(500, F("text/plain"), F("ONLY .cer files allowed\n"));
                    }

                    #if DEBUG_LVL >= 3
                        DEBUG_PRINTF(" UploadStart: %s\n", filename.c_str());
                    #endif

                    /// open new SPIFFS file for writing data
                    fsUploadFile = SPIFFS.open("/cert/" + filename, FILE_WRITE);            // Open the file for writing in SPIFFS (create if it doesn't exist)

                } else {
                    /// open existing SPIFFS file for appending data
                    fsUploadFile = SPIFFS.open("/cert/" + filename, FILE_APPEND);
                }

                // Second step: write received buffer to SPIFFS file
                if(len) {
                    #if DEBUG_LVL >= 3
                        DEBUG_PRINT(F(" Writing:\t"));
                        DEBUG_PRINTLN(len);
                    #endif

                    /// write data
                    if(fsUploadFile.write(data, len) != len) {
                        #if DEBUG_LVL >= 3
                            DEBUG_PRINTLN(F(" Write error!"));
                        #endif

                        /// if write failed return error
                        request->send(500, F("text/plain"), F("Write error!\n"));
                    }
                }

                // Last step: close file
                if(final) {
                    #if DEBUG_LVL >= 3
                        DEBUG_PRINTF(" UploadEnd: %s, %u B\n", filename.c_str(), index+len);
                    #endif

                    /// close file
                    fsUploadFile.close();
                }
            }
        );

        // locally upload new firmware
        #if OTA_LOCAL_UPDATE == true
            this->server->on("/update", HTTP_POST, [&](AsyncWebServerRequest *request) {
                bool result = false;
                if(!Update.hasError()) {
                    // update is success
                    result = true;

                    // save new app name & version
                    {
                    request->getParam("n", true)->value().toCharArray(this->_config->appName, 33);
                    String("(local)").toCharArray(this->_config->appVersion, 12);
                    }

                }
                AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", result?"OK":"FAIL");
                response->addHeader("Connection", "close");
                request->send(response);
                delay(200);
                ESP.restart();
            }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
                if(!index) {
                    #if DEBUG_LVL >= 2
                        DEBUG_PRINTF(" Update Start: %s\n", filename.c_str());
                    #endif
                    #ifdef ESP8266
                        Update.runAsync(true);
                    #endif
                    if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
                        #if DEBUG_LVL >= 2
                            Update.printError(Serial);
                        #endif
                    }
                }
                if(!Update.hasError()) {
                    if(Update.write(data, len) != len){
                        #if DEBUG_LVL >= 2
                            Update.printError(Serial);
                        #endif
                    }
                }
                if(final) {
                    bool status = Update.end(true);
                    #if DEBUG_LVL >= 2
                        if(status) {
                            DEBUG_PRINTF(" Update Success: %uB\n", index+len);
                        } else {
                            DEBUG_PRINTLN(" Update Failed!");
                        }
                    #endif
                }
            });
        #endif

        // serv the app fields in json format
        this->server->on("/app", HTTP_GET, [&](AsyncWebServerRequest *request) { hdlReturn(request, this->_ias->strRetAppInfo(), F("text/json")); });

        // save the received app fields and serv results
        this->server->on("/as", HTTP_POST, [&](AsyncWebServerRequest *request) { hdlReturn(request, this->_ias->servSaveAppInfo(request)); });

        // save the received device activation code
        this->server->on("/ds", HTTP_POST, [&](AsyncWebServerRequest *request) { hdlReturn(request, this->_ias->servSaveActcode(request->getParam("ac", true)->value())); });

        // close and exit the web server
        this->server->on("/close", HTTP_GET, [&](AsyncWebServerRequest *request) { exitConfig = true; });


        // serv 404 page
        this->server->onNotFound([](AsyncWebServerRequest *request) {
            request->send(404);
        });

        // start the server
        this->server->begin();

        // server loop
        while(exitConfig == false) {

            yield();
            #if defined  ESP8266
                MDNS.update();
            #endif

            if(this->_ias->_connected) {
                //DEBUG_PRINTLN("Configserver connected loop part");

                // when succesfully added wifi cred in AP mode change to STA mode
                if(_connChangeMode) {
                    delay(1000);
                    WiFi.mode(WIFI_STA);
                    delay(100);
                    _connChangeMode = false;

                    #if DEBUG_LVL >= 2
                        DEBUG_PRINTLN(SER_CONNECTED);
                        DEBUG_PRINT(SER_CONFIG_STA_MODE_CHANGE);
                        DEBUG_PRINTLN(WiFi.localIP());
                    #endif
                }
            } else {

                // smartconfig default false / off
                #if WIFI_SMARTCONFIG == true
                    if(WiFi.smartConfigDone()) {
                        WiFi.mode(WIFI_AP_STA);
                        this->_ias->WiFiConnectToAP();
                    }
                #endif

                // wifi connect when asked
                if(_tryToConn == true) {

                    int retries = WIFI_CONN_MAX_RETRIES;

                    #if DEBUG_LVL >= 2
                        DEBUG_PRINTLN(SER_CONN_REC_CRED);
                        DEBUG_PRINT(F(" "));
                    #endif

                    while (WiFi.status() != WL_CONNECTED && retries-- > 0 ) {
                        delay(500);
                        #if DEBUG_LVL >= 1
                            DEBUG_PRINT(F("."));
                        #endif
                    }

                    #if DEBUG_LVL >= 1
                        DEBUG_PRINT(F("\n"));
                    #endif

                    if(retries > 0) {
                        this->_ias->_connected = true;
                        _connFail = false;
                    } else {
                        this->_ias->_connected = false;
                        _connFail = true;
                    }

                    _tryToConn = false;
                    yield();
                }
            }
        }

        #if DEBUG_LVL >= 2
            DEBUG_PRINTLN(SER_CONFIG_EXIT);
        #endif
}

/*-----------------------------------------------------------------------------
                        ConfigServer onUpload

Handle uploads

*///---------------------------------------------------------------------------
void ConfigServer::onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if(!index) {
        Serial.printf("UploadStart: %s\n", filename.c_str());
    }

    for(size_t i=0; i<len; i++) {
        Serial.write(data[i]);
    }

    if(final) {
        Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index+len);
    }
}

/*-----------------------------------------------------------------------------
                        ConfigServer hdlReturn

Return page handler

*///---------------------------------------------------------------------------
void ConfigServer::hdlReturn(AsyncWebServerRequest* request, String retHtml, String type) {
    #if CFG_AUTHENTICATE == true
    if(!request->authenticate("admin", this->_config->cfg_pass)) {
        return request->requestAuthentication();
    } else {
    #endif // CFG_AUTHENTICATE

        AsyncWebServerResponse *response = request->beginResponse(200, type, retHtml);
        response->addHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
        response->addHeader(F("Pragma"), F("no-cache"));
        response->addHeader(F("Expires"), F("-1"));
        request->send(response);

    #if CFG_AUTHENTICATE == true
    }
    #endif // CFG_AUTHENTICATE
}

/*-----------------------------------------------------------------------------
                        ConfigServer hdlReturn

Return page handler

*///---------------------------------------------------------------------------
void ConfigServer::hdlReturn(AsyncWebServerRequest *request, int ret) {
    this->hdlReturn(request, String(ret));
}

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
#endif // ESP8266 || defined ESP32
