#if defined ESP8266 || defined ESP32
    #ifndef configServer_h
        #define configServer_h

        #include <IOTAppStory.h>
        #ifdef ESP32
            #include <AsyncTCP.h>                       // https://github.com/me-no-dev/AsyncTCP
            #include <FS.h>                             // esp32 core SPIFFS library
            #include <SPIFFS.h>
        #elif defined  ESP8266
            #include <ESPAsyncTCP.h>                    // https://github.com/me-no-dev/ESPAsyncTCP
            #include <FS.h>                             // esp8266 core SPIFFS library
            #define FILE_WRITE  "w"
            #define FILE_APPEND "a"

        #endif
        #include <ESPAsyncWebServer.h>                  // https://github.com/me-no-dev/ESPAsyncWebServer

        struct configStruct;
        class IOTAppStory;


        class configServer {
            public:
                configServer(IOTAppStory &ias, configStruct &config);
                void run();

            private:
                IOTAppStory* _ias;
                configStruct* _config;
                std::unique_ptr<AsyncWebServer>     server;

                bool _tryToConn         = false;        // is the wifi connector busy? (trying to connect)
                bool _connFail          = false;        // did the last connection attempt faile
                bool _connChangeMode    = false;        // flag to notify the loop to change from AP to STA mode

                void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
                void hdlReturn(AsyncWebServerRequest *request, String retHtml, String type = "text/html");

        };
    #endif
#endif
