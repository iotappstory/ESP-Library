/*
  Copyright (c) [2019] [Onno Dirkzwager]
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

char tempValue[NROFADDEDFIELDS][LARGESTLENGTH];



// ================================================ functions =========================================
void runUpgrade(){
  oldConfig oldCfg;
  const int oldCfgStartAddress  = 0;
  const int oldMagicBytesBegin  = oldCfgStartAddress + sizeof(oldConfig) - 4;
  int nrOfFieldsFound           = 0;

  EEPROM.begin(EEPROM_SIZE);

  
  Serial.println(FPSTR(SER_DEV));
  Serial.printf(" Searching for the old(v2.0.X) config struct magic bytes(%s) at: %d\n", MAGICBYTES, oldMagicBytesBegin);

  // check for magicBytes to confirm the old config struct is stored in EEPROM
  if(EEPROM.read(oldMagicBytesBegin) == MAGICBYTES[0] && EEPROM.read(oldMagicBytesBegin + 1) == MAGICBYTES[1] && EEPROM.read(oldMagicBytesBegin + 2) == MAGICBYTES[2]){
    Serial.printf(" Found config! Size: %d\n", sizeof(oldConfig));
    
    // get the old config from EEPROM
    EEPROM.get(oldCfgStartAddress, oldCfg);

    // This function shows the old config values. Comment this line out for remote devices!
    showOldConfig(oldCfg);

    // find and temp store added field values
    if(NROFADDEDFIELDS > 0){
      nrOfFieldsFound = findAddedFields();
    }
    EEPROM.end();
    
    
    /**
      If you need to "backup" old EEPROM values that are not within the config & added field structs. This is the place to put your code!
      As we have the old config & added fields in memory and will after erasing all EEPROM write them in the lines below.
    */

    
    #if WRITECHANGES == true
      Serial.println(FPSTR(SER_DEV));
      IAS.eraseEEPROM('F');
      Serial.println(FPSTR(SER_DEV));
      
      Serial.println(F(" Write WiFi credentials from the old config to the new WiFi struct in EEPROM"));
      
      // Write WiFi credentials from the old config to the new WiFi struct in EEPROM
      WiFiConnector WiFiConn;
      if(oldCfg.ssid[0] != ""){WiFiConn.addAPtoEEPROM(oldCfg.ssid[0], oldCfg.password[0], 1);}
      if(oldCfg.ssid[1] != ""){WiFiConn.addAPtoEEPROM(oldCfg.ssid[1], oldCfg.password[1], 2);}
      if(oldCfg.ssid[2] != ""){WiFiConn.addAPtoEEPROM(oldCfg.ssid[2], oldCfg.password[2], 3);}

     
      
      // write the board details to the new config struct in EEPROM
      configStruct newConfig;
      strcpy(newConfig.actCode, oldCfg.actCode);
      strcpy(newConfig.appName, oldCfg.appName);
      strcpy(newConfig.appVersion, oldCfg.appVersion);
      strcpy(newConfig.deviceName, oldCfg.deviceName);
      strcpy(newConfig.compDate, oldCfg.compDate);
    
      #if defined  ESP8266
        strcpy(newConfig.sha1, oldCfg.sha1);
      #endif
      #if CFG_AUTHENTICATE == true
        strcpy(newConfig.cfg_pass, oldCfg.cfg_pass);
      #endif
      
      Serial.println(F(" Write the board details to the new config struct in EEPROM"));
      IAS.writeConfig(newConfig);
  
      // write the found fields to the new field struct in EEPROM
      if(nrOfFieldsFound > 0){
        writeAddedFields(nrOfFieldsFound);
      }
    #endif
    
  }else{
    Serial.println(F(" Could not find config!"));
    EEPROM.end();
  }
  Serial.println(FPSTR(SER_DEV));
}



void showOldConfig(oldConfig &oldCfg){
  
  Serial.println(FPSTR(SER_DEV));
  Serial.println(F("\n Old config values"));
  Serial.print(F(" actCode\t\t: "));
  Serial.println(oldCfg.actCode);
  Serial.print(F(" appName\t\t: "));
  Serial.println(oldCfg.appName);
  Serial.print(F(" appVersion\t: "));
  Serial.println(oldCfg.appVersion);

  Serial.print(F("\n SSID 1\t\t: "));
  Serial.println(oldCfg.ssid[0]);
  Serial.print(F(" Pass 1\t\t: "));
  Serial.println(oldCfg.password[0]);
  Serial.print(F(" SSID 2\t\t: "));
  Serial.println(oldCfg.ssid[1]);
  Serial.print(F(" Pass 2\t\t: "));
  Serial.println(oldCfg.password[1]);
  Serial.print(F(" SSID 3\t\t: "));
  Serial.println(oldCfg.ssid[2]);
  Serial.print(F(" Pass 3\t\t: "));
  Serial.println(oldCfg.password[2]);

  Serial.print(F("\n deviceName\t: "));
  Serial.println(oldCfg.deviceName);
  Serial.print(F(" compDate\t: "));
  Serial.println(oldCfg.compDate);

  #if defined  ESP8266
    Serial.print(" sha1\t\t: ");
    Serial.println(oldCfg.sha1);
  #endif
}



