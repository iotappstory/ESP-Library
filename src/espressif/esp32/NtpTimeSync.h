/*                          =======================
============================   C/C++ HEADER FILE   ============================
                            =======================                       *//**
  NtpTimeSync.h

  Created by Onno Dirkzwager on 20.12.2020.
  Copyright (c) 2020 IOTAppStory. All rights reserved.

*///===========================================================================
#ifdef ESP32
	#ifndef __NtpTimeSync_h__
		#define __NtpTimeSync_h__

			/*---------------------------------------------------------------------------*/
			/*                                    INCLUDES                               */
			/*---------------------------------------------------------------------------*/
			#include <Arduino.h> 		// configTime()
			#include <sys/time.h>
			
			/*                          =======================
			============================   CLASS DEFINITION    ============================
										=======================
			*/
			class NtpTimeSync {
				public:
					void Settimeofday(long int timestamp);
					void ServerSetup(const char* tz, const char* server1, const char* server2 = nullptr, const char* server3 = nullptr);
					bool WaitForSync(int retries, const char* waitChar = nullptr);
			};
			
			
			/*---------------------------------------------------------------------------*/
			/*                                    EOF                                    */
			/*---------------------------------------------------------------------------*/
		#endif // __NtpTimeSync_h__
#endif // ESP32