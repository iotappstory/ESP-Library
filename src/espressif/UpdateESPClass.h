/*                          =======================
============================   C/C++ HEADER FILE   ============================
                            =======================                       *//**
  UpdateESPClass.h

  Created by Onno Dirkzwager on 10.02.2019.
  Copyright (c) 2019 IOTAppStory. All rights reserved.

*///===========================================================================

#ifndef __UpdateESPClass_h__
#define __UpdateESPClass_h__

/*---------------------------------------------------------------------------*/
/*                                    INCLUDES                               */
/*---------------------------------------------------------------------------*/

#include "UpdateClassVirt.h"
#include <StreamString.h>
#ifdef ESP32
    #include <Update.h>
    #include <esp_partition.h>
    #include <esp_ota_ops.h>                // get running partition
#else
    extern "C" uint32_t _SPIFFS_start;
    extern "C" uint32_t _SPIFFS_end;
#endif

/*---------------------------------------------------------------------------*/
/*                            DEFINITIONS AND MACROS                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        TYPEDEFS, CLASSES AND STRUCTURES                   */
/*---------------------------------------------------------------------------*/

/*                          =======================
============================   CLASS DEFINITION    ============================
                            =======================                       *//**
  UpdateESPClass.

*//*=========================================================================*/
class UpdateESPClass : public UpdateClassVirt {
public:
    ~UpdateESPClass() {};

    virtual bool prepareUpdate(uint32_t upd_size, String &upd_md5, uint16_t command);

    virtual bool update(uint8_t *file_buf, size_t buf_size);

    virtual bool end(void);

    virtual void sm(String *statusMessage);

private:
    String* _statusMessage;
    uint32_t _upd_size;
    String* _upd_md5;
    uint16_t _command;
};

/*---------------------------------------------------------------------------*/
/*                                GLOBAL VARIABLES                           */
/*---------------------------------------------------------------------------*/

extern UpdateESPClass UpdateESP;

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
#endif // __UpdateESPClass_h__
