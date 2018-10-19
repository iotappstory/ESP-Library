/**
 *
 * @file ESPHTTPUpdateIasMod.cpp
 * @date 01.01.2018
 * @author Onno Dirkzwager
 *
 * This is a heavily modified version of the original ESP8266HTTPUpdate.cpp written by Markus Sattler
 * It is specifically modified for IotAppStory.com But feel free to use it in any way you would like to.
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file was originally part of the ESP8266 Http Updater.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "ESPhttpUpdateIasMod.h"
#include <StreamString.h>

extern "C" uint32_t _SPIFFS_start;
extern "C" uint32_t _SPIFFS_end;


/**
 *
 * @param http HTTPClient *
 * @param currentVersion const char *
 * @return HTTPUpdateResult
 */
bool ESP8266HTTPUpdate::handleUpdate(HTTPClient& http, int len, bool spiffs){
	
	if(len <= 0){
		#if DEBUG_LVL >= 2
			DEBUG_PRINTF_P(PSTR(" Error: Received Content-Length header is %d"), len);
		#else
			DEBUG_PRINT(F(" Error: Content-Length"));
		#endif
	}else{

		int freeSpace;
		bool error = false;
		
		#if defined ESP8266
		if(spiffs){
			// current spiffs space
			freeSpace = ((size_t) &_SPIFFS_end - (size_t) &_SPIFFS_start);
		}else{
			// current FreeSketchSpace
			freeSpace = ESP.getFreeSketchSpace();
		}
		
	
		if(len > freeSpace){
			#if DEBUG_LVL >= 2
				DEBUG_PRINTF_P(PSTR(" Error: Not enough space (%d) update size: %d"), freeSpace, len);
			#else
				DEBUG_PRINT(F(" Error: space"));
			#endif
			
			// if the Received Content-Length is larger than the free space exit update proces and return error
			//return String(printf_P(PSTR("Not enough space (%d) update size: %d"), freeSpace, len));
		}else{
		#endif
		
			WiFiClient * tcp = http.getStreamPtr();
			
			#if defined ESP8266
				WiFiUDP::stopAll();
				WiFiClient::stopAllExcept(tcp);
			#elif defined ESP32

			#endif
			

			delay(100);

			int command;
			if(spiffs){
				
				command = U_SPIFFS;
				//DEBUG_PRINTLN("[httpUpdate] runUpdate spiffs...\n");
				
			}else{
				
				command = U_FLASH;
				//DEBUG_PRINTLN("[httpUpdate] runUpdate flash...\n");

				
				#if defined ESP8266
					// Analise received bin file
					uint8_t buf[4];
					if(tcp->peekBytes(&buf[0], 4) != 4) {
						#if DEBUG_LVL >= 2
							DEBUG_PRINTLN(F(" Error: Failed to verify the bin header"));
						#endif
						error = true;
					}
					if(buf[0] != 0xE9) {
						#if DEBUG_LVL >= 2
							DEBUG_PRINTLN(F(" Error: Header did not start with magic byte 0xE9"));
						#endif
						error = true;
					}
					uint32_t bin_flash_size = ESP.magicFlashChipSize((buf[3] & 0xf0) >> 4);
					if(bin_flash_size > ESP.getFlashChipRealSize()) {
						#if DEBUG_LVL >= 2
							DEBUG_PRINTLN(F(" Error: Received file to large for flash"));
						#endif
						error = true;
					}
				#elif defined ESP32

				#endif

				
			}
			
			if(!error){
				if(runUpdate(*tcp, len, http.header("x-MD5"), command)){
					#if DEBUG_LVL >= 1
						DEBUG_PRINT(F(" Received & installed: "));
						DEBUG_PRINT(http.header("x-name").c_str());
						DEBUG_PRINT(F(" "));
						DEBUG_PRINT(http.header("x-ver").c_str());
					#endif
					http.end();

					if(spiffs == false) {
						return true;
					}else{
						return false;
					}

				}
			}
			#if DEBUG_LVL < 2
			else{
				DEBUG_PRINTLN(F(" Error: File error"));
			}
			#endif
		
#if defined ESP8266		
		}
#endif
	}
	return false;
}


/**
 * write Update to flash
 * @param in Stream&
 * @param size uint32_t
 * @param md5 String
 * @return true if Update ok
 */
bool ESP8266HTTPUpdate::runUpdate(Stream& in, uint32_t size, String md5, int command){

	#if DEBUG_LVL == 3
		StreamString error;
	#endif


    if(!Update.begin(size, command)) {
		#if DEBUG_LVL == 3
			Update.printError(error);
			error.trim(); // remove line ending
			DEBUG_PRINTF_P(PSTR(" Error: Update.begin failed! (%s)\n"), error.c_str());
		#endif
        return false;
    }
	


    if(md5.length()) {
        if(!Update.setMD5(md5.c_str())) {
			#if DEBUG_LVL == 3
				DEBUG_PRINTF_P(PSTR(" Error: Update.setMD5 failed! (%s)\n"), md5.c_str());
			#endif
            return false;
        }
	}


    if(Update.writeStream(in) != size) {
		#if DEBUG_LVL == 3
			Update.printError(error);
			error.trim(); // remove line ending
			DEBUG_PRINTF_P(PSTR(" Error: Update.writeStream failed! (%s)\n"), error.c_str());
		#endif
        return false;
    }
	
	#if DEBUG_LVL >= 1
		DEBUG_PRINT(F("."));
	#endif

    if(!Update.end()) {
		#if DEBUG_LVL == 3
			Update.printError(error);
			error.trim(); // remove line ending
			DEBUG_PRINTF_P(PSTR(" Error: Update.end failed! (%s)\n"), error.c_str());
		#endif
        return false;
    }
	
	#if DEBUG_LVL >= 1
		DEBUG_PRINT(F("."));
	#endif

    return true;
}


