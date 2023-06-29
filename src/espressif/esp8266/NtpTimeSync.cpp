/*                          =======================
============================   C/C++ SOURCE FILE   ============================
                            =======================                       *//**
  NtpTimeSync.cpp

  Created by Onno Dirkzwager on 20.12.2020.
  Copyright (c) 2020 IOTAppStory. All rights reserved.

*///===========================================================================

#ifdef ESP8266

	/*---------------------------------------------------------------------------*/
	/*                                INCLUDES                                   */
	/*---------------------------------------------------------------------------*/

	#include "NtpTimeSync.h"
	
	
	
	void NtpTimeSync::Settimeofday(long int timestamp) {
  
		time_t rtc = timestamp;
		timeval tv = { rtc, 0 };
		settimeofday(&tv, nullptr);
	}
	
	
	
	void NtpTimeSync::ServerSetup(const char* tz, const char* server1, const char* server2, const char* server3) {
		#if SNTP_USE_STORED_UNTILL_SYNC == true
			RtcLastNtpSync rtcMem;
			
			system_rtc_mem_read(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
			if (rtcMem.markerFlag == MAGICBYTE) {
				this->Settimeofday(rtcMem.lastKnownTime);
				this->_timeSet = true;
			}
		#endif
		
		// settimeofday_cb() is called when settimeofday is called (by SNTP or from your code..)
		// once enabled (by DHCP), SNTP is updated every hour
		settimeofday_cb([this]{
			this->_timeSet = true;
			
			#if SNTP_USE_STORED_UNTILL_SYNC == true
				RtcLastNtpSync rtcMem;
				
				time_t now            = time(nullptr);
				rtcMem.markerFlag     = MAGICBYTE;
				rtcMem.lastKnownTime  = now;
				
				system_rtc_mem_write(RTCMEMBEGIN, &rtcMem, sizeof(rtcMem));
			#endif
		});
	
		configTime(tz, server1, server2, server3);
	}
	
	
	bool NtpTimeSync::WaitForSync(int retries, const char *waitChar) {
		while(!this->_timeSet) {
			delay(500);
			
			if(waitChar) {
				Serial.print(waitChar);
			}
			
			retries--;
			if(retries == 0){
				return false;
			}
		}
		
		return true;
	}

/*---------------------------------------------------------------------------*/
/*                                    EOF                                    */
/*---------------------------------------------------------------------------*/
#endif // ESP8266
