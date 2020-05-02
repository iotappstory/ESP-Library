/*                          =======================
============================   C/C++ SOURCE FILE   ============================
                            =======================                       *//**
  UpdateESPClass.cpp

  Created by Onno Dirkzwager on 10.02.2019.
  Copyright (c) 2019 IOTAppStory. All rights reserved.

*///===========================================================================

/*---------------------------------------------------------------------------*/
/*                                INCLUDES                                   */
/*---------------------------------------------------------------------------*/

#include <Arduino.h>
#include "UpdateESPClass.h"
#include "../serialFeedback_EN.h"

/*---------------------------------------------------------------------------*/
/*                        DEFINITIONS AND MACROS                             */
/*---------------------------------------------------------------------------*/

#ifdef DEBUG_SERIAL_ENABLE
    #define dbSerialPrint(a)    Serial.print(a)
    #define dbSerialPrintln(a)  Serial.println(a)
    #define dbSerialPrintf(a)    Serial.printf( __VA_ARGS__ )
    #define dbSerialBegin(a)    Serial.begin(a)
#else
    #define dbSerialPrint(a)    do{}while(0)
    #define dbSerialPrintln(a)  do{}while(0)
    #define dbSerialPrintf(a)   do{}while(0)
    #define dbSerialBegin(a)    do{}while(0)
#endif

/*---------------------------------------------------------------------------*/
/*                        TYPEDEFS AND STRUCTURES                            */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                                PROTOTYPES                                 */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                            LOCAL VARIABLES                                */
/*---------------------------------------------------------------------------*/

UpdateESPClass UpdateESP;

/*---------------------------------------------------------------------------*/
/*                        FUNCTION IMPLEMENTATION                            */
/*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                        UpdateESPClass prepareUpdate

*///---------------------------------------------------------------------------
bool UpdateESPClass::prepareUpdate(uint32_t upd_size, String &upd_md5, uint16_t command) {
    this->_upd_size = upd_size;
    this->_upd_md5 = &upd_md5;
    this->_command = command;

    /**
        Check for enough space before starting the update
    */
    if(this->_command == U_SPIFFS) {
        #ifdef ESP32
            const esp_partition_t* _partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
            if(!_partition) {
                (*this->_statusMessage) = SER_SPIFFS_PART_NOT_FOUND;
                return false;
            }

            if(this->_upd_size > _partition->size) {
                #ifdef DEBUG_SERIAL_ENABLE
                    dbSerialPrintf("SPIFFS update to large! (%d) free: %d\n", this->_upd_size, _partition->size);
                #endif

                (*this->_statusMessage) = SER_UPD_SPIFFS_TO_LARGE;
                return false;
            }
        #elif defined ESP8266
            size_t spiffsSize = ((size_t) &_SPIFFS_end - (size_t) &_SPIFFS_start);
            if(this->_upd_size > (int) spiffsSize) {
                #ifdef DEBUG_SERIAL_ENABLE
                    dbSerialPrintf("SPIFFS update to large! (%d) free: %d\n", this->_upd_size, spiffsSize);
                #endif

                (*this->_statusMessage) = SER_UPD_SPIFFS_TO_LARGE;
                return false;
            }
        #endif
    } else {
        if(this->_upd_size > (int) ESP.getFreeSketchSpace()) {
            #ifdef DEBUG_SERIAL_ENABLE
                dbSerialPrintf("Sketch update to large! (%d) free: %d\n", this->_upd_size, ESP.getFreeSketchSpace());
            #endif

            (*this->_statusMessage) = SER_UPD_SKETCH_TO_LARGE;
            return false;
        }
    }

    /**
        Initiate the esp update class
    */
    if(!Update.begin(this->_upd_size, this->_command)) {
        #ifdef DEBUG_SERIAL_ENABLE
            StreamString stError;
            Update.printError(stError);
            stError.trim();

            dbSerialPrint(F("Update.begin failed! "));
            dbSerialPrintln(stError.c_str());
        #endif

        (*this->_statusMessage) = SER_UPD_BEGIN_FAILED;
        return false;
    }
    //Update.runAsync(true);


    /**
        add and check the received md5 string
    */
    if(this->_upd_md5->length()) {

        if(!Update.setMD5(this->_upd_md5->c_str())) {
            #ifdef DEBUG_SERIAL_ENABLE
                dbSerialPrintln(F("Update.setMD5 failed! "));
                dbSerialPrint(F(" Received md5 string: "));
                dbSerialPrintln(this->_upd_md5->c_str());
            #endif

            (*this->_statusMessage) = SER_UPD_MD5_FAILED;

            return false;
        }
    }

    return true;
}

/*-----------------------------------------------------------------------------
                        UpdateESPClass update

*///---------------------------------------------------------------------------
bool UpdateESPClass::update(uint8_t *file_buf, size_t buf_size) {
    // Write the buffered bytes to the esp. If this fails, return false.
    if(!Update.write(file_buf, buf_size)){
        (*this->_statusMessage) = SER_UPD_WRITE_FAILED;
        return false;
    }
    return true;
}

/*-----------------------------------------------------------------------------
                        UpdateESPClass end

*///---------------------------------------------------------------------------
bool UpdateESPClass::end() {
    this->_upd_size = 0;

    /**
        End the esp update class
        If the update process end in an error print it
    */
    if(!Update.end()) {
        // on failed firmware installation
        #ifdef DEBUG_SERIAL_ENABLE
          Update.printError(stError);
          stError.trim();

          dbSerialPrint(F("Update.end failed!\n"));
          dbSerialPrintln(stError.c_str());
        #endif

        (*this->_statusMessage) = SER_UPD_END_FAILED;

        return false;
    }

    return true;
}

/*-----------------------------------------------------------------------------
                        UpdateESPClass sm

*///---------------------------------------------------------------------------
void UpdateESPClass::sm(String *statusMessage) {
    this->_statusMessage = statusMessage;
}

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
