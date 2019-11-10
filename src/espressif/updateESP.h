#ifndef _UPDATEESP
	#define _UPDATEESP

	#include "UpdateClassVirt.h"
	#include <StreamString.h>
	#ifdef ESP32
		#include <Update.h>
		#include <esp_partition.h>
		#include <esp_ota_ops.h>                // get running partition
	#else
		extern "C" uint32_t _SPIFFS_start;
		extern "C" uint32_t _SPIFFS_end;
	#endif

	class UpdateESPClass : public UpdateClassVirt {
		public:
			~UpdateESPClass() {};

			virtual bool prepareUpdate(uint32_t upd_size, String &upd_md5, uint16_t command);

			virtual bool update(uint8_t *file_buf, size_t buf_size);

			virtual bool end(void);

			virtual void sm(String *statusMessage);

		private:
			String *_statusMessage;
			uint32_t _upd_size;
			String *_upd_md5;
			uint16_t _command;
	};

	extern UpdateESPClass UpdateESP;
#endif