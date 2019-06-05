/*
 * ESP-DASH V2 library demo
 * This sketch is one of the examples that comes with the ESP-DASH V2 library 
 * slightly modified for use with the IOTAppStory library.
 * 
 * This Example is a Demo of Realtime Capabilities of ESP-DASH with a Line Chart.
 * Open Dashboard after Uploading and UI will auto update as the Chart Value Changes
 * 
 * This sketch is based on:
 * VirginSoil sketch [Andreas Spiess]
 * ESP_realtime [Ayush Sharma]
 * 
 * For more info on the ESP-DASH V2 library: https://github.com/ayushsharma82/ESP-DASH
*/

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                                        // Button pin on the esp for selecting modes. D3 for the Wemos!

#include <WiFi.h>
#include <AsyncTCP.h>                                       // https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWebServer.h>                              // https://github.com/me-no-dev/ESPAsyncWebServer
#include <ESPDash.h>                                        // https://github.com/ayushsharma82/ESP-DASH
#include <IOTAppStory.h>                                    // IotAppStory.com library

IOTAppStory IAS(COMPDATE, MODEBUTTON);                      // Initialize IotAppStory

AsyncWebServer server(80);



// ================================================ EXAMPLE VARS =========================================
String deviceName = "ESPDash-realtime";
String chipId;

// used in this example to blink (LEDpin) every (blinkTime) miliseconds
unsigned long lastUpdate;

// Line Chart Data
int x_axis_size = 7;
String x_axis[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"}; 
int y_axis_size = 7;
int y_axis[7] = {2, 5, 10, 12, 18, 8, 5};



// ================================================ SETUP ================================================
void setup(){
  
  // creat a unique deviceName for classroom situations (deviceName-123)
  chipId      = String(ESP_GETCHIPID);
  chipId      = "-"+chipId.substring(chipId.length()-3);
  deviceName += chipId;
 
  IAS.preSetDeviceName(deviceName);                         // preset deviceName this is also your MDNS responder: http://ESPDash-realtime-123.local
  

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
    IAS.eraseEEPROM('P');                                   // Optional! What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase
  });
  */
  
  IAS.begin();                                              // Run IOTAppStory


  //-------- Your Setup starts from here ---------------

  
  delay(500);
  Serial.println(F(" Start server..."));
  Serial.print(F(" IP Address: "));
  Serial.println(WiFi.localIP());

  // Initialize ESP Dash sever
  ESPDash.init(server);   // Initiate ESPDash and attach your Async webserver instance

  // Add Respective Cards
  ESPDash.addLineChart("chart1", "Example Line Chart", x_axis, x_axis_size, "Power KwH", y_axis, y_axis_size);

  // Server begin
  server.begin();
}



// ================================================ LOOP =================================================
void loop() {
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)


  //-------- Your Sketch starts from here ---------------
  
  // Update Card values every 3 sec
  if (millis() - lastUpdate > 3000){

    // Fill Data with random Values
    for(int i=0; i < 7; i++){
        y_axis[i] = random(2, 20);
    }
    ESPDash.updateLineChart("chart1", x_axis, x_axis_size, y_axis, y_axis_size);
    lastUpdate = millis();
  }
}
