/**
 *
 * @file ESPhttpUpdateIasMod.cpp
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

#ifndef ESPHTTPUPDATEIASMOD_H_
	#define ESPHTTPUPDATEIASMOD_H_

	#include <Arduino.h>


	
	
	#ifdef ESP32
		#include <WiFi.h>
		#include <HTTPClient.h>
		#include <Update.h>
	#else
		#include <ESP8266WiFi.h>
		#include <ESP8266HTTPClient.h>
	#endif


	#include <WiFiClient.h>
	#include <WiFiUdp.h>
	
	
	
  #ifndef DEBUG_LVL
		#define DEBUG_LVL 2	// 1, 2 or 3
		#if DEBUG_LVL >= 1
			#define         DEBUG_PRINT(x)    { Serial.print(x);  }
			#define         DEBUG_PRINTF(...) { Serial.printf(__VA_ARGS__);  }
			#define         DEBUG_PRINTF_P(...) { Serial.printf_P(__VA_ARGS__);  }
			#define         DEBUG_PRINTLN(x)  { Serial.println(x); }
		#endif
	#endif

	class ESP8266HTTPUpdate{
		
		public:


			bool handleUpdate(HTTPClient& http, int len, bool spiffs);


		protected:

			bool runUpdate(Stream& in, uint32_t size, String md5, int command = U_FLASH);
		
	};

#endif
