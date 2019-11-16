
#ifndef _UPDATECLASS
    #define _UPDATECLASS

    #define U_FLASH   0
    #define U_SPIFFS  100
    #define U_AUTH    200
    #define U_NEXTION 300

    class UpdateClassVirt {
        public:
            virtual bool prepareUpdate(uint32_t upd_size, String &upd_md5, uint16_t command);

            virtual bool update(uint8_t *file_buf, size_t buf_size);

            virtual bool end(void);

            virtual void sm(String *statusMessage);
    };
#endif
