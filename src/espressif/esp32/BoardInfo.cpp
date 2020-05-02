/*                          =======================
============================   C/C++ SOURCE FILE   ============================
                            =======================                       *//**
  BoardInfo.cpp

  Created by Onno Dirkzwager on 22.11.2018.
  Copyright (c) 2018 IOTAppStory. All rights reserved.

*///===========================================================================

#ifdef ESP32

/*---------------------------------------------------------------------------*/
/*                                INCLUDES                                   */
/*---------------------------------------------------------------------------*/

#include "BoardInfo.h"

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
                        BoardInfo constructor

    @param bootTimes int &
    @param boardMode char &

*///---------------------------------------------------------------------------
BoardInfo::BoardInfo(int &bootTimes, char &boardMode) {
    this->_bootTimes = &bootTimes;
    this->_boardMode = &boardMode;
}

/*-----------------------------------------------------------------------------
                        BoardInfo read

*///---------------------------------------------------------------------------
void BoardInfo::read() {
    Preferences preferences;

    // Open Preferences
    preferences.begin("boardInfo", false);

    // Get the boardMode value, if the key does not exist, return a default value of 'N'
    (*this->_boardMode)   = preferences.getUInt("boardMode", 'N');

    // Get the bootTimes value, if the key does not exist, return a default value of 0
    (*this->_bootTimes)   = preferences.getUInt("bootTimes", 0);

    // Close the Preferences
    preferences.end();
}

/*-----------------------------------------------------------------------------
                        BoardInfo write

*///---------------------------------------------------------------------------
void BoardInfo::write() {
    Preferences preferences;

    // Open Preferences
    preferences.begin("boardInfo", false);

    // Store the boardMode value
    preferences.putUInt("boardMode", (*this->_boardMode));

    // Store the bootTimes value
    preferences.putUInt("bootTimes", (*this->_bootTimes));

    // Close the Preferences
    preferences.end();
}

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
#endif // ESP32
