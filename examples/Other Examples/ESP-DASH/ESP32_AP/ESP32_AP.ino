/*
 * ESP-DASH V2 library demo
 * This sketch is one of the examples that comes with the ESP-DASH V2 library 
 * slightly modified for use with the IOTAppStory library.
 * 
 * This Example is a Demo of Realtime Capabilities of ESP-DASH in Access Point Mode.
 * Open Dashboard after Uploading and UI will auto update as a Card's Value changes
 * 
 * This sketch is based on:
 * VirginSoil sketch [Andreas Spiess]
 * ESP_realtime [Ayush Sharma]
 * 
 * For more info on the ESP-DASH V2 library: https://github.com/ayushsharma82/ESP-DASH
*/

#define COMPDATE __DATE__ __TIME__                          // Button pin on the esp for selecting modes. D3 for the Wemos!
#define MODEBUTTON 0

#include <WiFi.h>
#include <AsyncTCP.h>                                       // https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWebServer.h>                              // https://github.com/me-no-dev/ESPAsyncWebServer
#include <ESPDash.h>                                        // https://github.com/ayushsharma82/ESP-DASH
#include <IOTAppStory.h>                                    // IotAppStory.com library

IOTAppStory IAS(COMPDATE, MODEBUTTON);                      // Initialize IotAppStory

AsyncWebServer server(80);



// ================================================ EXAMPLE VARS =========================================
String deviceName = "ESPDash-AP";
String chipId;
// used in this example to blink (LEDpin) every (blinkTime) miliseconds
unsigned long lastUpdate;

// We want to be able to edit these example variables below from the wifi config manager
// Currently only char arrays are supported.
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)

char* ssid = "ESPDash-V2";
char* password = "admin";



// ================================================ SETUP ================================================
void setup(){  
  
  // creat a unique deviceName for classroom situations (deviceName-123)
  chipId      = String(ESP_GETCHIPID);
  chipId      = "-"+chipId.substring(chipId.length()-3);
  deviceName += chipId;
 
  IAS.preSetDeviceName(deviceName);                         // preset deviceName this is also your MDNS responder: http://ESPDash-AP-123.local
  

  IAS.addField(ssid, "SSID", 16);                           // These fields are added to the "App Settings" page in config mode and saved to eeprom. Updated values are returned to the original variable.
  IAS.addField(password, "Password", 16);                   // reference to org variable | field label value | max char return | Optional "special field" char
                                                            // Find out more about the optional "special fields" at https://iotappstory.com/wiki
    

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
  Serial.println(F(" This example runs in AP mode. So we will have to change WiFi mode and disconnect\n from your set WiFi AP. In AP mode we cannot use setCallHomeInterval to call home\n for updates. To check for updates press reset!"));
  IAS.WiFiDisconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.println(F(" Changed to AP mode. Starting server..."));
  Serial.print(F(" IP Address: "));
  Serial.println(WiFi.softAPIP());
  Serial.println(F("*-------------------------------------------------------------------------*"));

  // Initialize ESP Dash sever
  ESPDash.init(server);   // Initiate ESPDash and attach your Async webserver instance

  // Add Respective Cards
  ESPDash.addNumberCard("num1", "Number Card", 264);
  ESPDash.addTemperatureCard("temp1", "Temperature Card", 0, 20);
  ESPDash.addHumidityCard("hum1", "Humidity Card", 98);

  // Server begin
  server.begin();
}



// ================================================ LOOP =================================================
void loop() {
  //IAS.loop();        // <<-- we cannot use the IAS.loop with this ESP-DASH example

  //-------- Your Sketch starts from here ---------------
  
  // Update Card values every 3 sec
  if (millis() - lastUpdate > 3000){
    ESPDash.updateNumberCard("num1", random(0, 5000));
    ESPDash.updateTemperatureCard("temp1", random(0, 50));
    ESPDash.updateHumidityCard("hum1", random(0, 100));
    lastUpdate = millis();
  }
}
