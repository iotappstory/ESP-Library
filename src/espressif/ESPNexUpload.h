/**
 * @file NexUpload.h
 *
 * The definition of class NexUpload. 
 * 
 * Modified to work with ESP8266 and SoftwareSerial
 * @author Ville Vilpas (psoden@gmail.com)
 * @date   2018/2/3
 * @version 0.2.0
 *
 * Original version (a part of https://github.com/itead/ITEADLIB_Arduino_Nextion)
 * @author  Chen Zengpeng (email:<zengpeng.chen@itead.cc>)
 * @date    2016/3/29
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __ESPNEXUPLOAD_H__
#define __ESPNEXUPLOAD_H__
#include <Arduino.h>
#include <IOTAppStory.h>								// IAS.com library
#if defined ESP8266
	#include <SoftwareSerial.h>
#else
	#include <HardwareSerial.h>
#endif
/**
 * @addtogroup CoreAPI 
 * @{ 
 */

/**
 *
 * Provides the API for nextion to download the ftf file.
 */
class ESPNexUpload
{
public: /* methods */

	/**
		* Constructor. 
		* 
		* @param file_name - tft file name. 
		* @param download_baudrate - set download baudrate.
	*/
	#if defined ESP8266
		ESPNexUpload(WiFiClientSecure file_name, uint32_t file_size, uint32_t download_baudrate, SoftwareSerial *softSerial);
	#else
		ESPNexUpload(WiFiClientSecure file_name, uint32_t file_size, uint32_t download_baudrate, HardwareSerial *softSerial);
	#endif
    
    
	/**
		* destructor. 
		* 
	*/
    ~ESPNexUpload(){}
    
	/*
		* start download.
		*
		* @return true if success, false for failure.
	*/
    bool upload();
    bool upload(String &statusMessage);

private: /* methods */

	/*
		* get communicate baudrate. 
		* 
		* @return communicate baudrate.
		*
	*/
    uint16_t _getBaudrate(void);
    
	/*
		* check tft file.
		*
		* @return true if success, false for failure. 
	*/
    //bool _checkFile(void);

	/*
		* search communicate baudrate.
		*
		* @param baudrate - communicate baudrate.
		*   
		* @return true if success, false for failure. 
	*/
    bool _searchBaudrate(uint32_t baudrate);

	/*
		* set download baudrate.
		*
		* @param baudrate - set download baudrate.
		*   
		* @return true if success, false for failure. 
	*/
    bool _setDownloadBaudrate(uint32_t baudrate);
    
	/**
		* start dowload tft file to nextion. 
		* 
		* @return none. 
	*/
    bool _downloadTftFile(void);
    
	/*
		* Send command to Nextion.
		*
		* @param cmd - the string of command.
		*
		* @return none.
	*/
    void sendCommand(const char* cmd);

	/*
		* Receive string data. 
		* 
		* @param buffer - save string data.  
		* @param timeout - set timeout time. 
		* @param recv_flag - if recv_flag is true,will braak when receive 0x05.
		*
		* @return the length of string buffer.
		*
	*/   
    uint16_t recvRetString(String &string, uint32_t timeout = 500,bool recv_flag = false);
    
private: /* data */ 
    uint32_t _baudrate; /*nextion serail baudrate*/
    const char *_file_name; /*nextion tft file name*/
    WiFiClientSecure _myFile; /*nextion tft file*/
    uint32_t _undownloadByte; /*undownload byte of tft file*/
    uint32_t _download_baudrate; /*download baudrate*/
    #if defined ESP8266
		SoftwareSerial *nexSerial;
	#else
		HardwareSerial *nexSerial;
	#endif
};
/**
 * @}
 */

#endif /* #ifndef __ESPNEXUPLOAD_H__ */
