/*                          =======================
============================   C/C++ SOURCE FILE   ============================
                            =======================                       *//**
  CallServer.cpp

  Created by Onno Dirkzwager on 02.10.2019.
  Copyright (c) 2019 IOTAppStory. All rights reserved.

*///===========================================================================

#ifdef ESP8266

/*---------------------------------------------------------------------------*/
/*                                INCLUDES                                   */
/*---------------------------------------------------------------------------*/

#include "CallServer.h"

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
                        CallServer constructor

    @param config ConfigStruct &
    @param command int

*///---------------------------------------------------------------------------
CallServer::CallServer(ConfigStruct &config, int command) {
    this->_config = &config;
    this->_command = command;
}

/*-----------------------------------------------------------------------------
                        CallServer getStream

*///---------------------------------------------------------------------------
Stream& CallServer::getStream(FirmwareStruct *firmwareStruct) {
    if(!this->get(this->_callFile, "")) {
        (*firmwareStruct).success = false;
        return this->_client;
    }

    (*firmwareStruct).xlength = 0;
    int code;

    #if DEBUG_LVL >= 3
        DEBUG_PRINTLN(F("\n Received http header"));
        DEBUG_PRINTLN(F(" ---------------------"));
    #endif

    while(this->_client.available()) {

        String line = this->_client.readStringUntil('\n');

        #if DEBUG_LVL >= 3
            // Read all the lines of the reply from server and print them to Serial
            DEBUG_PRINTLN(" " + line);
        #endif

        if(line.startsWith(F("HTTP/1.1 "))) {
            line.remove(0, 9);
            code = line.substring(0, 3).toInt();

            if(code != 200) {
                line.remove(0, 4);
                (*this->_statusMessage) = line;

                //return false;
                (*firmwareStruct).success = false;
                return this->_client;
            } else {
                #if DEBUG_LVL >= 1
                    DEBUG_PRINT(SER_DOWNLOADING);
                #endif
            }
        } else if(line.startsWith(F("Content-Length: "))) {
            line.remove(0, 16);
            (*firmwareStruct).xlength = line.toInt();

        } else if(line.startsWith(F("x-name: "))) {
            line.remove(0, 8);
            line.trim();
            (*firmwareStruct).xname = line;

        } else if(line.startsWith(F("x-ver: "))) {
            line.remove(0, 7);
            line.trim();
            (*firmwareStruct).xver = line;

        } else if(line.startsWith(F("x-MD5: "))) {
            line.remove(0, 7);
            line.trim();
            (*firmwareStruct).xmd5 = line;

        } else if(line == "\r") {
            break;
        }
    }

    #if DEBUG_LVL >= 3
        DEBUG_PRINTLN(F(" Extracted from header"));
        DEBUG_PRINTLN(F(" ---------------------"));
        DEBUG_PRINT(F(" Content-Length: "));
        DEBUG_PRINTLN((*firmwareStruct).xlength);
        DEBUG_PRINT(F(" x-name: "));
        DEBUG_PRINTLN((*firmwareStruct).xname);
        DEBUG_PRINT(F(" x-ver: "));
        DEBUG_PRINTLN((*firmwareStruct).xver);
        DEBUG_PRINT(F(" x-md5: "));
        DEBUG_PRINTLN((*firmwareStruct).xmd5);
        DEBUG_PRINTLN(F(" ---------------------\n"));
    #endif

    if(code == 200 && (*firmwareStruct).xlength > 0 && (*firmwareStruct).xname != "" && (*firmwareStruct).xver != "" && (*firmwareStruct).xmd5 != "") {
        (*firmwareStruct).success = true;
        return this->_client;
    } else {
        (*this->_statusMessage) = SER_CALLHOME_HDR_FAILED;
        (*firmwareStruct).success = false;
        return this->_client;
    }
}

