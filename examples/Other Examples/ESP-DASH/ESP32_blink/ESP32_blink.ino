/*
 * ESP-DASH V2 library demo
 * This sketch is one of the examples that comes with the ESP-DASH V2 library 
 * slightly modified for use with the IOTAppStory library.
 * 
 * This Example is a Demo of Button Capability in ESP-DASH.
 * Open Dashboard after Uploading and Click Button to Blink On-Board LED at GPIO 2
 * 
 * This sketch is based on:
 * VirginSoil sketch [Andreas Spiess]
 * ESP32_blink [Ayush Sharma]
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

void buttonClicked(const char* id);                         // forward declaration of buttonClicked



// ================================================ EXAMPLE VARS =========================================
String deviceName = "ESPDash-blink";
String chipId;

// We want to be able to edit these example variables below from the wifi config manager
// Currently only char arrays are supported. (Keep in mind that html form fields always return Strings)
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)

char* LEDpin    = "2";                                     // The value given here is the default value and can be overwritten by values saved in configuration mode



// ================================================ SETUP ================================================
void setup(){  
  
  // creat a unique deviceName for classroom situations (deviceName-123)
  chipId      = String(ESP_GETCHIPID);
  chipId      = "-"+chipId.substring(chipId.length()-3);
  deviceName += chipId;
 
  IAS.preSetDeviceName(deviceName);                         // preset deviceName this is also your MDNS responder: http://ESPDash-blink-123.local


  IAS.addField(LEDpin, "ledpin", 2, 'P');                   // These fields are added to the config wifimanager and saved to eeprom. Updated values are returned to the original variable.
                                                            // reference to org variable | field label value | max char return
  
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
  pinMode(IAS.dPinConv(LEDpin), OUTPUT);
  Serial.println(F(" Start server..."));
  Serial.print(F(" IP Address: "));
  Serial.println(WiFi.localIP());

  // Initialize ESP Dash sever
  ESPDash.init(server);   // Initiate ESPDash and attach your Async webserver instance

  // Add Respective Cards and attach Button Click Function
  ESPDash.addButtonCard("btn1", "Blink Button");
  ESPDash.attachButtonClick(buttonClicked);

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

}



// ================================================ OTHER FUNCTIONS =================================================
void buttonClicked(const char* id){
  Serial.println("Button Clicked - " + String(id));
  digitalWrite(IAS.dPinConv(LEDpin), HIGH);
  delay(100);
  digitalWrite(IAS.dPinConv(LEDpin), LOW);
}
