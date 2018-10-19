#ifdef  ESP32
	#include "boardInfo.h"

	boardInfo::boardInfo(int &bootTimes, char &boardMode){
		_bootTimes = &bootTimes;
		_boardMode = &boardMode;
	}



	void boardInfo::read(){
		Preferences preferences;
		
		// Open Preferences
		preferences.begin("boardInfo", false);
		
		// Get the boardMode value, if the key does not exist, return a default value of 'N'
		(*_boardMode)	= preferences.getUInt("boardMode", 'N');
		
		// Get the bootTimes value, if the key does not exist, return a default value of 0
		(*_bootTimes)	= preferences.getUInt("bootTimes", 0);
		
		// Close the Preferences
		preferences.end();
	}


	void boardInfo::write(){
		Preferences preferences;
		
		// Open Preferences
		preferences.begin("boardInfo", false);
		
		// Store the boardMode value
		preferences.putUInt("boardMode", (*_boardMode));
		
		// Store the bootTimes value
		preferences.putUInt("bootTimes", (*_bootTimes));
		
		// Close the Preferences
		preferences.end();
	}
#endif
