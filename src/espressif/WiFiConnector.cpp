/*                          =======================
============================   C/C++ SOURCE FILE   ============================
                            =======================                       *//**
  WiFiConnector.cpp

  Created by Onno Dirkzwager on 05.06.2019.
  Copyright (c) 2019 IOTAppStory. All rights reserved.

*///===========================================================================

/*---------------------------------------------------------------------------*/
/*                                INCLUDES                                   */
/*---------------------------------------------------------------------------*/

#include "WiFiConnector.h"

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
                        WiFiConnector getAPfromEEPROM

*///---------------------------------------------------------------------------
void WiFiConnector::getAPfromEEPROM(WiFiCredStruct& config, const int apNr) {
    #if WIFICONNECTOR_DEBUG == true
        Serial.print(F("WIFICONNECTOR_DEBUG\t| running getAPfromEEPROM("));
        Serial.print(apNr);
        Serial.println(F(")"));
    #endif

    EEPROM.begin(EEPROM_SIZE);

    const int eepStartAddress = WIFI_EEP_START_ADDR + ((apNr-1) * sizeof(WiFiCredStruct));
    const int magicBytesBegin = eepStartAddress + sizeof(WiFiCredStruct) - 4;

    #if WIFICONNECTOR_DEBUG == true
        Serial.print(F("WIFICONNECTOR_DEBUG\t| Searching for magicBytes("));
        Serial.print(MAGICBYTES);
        Serial.print(F(") at EEPROM address "));
        Serial.println(magicBytesBegin);
    #endif

    // check for magicBytes to confirm the config struct is stored in EEPROM
    if(EEPROM.read(magicBytesBegin) == MAGICBYTES[0] && EEPROM.read(magicBytesBegin + 1) == MAGICBYTES[1] && EEPROM.read(magicBytesBegin + 2) == MAGICBYTES[2]) {
        EEPROM.get(eepStartAddress, config);
    }
    EEPROM.end();

    //Serial.print("WIFICONNECTOR_DEBUG\t| config.ip: ");
    //Serial.println(config.ip);

    //return config;
}

/*-----------------------------------------------------------------------------
                        WiFiConnector getSSIDfromEEPROM

*///---------------------------------------------------------------------------
const char* WiFiConnector::getSSIDfromEEPROM(const int apNr) {
    EEPROM.begin(EEPROM_SIZE);
    WiFiCredStruct config;
    this->getAPfromEEPROM(config, apNr);
    return config.ssid;
}

/*-----------------------------------------------------------------------------
                        WiFiConnector addAPtoEEPROM

*///---------------------------------------------------------------------------
void WiFiConnector::addAPtoEEPROM(const char *ssid, const char *password, const int apNr) {
    if(apNr > 0) {
        this->_configCount = apNr - 1;
    }

    #if WIFI_MULTI == false
        if(this->_configCount >= 1){
            #if WIFICONNECTOR_DEBUG == true
                Serial.println(F("WIFICONNECTOR_DEBUG\t| Currently WIFI_MULTI is set to false. Which limits you to 1 AP. Ignoring ALL other config."));
            #endif
        }else{
    #endif

    #if WIFICONNECTOR_DEBUG == true
        Serial.print(F("\nWIFICONNECTOR_DEBUG\t| Running addAPtoEEPROM(\""));
        Serial.print(ssid);
        Serial.print(F("\", \""));
        Serial.print(password);
        Serial.println(F("\");"));
    #endif

    WiFiCredStruct config;

    strncpy(config.ssid, ssid, STRUCT_CHAR_ARRAY_SIZE);
    strncpy(config.password, password, STRUCT_PASSWORD_SIZE);

    config.dhcp     = true;
    this->_configCount++;
    this->WiFiCredStructToEEPROM(config, this->_configCount);

    #if WIFI_MULTI == false
        }
    #endif
}

/*-----------------------------------------------------------------------------
                        WiFiConnector addAPtoEEPROM

*///---------------------------------------------------------------------------
#if WIFI_DHCP_ONLY == false
void WiFiConnector::addAPtoEEPROM(const char *ssid, const char *password, IPAddress ip, IPAddress subnet, IPAddress gateway, IPAddress dnsserv) {
    #if WIFICONNECTOR_DEBUG == true
        Serial.print(F("\nWIFICONNECTOR_DEBUG\t| Running addAPtoEEPROM(\""));
        Serial.print(ssid);
        Serial.print(F("\", \""));
        Serial.print(password);
        Serial.print(F("\", \""));
        Serial.print(ip);
        Serial.print(F("\", \""));
        Serial.print(subnet);
        Serial.print(F("\", \""));
        Serial.print(gateway);
        Serial.print(F("\", \""));
        Serial.print(dnsserv);
        Serial.println(F("\");"));
        Serial.println(F("WIFICONNECTOR_DEBUG\t| Currently the wifiMulti library only supports DHCP. Ignoring ALL other config."));
    #endif

    WiFiCredStruct config;

    strncpy(config.ssid, ssid, STRUCT_CHAR_ARRAY_SIZE);
    strncpy(config.password, password, STRUCT_PASSWORD_SIZE);

    config.dhcp     = false;
    config.ip       = ip;
    config.subnet   = subnet;
    config.gateway  = gateway;
    config.dnsserv  = dnsserv;

    this->WiFiCredStructToEEPROM(config, 1);
}
#endif

