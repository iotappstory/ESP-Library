#include "../config.h"
#if OTA_UPD_CHECK_NEXTION == true
	/*                          =======================
	============================   C/C++ HEADER FILE   ============================
								=======================                       *//**
	  UpdateNextionClass.h

	  Created by Onno Dirkzwager on 10.02.2019.
	  Copyright (c) 2019 IOTAppStory. All rights reserved.

	*///===========================================================================

	#ifndef __UpdateNextionClass_h__
		#define __UpdateNextionClass_h__

		/*---------------------------------------------------------------------------*/
		/*                                    INCLUDES                               */
		/*---------------------------------------------------------------------------*/

		
		#include "UpdateClassVirt.h"
		#include <ESPNexUpload.h>

		/*---------------------------------------------------------------------------*/
		/*                            DEFINITIONS AND MACROS                         */
		/*---------------------------------------------------------------------------*/

		/*---------------------------------------------------------------------------*/
		/*                        TYPEDEFS, CLASSES AND STRUCTURES                   */
		/*---------------------------------------------------------------------------*/

		/*                          =======================
		============================   CLASS DEFINITION    ============================
									=======================                       *//**
		  UpdateNextionClass.

		*//*=========================================================================*/
		class UpdateNextionClass : public UpdateClassVirt {
			public:
				~UpdateNextionClass() {};

				virtual bool prepareUpdate(uint32_t upd_size, String &upd_md5, uint16_t command);

				virtual bool update(uint8_t *file_buf, size_t buf_size);

				virtual bool end(void);

				virtual void sm(String *statusMessage);

				ESPNexUpload nextion{NEXT_BAUD};

			private:
				String* _statusMessage;
		};

		/*---------------------------------------------------------------------------*/
		/*                                GLOBAL VARIABLES                           */
		/*---------------------------------------------------------------------------*/

		extern UpdateNextionClass UpdateNextion;

		/*---------------------------------------------------------------------------*/
		/*                                    EOF                                    */
		/*---------------------------------------------------------------------------*/
	#endif // __UpdateNextionClass_h__
#endif
