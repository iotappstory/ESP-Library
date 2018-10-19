#ifdef  ESP8266
	#include "boardInfo.h"

	boardInfo::boardInfo(int &bootTimes, char &boardMode){
		_bootTimes = &bootTimes;
		_boardMode = &boardMode;
	}



	void boardInfo::read(){
		rtcMemDef rtcMem;
		
		system_rtc_mem_read(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
		if (rtcMem.markerFlag != MAGICBYTE) {
			rtcMem.markerFlag = MAGICBYTE;
			rtcMem.bootTimes = 0;
			rtcMem.boardMode = 'N';
			system_rtc_mem_write(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
		}
		(*_boardMode) = rtcMem.boardMode;
		(*_bootTimes) = rtcMem.bootTimes;
	}


	void boardInfo::write(){
		rtcMemDef rtcMem;
		
		rtcMem.boardMode = (*_boardMode);
		rtcMem.bootTimes = (*_bootTimes);
		
		rtcMem.markerFlag = MAGICBYTE;
		system_rtc_mem_write(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
	}
#endif