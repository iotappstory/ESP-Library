#ifdef  ESP8266
	#ifndef boardInfo_h
		#define boardInfo_h

		#define MAGICBYTE     						85
		#define RTCMEMBEGIN   						68
		
		extern "C" {
			#include "user_interface.h"          // used by the RTC memory read/write functions
		}
		
		
		
		/**
			------ ------ ------ ------ ------ ------ STRUCTURES ------ ------ ------ ------ ------ ------
		**/
		typedef struct {
			uint8_t markerFlag;
			int bootTimes;
			char boardMode = 'N';                // Normal operation or Configuration mode?
		} rtcMemDef __attribute__((aligned(4)));
		
		
		
		class boardInfo {
			public:
				boardInfo(int &bootTimes, char &boardMode);
				void read();
				void write();
			
			private:
				int* _bootTimes;
				char* _boardMode;
				
		};
	#endif
#endif
