/**
 * @file NexUpload.cpp
 *
 * The implementation of uploading tft file for nextion displays. 
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

#include "ESPNexUpload.h"
#include <FS.h>
#include <IOTAppStory.h>								// IAS.com library
#if defined ESP8266
	#include <SoftwareSerial.h>
#else
	#include <SPIFFS.h>	
	#include <HardwareSerial.h>
#endif

#define DEBUG_SERIAL_ENABLE

#ifdef DEBUG_SERIAL_ENABLE
#define dbSerialPrint(a)    Serial.print(a)
#define dbSerialPrintln(a)  Serial.println(a)
#define dbSerialBegin(a)    Serial.begin(a)
#else
#define dbSerialPrint(a)    do{}while(0)
#define dbSerialPrintln(a)  do{}while(0)
#define dbSerialBegin(a)    do{}while(0)
#endif



#if defined ESP8266
	ESPNexUpload::ESPNexUpload(const char *file_name,uint32_t download_baudrate, SoftwareSerial *softSerial)
#else
	ESPNexUpload::ESPNexUpload(const char *file_name,uint32_t download_baudrate, HardwareSerial *softSerial)
#endif
{
    _file_name = file_name; 
    _download_baudrate = download_baudrate;
    nexSerial = softSerial;
}

bool ESPNexUpload::upload(void) {
    String temp = "";
    return this->upload(temp);
}

bool ESPNexUpload::upload(String &statusMessage)
{
    dbSerialBegin(115200);
    if(!_checkFile())
    {
        dbSerialPrintln("checkFile error");
        statusMessage = "checkFile error";
        return false;
    }

    if(_getBaudrate() == 0)
    {
        dbSerialPrintln("get baudrate error");
        statusMessage = "get baudrate error";
        return false;
    }
    if(!_setDownloadBaudrate(_download_baudrate))
    {
        dbSerialPrintln("modify baudrate error");
        statusMessage = "modifybaudrate error";
        return false;
    }
		
    if(!_downloadTftFile())
    {
        dbSerialPrintln("download file error");
        statusMessage = "download file error";
        return false;
    }
    dbSerialPrintln("download ok\r\n");
    statusMessage = "download ok";
    return true;
}

uint16_t ESPNexUpload::_getBaudrate(void)
{
    _baudrate = 0;
    uint32_t baudrate_array[7] = {115200,19200,9600,57600,38400,4800,2400};
    for(uint8_t i = 0; i < 7; i++)
    {
        if(_searchBaudrate(baudrate_array[i]))
        {
            _baudrate = baudrate_array[i];
            dbSerialPrintln("get baudrate");
            break;
        }
    }
    return _baudrate;
}

bool ESPNexUpload::_checkFile(void)
{
    if (SPIFFS.exists(_file_name)) { 
        _myFile = SPIFFS.open(_file_name, "r");                   
        dbSerialPrintln("file found");
        dbSerialPrintln(_file_name);
        
        _undownloadByte = _myFile.size();
        dbSerialPrintln("file size is:");
        dbSerialPrintln(_undownloadByte);
        dbSerialPrintln("check file ok");
        return 1;
    }
    return 0;
}

bool ESPNexUpload::_searchBaudrate(uint32_t baudrate)
{
    String string = String("");  


		nexSerial->begin(baudrate);
		
    this->sendCommand("");
    this->sendCommand("connect");
    this->recvRetString(string);  
    if(string.indexOf("comok") != -1)
    {
        return 1;
    } 
    return 0;
}

void ESPNexUpload::sendCommand(const char* cmd)
{
    while (nexSerial->available())
    {
        nexSerial->read();
    }

    nexSerial->print(cmd);
    nexSerial->write(0xFF);
    nexSerial->write(0xFF);
    nexSerial->write(0xFF);
}

uint16_t ESPNexUpload::recvRetString(String &string, uint32_t timeout,bool recv_flag)
{
    uint16_t ret = 0;
    uint8_t c = 0;
    long start;
    bool exit_flag = false;
    start = millis();
    while (millis() - start <= timeout)
    {
        while (nexSerial->available())
        {
            c = nexSerial->read(); 
            if(c == 0)
            {
                continue;
            }
            string += (char)c;
            if(recv_flag)
            {
                if(string.indexOf(0x05) != -1)
                { 
                    exit_flag = true;
                } 
            }
        }
        if(exit_flag)
        {
            break;
        }
    }
    ret = string.length();
    return ret;
}

bool ESPNexUpload::_setDownloadBaudrate(uint32_t baudrate)
{
    String string = String(""); 
    String cmd = String("");
    
    String filesize_str = String(_undownloadByte,10);
    String baudrate_str = String(baudrate);
    cmd = "whmi-wri " + filesize_str + "," + baudrate_str + ",0";

    this->sendCommand("");
    this->sendCommand(cmd.c_str());

    dbSerialPrintln("Changing baudrate...");
		nexSerial->begin(baudrate);
    
    this->recvRetString(string);

    if(string.indexOf(0x05) != -1)
    { 
        return 1;
    } 
    return 0;
}

bool ESPNexUpload::_downloadTftFile(void)
{
	
		nexSerial->begin(57600);
		
    uint8_t c;
    uint16_t send_timer = 0;
    uint16_t last_send_num = 0;
    String string = String("");
    send_timer = _undownloadByte / 4096 + 1;
    last_send_num = _undownloadByte % 4096;

    while(send_timer)
    {

        if(send_timer == 1)
        {
            for(uint16_t j = 1; j <= 4096; j++)
            {
                if(j <= last_send_num)
                {
                    c = _myFile.read();
                    nexSerial->write(c);
                }
                else
                {
                    break;
                }
            }
        }

        else
        {
            for(uint16_t i = 1; i <= 4096; i++)
            {
                c = _myFile.read();
                nexSerial->write(c);
            }
        }
        this->recvRetString(string,500,true);  
        if(string.indexOf(0x05) != -1)
        { 
            string = "";
        } 
        else
        {
            return 0;
        }
         --send_timer;
    }
    return true;  
}

