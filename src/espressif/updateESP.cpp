#include <Arduino.h>
#include "updateESP.h"


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



bool UpdateESPClass::prepareUpdate(uint32_t upd_size, String &upd_md5, uint16_t command){
    
	_upd_size 	= upd_size;
	_upd_md5	= &upd_md5;
    _command 	= command;
	
	/**
		Check for enough space before starting the update
	*/
	if(_command == U_SPIFFS){
		#ifdef ESP32
			const esp_partition_t* _partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
			if(!_partition){
				(*_statusMessage) = F("Partition Could Not be Found");
				return false;
			}

			if(_upd_size > _partition->size) {
				#ifdef DEBUG_SERIAL_ENABLE
					dbSerialPrintf("SPIFFS update to large! (%d) free: %d\n", _upd_size, _partition->size);
				#endif
				
				(*_statusMessage) = F("SPIFFS update to large!");
				return false;
			}
		#elif defined ESP8266
			size_t spiffsSize = ((size_t) &_SPIFFS_end - (size_t) &_SPIFFS_start);
			if(_upd_size > (int) spiffsSize) {
				#ifdef DEBUG_SERIAL_ENABLE
					dbSerialPrintf("SPIFFS update to large! (%d) free: %d\n", _upd_size, spiffsSize);
				#endif
				
				(*_statusMessage) = F("SPIFFS update to large!");
				return false;
			}
		#endif
	}else{
		if(_upd_size > (int) ESP.getFreeSketchSpace()) {
			#ifdef DEBUG_SERIAL_ENABLE
				dbSerialPrintf("Sketch update to large! (%d) free: %d\n", _upd_size, ESP.getFreeSketchSpace());
			#endif
			
			(*_statusMessage) = F("Sketch update to large!");
			return false;
		}
	}

	
	/**
		Initiate the esp update class
	*/
	if(!Update.begin(_upd_size, _command)) {
		#ifdef DEBUG_SERIAL_ENABLE
			StreamString stError;
			Update.printError(stError);
			stError.trim();
			
			dbSerialPrint(F("Update.begin failed! "));
			dbSerialPrintln(stError.c_str());
		#endif

		(*_statusMessage) = F("Update.begin failed! ");
		return false;
	}
	//Update.runAsync(true);
  

	/**
		add and check the received md5 string
	*/
	if(_upd_md5->length()) {
		
		if(!Update.setMD5(_upd_md5->c_str())) {
			#ifdef DEBUG_SERIAL_ENABLE
				dbSerialPrintln(F("Update.setMD5 failed! "));
				dbSerialPrint(F(" Received md5 string: "));
				dbSerialPrintln(_upd_md5->c_str());
			#endif
			
			(*_statusMessage) = F("Update.setMD5 failed!");
		
			return false;
		}
	}
	
	return true;
}



bool UpdateESPClass::update(uint8_t *file_buf, size_t buf_size){
	
	// Write the buffered bytes to the esp. If this fails, return false.
	if(!Update.write(file_buf, buf_size)){
		(*_statusMessage) = F("Update.write failed!");
		return false;
	}
	
    return true;  
}



bool UpdateESPClass::end(){
	
	_upd_size = 0;
	
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
		
		(*_statusMessage) = F("Update.end failed!");
		
		return false;
	}

    return true;
}



void UpdateESPClass::sm(String *statusMessage){
	_statusMessage = statusMessage;
}
	


UpdateESPClass UpdateESP;