/*-----------------------------------------------------------------------------
                        WiFiConnector addAPtoEEPROM

*///---------------------------------------------------------------------------
void WiFiConnector::addAPtoEEPROM(const char *ssid, const char *password, String ip, String subnet, String gateway, String dnsserv) {
    this->addAPtoEEPROM(ssid, password, this->ipFromString(ip), this->ipFromString(subnet), this->ipFromString(gateway), this->ipFromString(dnsserv));
}

/*-----------------------------------------------------------------------------
                        WiFiConnector addAndShiftAPinEEPROM

*///---------------------------------------------------------------------------
void WiFiConnector::addAndShiftAPinEEPROM(const char *ssid, const char *password) {
    // get current AP1
    WiFiCredStruct ap1;
    this->getAPfromEEPROM(ap1, 1);

    // is AP1 valid?
    if(strcmp(ap1.magicBytes, MAGICBYTES) == 0){

        // get current AP2
        WiFiCredStruct ap2;
        this->getAPfromEEPROM(ap2, 2);

        // is AP2 valid?
        if(strcmp(ap2.magicBytes, MAGICBYTES) == 0){

            // move AP2 to position 3
            this->WiFiCredStructToEEPROM(ap2, 3);
            this->_configCount = 3;

        }else{
            this->_configCount = 2;
        }

        // move AP1 to position 2
        this->WiFiCredStructToEEPROM(ap1, 2);
    }

    // write AP1
    this->addAPtoEEPROM(ssid, password, 1);
}

/*-----------------------------------------------------------------------------
                        WiFiConnector WiFiCredStructToEEPROM

*///---------------------------------------------------------------------------
void WiFiConnector::WiFiCredStructToEEPROM(WiFiCredStruct config, const int apNr) {
    EEPROM.begin(EEPROM_SIZE);

    const int configSize = sizeof(config);
    const int eepStartAddress = WIFI_EEP_START_ADDR + ((apNr-1) * configSize);

    EEPROM.put(eepStartAddress, config);
    EEPROM.end();

    #if WIFICONNECTOR_DEBUG == true
        Serial.print(F("WIFICONNECTOR_DEBUG\t| "));
        Serial.print(config.magicBytes);
        Serial.print(F(" | Written config nr"));
        Serial.print(apNr);
        Serial.print(F(" to EEPROM from "));
        Serial.print(eepStartAddress);
        Serial.print(F(" to "));
        Serial.println(apNr * configSize);
    #endif
}