int findAddedFields(){
  
  int lastFieldEndAt = 0;
  int fieldsFound = 0;

  Serial.println(F(""));
  Serial.println(FPSTR(SER_DEV));
  Serial.print(F(" Searching for "));
  Serial.print(NROFADDEDFIELDS);
  Serial.println(F(" added fields.\n"));
  
  // field loop
  for(int fnr=0; fnr < NROFADDEDFIELDS; fnr++){

    bool fieldFound = false;
    int tempValCount = 0;
    int fieldSearchStartAddress;
    
    if(lastFieldEndAt == 0){
      fieldSearchStartAddress = sizeof(oldConfig);
    }else{
      fieldSearchStartAddress = lastFieldEndAt;
    }
    
    Serial.print(F(" Field nr "));
    Serial.println(fnr+1);
    //Serial.print("Search started at ");
    //Serial.println(fieldSearchStartAddress);

    // EEPROM loop search for field values
    for(int eep=fieldSearchStartAddress; eep < EEPROM_SIZE; eep++){

      if(!fieldFound){
        // read eeprom, check for MAGICEEP
        if(EEPROM.read(eep) == MAGICEEP[0]){
          fieldFound = true;
        }
      }else{
        // add read char from EEPROM to tempValue
        tempValue[fnr][tempValCount] = EEPROM.read(eep);

        // read the next byte
        if(EEPROM.read(eep+1) == '^'){
          // if the next byte is the value "end" byte end this eeprom loop and continu with the next value
          lastFieldEndAt = eep;
          eep = EEPROM_SIZE;
          fieldsFound++;
        }else{
          // if the next byte is NOT the value "end" byte continu with the next value
          tempValCount++;
        }
      }
    }

    if(fieldFound){
      Serial.print(F(" foundVal: "));
      Serial.println(tempValue[fnr]);

      // reset for the next field
      fieldFound = false;
    }else{
      Serial.println(F(" Could not find."));
    }
    Serial.println(F("---------------------"));
    
  }
  Serial.print(F(" Total nr of fields found: "));
  Serial.println(fieldsFound);

  return fieldsFound;
}



void writeAddedFields(int fieldsFound){
  
  int _nrXFlastAdd  = 0;
  
  Serial.println(FPSTR(SER_DEV));
  Serial.print(F(" Writing "));
  Serial.print(fieldsFound);
  Serial.println(F(" fields to new EEPROM locations"));

  // EEPROM begin
  EEPROM.begin(EEPROM_SIZE);
  
  for(int i=0; i < fieldsFound; i++){

    // init fieldStruct
    addFieldStruct fieldStruct;
    
    // calculate EEPROM addresses
    const int eepStartAddress = FIELD_EEP_START_ADDR + (i * sizeof(fieldStruct));
    const int eepEndAddress = eepStartAddress + sizeof(fieldStruct);
    const int magicBytesBegin = eepEndAddress - 3;
    int eepFieldStart;
    
    if(i == 0){
      eepFieldStart = FIELD_EEP_START_ADDR + (MAXNUMEXTRAFIELDS * sizeof(fieldStruct)) + _nrXFlastAdd;
    }else{
      eepFieldStart = _nrXFlastAdd;
    }
    _nrXFlastAdd = eepFieldStart + fieldMaxLength[i] + 1;
    

    // add values to the fieldstruct
    fieldStruct.length  = fieldMaxLength[i];
    fieldStruct.type    = fieldType[i];
    
    // put the fieldStruct to EEPROM
    EEPROM.put(eepStartAddress, fieldStruct);
    
    // temp val buffer
    char eepVal[fieldMaxLength[i]+1];
    strcpy(eepVal, tempValue[i]);
    
    // put the field value to EEPROM
    unsigned int ee = 0;
    for(unsigned int e=eepFieldStart; e < _nrXFlastAdd; e++){
      EEPROM.write(e, eepVal[ee]);
      ee++;
    }

    // print details to EEPROM
    if(i > 0){
      Serial.println(F("---------------------"));
    }
    Serial.print(F(" fieldValue\t: "));
    Serial.println(tempValue[i]);
    Serial.print(F(" fieldType\t: "));
    Serial.println(fieldType[i]);
    Serial.print(F(" fieldMaxLength\t: "));
    Serial.println(fieldMaxLength[i]);

    Serial.print(F(" Written field struct from "));
    Serial.print(eepStartAddress);
    Serial.print(F(" to "));
    Serial.println(eepEndAddress);

    Serial.print(F(" Written value from "));
    Serial.print(eepFieldStart);
    Serial.print(F(" to "));
    Serial.println(_nrXFlastAdd);
  }
  delay(200);
  
  // EEPROM end
  EEPROM.end();
  
 
  DEBUG_PRINTLN("");
}
