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
		settimeofday_cb([this]{
			this->_timeSet = true;
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