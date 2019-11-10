#include <Arduino.h>
#include "updateNextion.h"



bool UpdateNextionClass::prepareUpdate(uint32_t upd_size, String &upd_md5, uint16_t command){

	// prepare upload: setup serial connection, send update command and send the expected update size
	if(!nextion.prepareUpload(upd_size)){
		(*_statusMessage) = nextion.statusMessage;
		return false;
	}

	return true;
}



bool UpdateNextionClass::update(uint8_t *file_buf, size_t buf_size){

	// Write the buffered bytes to the nextion display. If this fails, return false.
	if(!nextion.upload(file_buf, buf_size)){
		(*_statusMessage) = nextion.statusMessage;
		return false;
	}

    return true;
}



bool UpdateNextionClass::end(){
	// end: wait(delay) for the nextion to finish the update process, send nextion reset command and end the serial connection to the nextion
	nextion.end();
}



void UpdateNextionClass::sm(String *statusMessage){
	_statusMessage = statusMessage;
}



UpdateNextionClass UpdateNextion;