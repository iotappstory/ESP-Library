#ifndef _UPDATENEXTION
	#define _UPDATENEXTION
	
	#include "UpdateClassVirt.h"
	#include <ESPNexUpload.h>

	
	class UpdateNextionClass : public UpdateClassVirt{
		public:
			~UpdateNextionClass(){};
		
			virtual bool prepareUpdate(uint32_t upd_size, String &upd_md5, uint16_t command);
			
			virtual bool update(uint8_t *file_buf, size_t buf_size);
			
			virtual bool end(void);
			
			virtual void sm(String *statusMessage);
			
			ESPNexUpload nextion{115200};

		private:
			String *_statusMessage;
	};

	extern UpdateNextionClass UpdateNextion;
#endif