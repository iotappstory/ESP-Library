/* 
  A simple server implementation showing how to:
  * setup the ESP Async Webserver in conjunction with the IOTAppStory library
  * serve static messages
  * read GET and POST parameters
  * handle missing pages / 404s
  * reboot and go to config mode
  
  This sketch is based on:
  VirginSoil sketch [Andreas Spiess]
  simple_server - Example that comes with the ESP Async Webserver [me-no-dev]
  
  Copyright (c) [2019] [Onno Dirkzwager]
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  
  webserver V0.0.5
*/

#if defined ESP8266
  #include <ESPAsyncTCP.h>                                  // https://github.com/me-no-dev/ESPAsyncTCP
#elif defined ESP32
  #include <AsyncTCP.h>                                     // https://github.com/me-no-dev/AsyncTCP
#endif

#include <ESPAsyncWebServer.h>                              // https://github.com/me-no-dev/ESPAsyncWebServer
#include <Hash.h>
#include <IOTAppStory.h>                                    // IotAppStory.com library

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                                        // Button pin on the esp for selecting modes. D3 for the Wemos!
IOTAppStory IAS(COMPDATE, MODEBUTTON);                      // Initialize IotAppStory


AsyncWebServer server(80);



// ================================================ EXAMPLE VARS =========================================
// We want to be able to edit these example variables below from the wifi config manager
// Currently only char arrays are supported.
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)

char* hostName = "esp-async";
char* PARAM_MESSAGE = "message";



// ================================================ SETUP ================================================
void setup(){
  IAS.preSetDeviceName(hostName);                           // preset deviceName this is also your MDNS responder: http://virginsoil.local
  

  IAS.addField(hostName, "Hostname", 16);                   // These fields are added to the "App Settings" page in config mode and saved to eeprom. Updated values are returned to the original variable.
  IAS.addField(PARAM_MESSAGE, "Param message", 16);              // reference to org variable | field label value | max char return | Optional "special field" char Find out more about the optional "special fields" at https://iotappstory.com/wiki
    

  // You can configure callback functions that can give feedback to the app user about the current state of the application.
  // In this example we use serial print to demonstrate the call backs. But you could use leds etc.

  IAS.onModeButtonShortPress([]() {
    Serial.println(F(" If mode button is released, I will enter in firmware update mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onModeButtonLongPress([]() {
    Serial.println(F(" If mode button is released, I will enter in configuration mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onFirmwareUpdateProgress([](int written, int total){
      Serial.print(".");
  });
  /*
  IAS.onFirstBoot([]() {
    IAS.eraseEEPROM('P');                                 // Optional! What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase
  });
  */
  
  IAS.begin();                                            // Run IOTAppStory


  //-------- Your Setup starts from here ---------------
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "Hello, world");
  });

  // Send a GET request to <IP>/get?message=<message>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String message;
      if (request->hasParam(PARAM_MESSAGE)) {
          message = request->getParam(PARAM_MESSAGE)->value();
      } else {
          message = "No message sent";
      }
      request->send(200, "text/plain", "Hello, GET: " + message);
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
      String message;
      if (request->hasParam(PARAM_MESSAGE, true)) {
          message = request->getParam(PARAM_MESSAGE, true)->value();
      } else {
          message = "No message sent";
      }
      request->send(200, "text/plain", "Hello, POST: " + message);
  });
  
  // reboot and go to config mode
  server.on("/cfgmode", HTTP_GET, [](AsyncWebServerRequest *request){
    IAS.espRestart('C');
  });

  server.onNotFound(notFound);

  server.begin();
  Serial.printf("Open http://%s.local in your browser\n", hostName);
}



// ================================================ LOOP =================================================
void loop() {
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)


  //-------- Your Sketch starts from here ---------------
  
}



// ================================================ OTHER FUNCTIONS =================================================
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}