/*-----------------------------------------------------------------------------
                        CallServer get

*///---------------------------------------------------------------------------
bool CallServer::get(const char* url, String args) {

    #if DEBUG_FREE_HEAP == true
        DEBUG_PRINTLN(" begin callServer::get(const char* url, String args)");
        DEBUG_PRINTF(" Free heap: %u\n", ESP.getFreeHeap());
    #endif

    // connect to client
    #if HTTPS == true
        #if HTTPS_8266_TYPE == CERTIFICATE

            #if HTTPS_CERT_STORAGE == ST_SPIFFS
                // Set root certificate (from SPIFFS file)
                #if DEBUG_LVL >= 3
                    DEBUG_PRINT(SER_SPIFFS_MOUNTING);
                #endif

                // Start SPIFFS and load partition
                if(!SPIFFS.begin()) {
                   (*this->_statusMessage) = SER_SPIFFS_PART_NOT_FOUND;
                   return false;
                }

                // Get root certificate file from SPIFFS
                File file = SPIFFS.open("/cert/iasRootCa.cer", "r");
                if(!file) {
                   (*this->_statusMessage) = SER_CERTIFICATE_NOT_FOUND;
                    return false;
                }

                #if DEBUG_LVL >= 3
                    DEBUG_PRINT("File size: ");
                    DEBUG_PRINTLN(file.size());
                #endif

                // Load root certificate (from SPIFFS)
                if(!this->_client.loadCACert(file, file.size()) || file.size() == 0) {
                   (*this->_statusMessage) = SER_CERTIFICATE_NOT_LOADED;
                    return false;
                }

                //setClock(); <--> moved to the IAS callhome method

            #else
                // Set root certificate (from const char ROOT_CA[] PROGMEM)
                this->_client.setCACert(ROOT_CA);
            #endif

            this->_client.connect(this->_callHost, 443);

            #if DEBUG_FREE_HEAP == true
                DEBUG_PRINTLN(" after _client.connect");
                DEBUG_PRINTF(" Free heap: %u\n", ESP.getFreeHeap());
            #endif

            // check connection return false if connection failed
            if (!this->_client.connected()) {
                // Error: connection failed
                (*this->_statusMessage) = SER_CALLHOME_FAILED;
                return false;
            }

        #elif HTTPS_8266_TYPE == FNGPRINT
            this->_client.setFingerprint(this->_config->sha1);

            if(!this->_client.connect(this->_callHost, 443)) {
                // Error: connection failed
                (*this->_statusMessage) = SER_CALLHOME_FAILED;
                return false;
            }

            if(!this->_client.verify(this->_config->sha1, this->_callHost)) {
                //ERROR: certificate verification failed!
                (*this->_statusMessage) = SER_CALLHOME_CERT_FAILED;
                return false;
            }
        #endif
    #else
        this->_client.connect(this->_callHost, 80);
    #endif

    String mode, md5;
    if(this->_command == U_LOGGER) {
        mode = F("log");
        md5 = ESP.getSketchMD5();
    } else if(this->_command == U_FLASH) {
        mode = F("sketch");
        md5 = ESP.getSketchMD5();
    } else if(this->_command == U_SPIFFS) {
        mode = F("spiffs");
        md5 = ESP.getSketchMD5();
    }
    #if OTA_UPD_CHECK_NEXTION == true
        else if(this->_command == U_NEXTION) {
            mode = F("nextion");
            md5 = this->_config->next_md5;
        }
    #endif

    // This will send the request to the server
    this->_client.print(String("GET ") + url + "?" + args + F(" HTTP/1.1\r\n") +
               F("Host: ") + this->_callHost +

               F("\r\nUser-Agent: ESP-http-Update") +

               F("\r\nx-ESP-STA-MAC: ") + WiFi.macAddress() +
               F("\r\nx-ESP-ACT-ID: ") + this->_config->actCode +
               F("\r\nx-ESP-LOCIP: ") + WiFi.localIP().toString() +
               F("\r\nx-ESP-FREE-SPACE: ") + ESP.getFreeSketchSpace() +
               F("\r\nx-ESP-SKETCH-SIZE: ") + ESP.getSketchSize() +


               F("\r\nx-ESP-SKETCH-MD5: ") + md5 +
               F("\r\nx-ESP-FLASHCHIP-ID: ") + ESP_GETFLASHCHIPID +
               F("\r\nx-ESP-CHIP-ID: ") + ESP_GETCHIPID +
               F("\r\nx-ESP-CORE-VERSION: ") + ESP.getCoreVersion() +
               F("\r\nx-ESP_IASLIB: ") + IASLIB +

               F("\r\nx-ESP-FLASHCHIP-SIZE: ") + ESP.getFlashChipSize() +
               F("\r\nx-ESP-VERSION: ") + this->_config->appName + " v" + this->_config->appVersion +

               F("\r\nx-ESP-MODE: ") + mode +

               F("\r\nConnection: close\r\n\r\n"));


    // return false and set statusMessage on timeout
    unsigned long timeout = millis();
    while(this->_client.available() == 0) {
        if(millis() - timeout > 5000) {
            (*this->_statusMessage) = SER_CALLHOME_TIMEOUT;
            this->_client.stop();
            return false;
        }
    }
    return true;
}

/*-----------------------------------------------------------------------------
                        CallServer sm

*///---------------------------------------------------------------------------
void CallServer::sm(String *statusMessage) {
    this->_statusMessage = statusMessage;
}

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
#endif // ESP8266