/*-----------------------------------------------------------------------------
                        WiFiConnector setup

    Setup WiFi

*///---------------------------------------------------------------------------
bool WiFiConnector::setup() {
    #if WIFICONNECTOR_DEBUG == true
        Serial.println(F("\nWIFICONNECTOR_DEBUG\t| Running setup();"));
    #endif

    EEPROM.begin(EEPROM_SIZE);


    // config Variable to store custom object read from EEPROM.
    WiFiCredStruct config;

    // get credentials from eeprom
    const int configSize = sizeof(WiFiCredStruct);
    int nrOfCfgFound = 0;

    #if WIFICONNECTOR_DEBUG == true
        Serial.println(F("WIFICONNECTOR_DEBUG\t| Loading config settings from EEPROM"));

        Serial.print(F("WIFICONNECTOR_DEBUG\t| configSize: "));
        Serial.println(configSize);
        Serial.print(F("WIFICONNECTOR_DEBUG\t| Total of "));
        Serial.print(WIFI_MULTI_MAX);
        Serial.print(F(" x configSize: "));
        Serial.println(configSize * WIFI_MULTI_MAX);
    #endif

    for(int i=0; i < WIFI_MULTI_MAX; i++) {
        const int eepStartAddress = WIFI_EEP_START_ADDR + (i * configSize);

        // check for magicBytes to confirm the config struct is stored in EEPROM
        const int magicBytesBegin = eepStartAddress + configSize - 4;

        #if WIFICONNECTOR_DEBUG == true
            Serial.print(F("WIFICONNECTOR_DEBUG\t| Searching for magicBytes("));
            Serial.print(MAGICBYTES);
            Serial.print(F(") at EEPROM address "));
            Serial.println(magicBytesBegin);
        #endif

        if(EEPROM.read(magicBytesBegin) == MAGICBYTES[0] && EEPROM.read(magicBytesBegin + 1) == MAGICBYTES[1] && EEPROM.read(magicBytesBegin + 2) == MAGICBYTES[2]) {
            nrOfCfgFound++;

            #if WIFICONNECTOR_DEBUG == true
                Serial.print(F("WIFICONNECTOR_DEBUG\t| Loaded config nr"));
                Serial.print(i+1);

                Serial.print(F(" from EEPROM from "));
                Serial.print(eepStartAddress);
                Serial.print(F(" to "));
                Serial.println((i+1) * configSize);
            #endif

            EEPROM.get(eepStartAddress, config);

            #if WIFICONNECTOR_DEBUG == true
                Serial.print(F("WIFICONNECTOR_DEBUG\t| SSID\t\t: "));
                Serial.println(config.ssid);
                Serial.print(F("WIFICONNECTOR_DEBUG\t| Password\t: "));
                Serial.println(config.password);
            #endif

            // setup wifi credentials
            #if WIFI_DHCP_ONLY != true
                if(!config.dhcp) {

                    #if WIFICONNECTOR_DEBUG == true
                        Serial.println(F("WIFICONNECTOR_DEBUG\t| IP settings\t: Static"));
                        Serial.print(F("WIFICONNECTOR_DEBUG\t| IPAddress\t: "));
                        Serial.println(config.ip);

                        Serial.print(F("WIFICONNECTOR_DEBUG\t| Subnet\t: "));
                        Serial.println(config.subnet);

                        Serial.print(F("WIFICONNECTOR_DEBUG\t| Gateway\t: "));
                        Serial.println(config.gateway);

                        Serial.print(F("WIFICONNECTOR_DEBUG\t| DNS server\t: "));
                        Serial.println(config.dnsserv);

                        Serial.print(F("WIFICONNECTOR_DEBUG\t| Running "));
                        Serial.print(F("WiFi.config(\""));
                        Serial.print(config.ip);
                        Serial.print(F("\", \""));
                        Serial.print(config.gateway);
                        Serial.print(F("\", \""));
                        Serial.print(config.subnet);
                        Serial.print(F("\", \""));
                        Serial.print(config.dnsserv);
                        Serial.println(F("\")"));

                        if(!WiFi.config(config.ip, config.gateway, config.subnet, config.dnsserv)) {
                            Serial.println(F("WIFICONNECTOR_DEBUG\t| Failed to configure static ip. Continuing in DHCP mode."));
                        } else {
                            i = 3;
                        }

                    #else
                        if(WiFi.config(config.ip, config.gateway, config.subnet, config.dnsserv)) {
                            i = 3;
                        }
                    #endif
                    // TODO: USELESS
                    // this->_static = true;
                }
            #endif


            #if WIFICONNECTOR_DEBUG == true
                Serial.print(F("WIFICONNECTOR_DEBUG\t| Running "));
                Serial.print(WIFIBEGINDEBUG);
                Serial.print(F("(\""));
                Serial.print(config.ssid);
                Serial.print(F("\", \""));
                Serial.print(config.password);
                Serial.println(F("\")"));
            #endif

            // add single credential
            WIFIBEGIN(config.ssid, config.password);
        }
    }

    if(nrOfCfgFound == 0) {
        #if WIFICONNECTOR_DEBUG == true
            Serial.println(F("WIFICONNECTOR_DEBUG\t| Failed to find previous stored config in EEPROM. Run addAPtoEEPROM and try again."));
        #endif
        return false;
    }

    return true;
}

/*-----------------------------------------------------------------------------
                        WiFiConnector connectToAP

     Connect to Wifi AP

*///---------------------------------------------------------------------------
bool WiFiConnector::connectToAP(const char *waitChar) {

    int retries = WIFI_CONN_MAX_RETRIES;

    #if WIFICONNECTOR_DEBUG == true
        Serial.print(F("WIFICONNECTOR_DEBUG\t| Running "));
        Serial.println(WIFIRUNDEBUG);
    #endif
    while(WIFIRUN != WL_CONNECTED && retries-- > 0 ) {
        delay(500);
        if(waitChar) {
            Serial.print(waitChar);
        }
    }

    if(waitChar) {
        Serial.println("");
    }

    if(retries > 0) {
        this->_connected = true;
        return true;
    } else {
        this->_connected = false;
        return false;
    }
}


/*-----------------------------------------------------------------------------
                        WiFiConnector connectLoop

     Disconnect wifi

*///---------------------------------------------------------------------------
bool WiFiConnector::connectLoop(const char *waitChar) {
    if(WiFi.status() == WL_NO_SSID_AVAIL) {
        this->_connected = false;
        WiFi.disconnect(false);
        delay(10);

        if(this->connectToAP(waitChar)) {
            return true;
        }
    }
    return false;
}

/*-----------------------------------------------------------------------------
                        WiFiConnector disconnect

     Disconnect wifi

*///---------------------------------------------------------------------------
void WiFiConnector::disconnect() {
    WiFi.disconnect();
    this->_connected = false;
}

/*-----------------------------------------------------------------------------
                        WiFiConnector ipFromString

     Disconnect wifi

*///---------------------------------------------------------------------------
IPAddress WiFiConnector::ipFromString(String strIP) {
    IPAddress ip;
    ip.fromString(strIP);
    return ip;
}

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
