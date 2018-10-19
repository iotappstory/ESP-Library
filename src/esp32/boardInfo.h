#ifdef  ESP32
	#ifndef boardInfo_h
		#define boardInfo_h

		#include <Preferences.h>
		
		
		
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