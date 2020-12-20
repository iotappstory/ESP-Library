/*                          =======================
============================   C/C++ SOURCE FILE   ============================
                            =======================                       *//**
  NtpTimeSync.cpp

  Created by Onno Dirkzwager on 20.12.2020.
  Copyright (c) 2020 IOTAppStory. All rights reserved.

*///===========================================================================

#ifdef ESP32

	/*---------------------------------------------------------------------------*/
	/*                                INCLUDES                                   */
	/*---------------------------------------------------------------------------*/

	#include "NtpTimeSync.h"
	
	
	
	void NtpTimeSync::Settimeofday(long int timestamp) {
  
		struct timeval tv;
		tv.tv_sec =   timestamp;
		settimeofday(&tv, NULL);
	}
	
	
	void NtpTimeSync::ServerSetup(const char* tz, const char* server1, const char* server2, const char* server3) {

		configTzTime(tz, server1, server2, server3);
	}
	
	
	bool NtpTimeSync::WaitForSync(int retries, const char *waitChar) {
		time_t now = time(nullptr);
		while(now < 8 * 3600 * 2) {
			delay(500);

			if(waitChar) {
				Serial.print(waitChar);
			}
			now = time(nullptr);
			
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
#endif // ESP32