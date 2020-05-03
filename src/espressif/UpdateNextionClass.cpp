#include "../config.h"
#if OTA_UPD_CHECK_NEXTION == true
	/*                          =======================
	============================   C/C++ SOURCE FILE   ============================
								=======================                       *//**
	  UpdateNextion.cpp

	  Created by Onno Dirkzwager on 10.02.2019.
	  Copyright (c) 2019 IOTAppStory. All rights reserved.

	*///===========================================================================

	/*---------------------------------------------------------------------------*/
	/*                                INCLUDES                                   */
	/*---------------------------------------------------------------------------*/

	#include <Arduino.h>
	#include "UpdateNextionClass.h"

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

	UpdateNextionClass UpdateNextion;

	/*---------------------------------------------------------------------------*/
	/*                        FUNCTION IMPLEMENTATION                            */
	/*---------------------------------------------------------------------------*/

	/*-----------------------------------------------------------------------------
							UpdateNextionClass prepareUpdate

	*///---------------------------------------------------------------------------
	bool UpdateNextionClass::prepareUpdate(uint32_t upd_size, String &upd_md5, uint16_t command) {

		// prepare upload: setup serial connection, send update command and send the expected update size
		if(!this->nextion.prepareUpload(upd_size)) {
			(*this->_statusMessage) = this->nextion.statusMessage;
			return false;
		}

		return true;
	}

	/*-----------------------------------------------------------------------------
							UpdateNextionClass update

	*///---------------------------------------------------------------------------
	bool UpdateNextionClass::update(uint8_t *file_buf, size_t buf_size) {

		// Write the buffered bytes to the nextion display. If this fails, return false.
		if(!this->nextion.upload(file_buf, buf_size)) {
			(*this->_statusMessage) = this->nextion.statusMessage;
			return false;
		}

		return true;
	}

	/*-----------------------------------------------------------------------------
							UpdateNextionClass end

	*///---------------------------------------------------------------------------
	bool UpdateNextionClass::end() {
		// end: wait(delay) for the nextion to finish the update process, send nextion reset command and end the serial connection to the nextion
		this->nextion.end();
	}

	/*-----------------------------------------------------------------------------
							UpdateNextionClass sm

	*///---------------------------------------------------------------------------
	void UpdateNextionClass::sm(String *statusMessage) {
		this->_statusMessage = statusMessage;
	}

	/*---------------------------------------------------------------------------*/
	/*                                    EOF                                    */
	/*---------------------------------------------------------------------------*/
#endif
