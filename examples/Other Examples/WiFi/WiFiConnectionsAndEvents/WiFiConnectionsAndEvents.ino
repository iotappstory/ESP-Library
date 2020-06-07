/*
  The IOTAppStory-ESP library uses wifimulti internally and takes care of:
  - Managing & storing multiple credentials
  - Automaticly connecting to the strongest AP from the list on boot
  - And optionally: go to config mode if a WiFi connection can not be made

  This often brings up the question:
  How can I as a programmer take control over wifi in my sketch whithout breaking the builtin options?

  This example demonstrates how to:
  - set whether or not to go to config mode when there is no Wifi connection
  - preSet WiFi credentials (developer mode)
  - set wifi events (callbacks)
  - connect & disconnect from whithin your sketch

  Wifi defines are defined in the config.h file in the IOTAppStory-ESP library folder. And are free to modify.
  DO NOT set these in your sketch. It will not work!
  #define WIFI_SMARTCONFIG            false   // Set to true to enable smartconfig by smartphone app "ESP Smart Config" or "ESP8266 SmartConfig" | This will add (+/- 2%) of program storage space and +/- 1%) of dynamic memory
  #define WIFI_MULTI                  true    // false: only 1 ssid & pass will be used | true: 3 sets of ssid & pass will be used
  #define WIFI_MULTI_FORCE_RECONN_ANY false   // By default wifi multi will only try to reconnect to the last AP it was connectected to. Setting this to true will force your esp to connect to any of the available AP's from the list.
  #define WIFI_CONN_MAX_RETRIES       20      // sets the maximum number of retries when trying to connect to the wifi
  #define WIFI_USE_MDNS               true    // include MDNS responder http://yourboard.local
*/

/*
  This sketch is based on:
  VirginSoil sketch [Andreas Spiess]
  WiFiEvents ESP8266 [Markus Sattler & Ivan Grokhotkov]
  WiFiEvents ESP32 [me-no-dev]

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

  WiFiConnections V0.0.2
*/

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                        // Button pin on the esp for selecting modes. D3 for the Wemos!

#include <IOTAppStory.h>                    // IotAppStory.com library
IOTAppStory IAS(COMPDATE, MODEBUTTON);      // Initialize IOTAppStory



// ================================================ EXAMPLE VARS =========================================
// used in this example to switch WiFi on or off every 10 seconds
String deviceName = "wifi-connections";
String chipId;
unsigned long connEntry;                    // Last time switched
unsigned long connTime = 10000;             // Every 10 seconds
bool oddEven = true;                        // Switch between 2 options

char* LEDpin    = "2";

// ================================================ SETUP ================================================
void setup() {
  // setup the built in led for output. As this led is used in the wifi events. We need to add it before IAS.begin.
  IAS.addField(LEDpin, "ledpin", 2, 'P');                   // These fields are added to the config wifimanager and saved to eeprom. Updated values are returned to the original variable.
  pinMode(IAS.dPinConv(LEDpin), OUTPUT);
  digitalWrite(IAS.dPinConv(LEDpin), HIGH);

  // create a unique deviceName for classroom situations (deviceName-123)
  chipId      = String(ESP_GETCHIPID);
  chipId      = "-"+chipId.substring(chipId.length()-3);
  deviceName += chipId;

  IAS.preSetDeviceName(deviceName);         // preset deviceName this is also your MDNS responder: http://wifi-connections-123.local
  IAS.preSetAutoConfig(true);               // Set whether or not the device should go into config mode after after failing to connect to a Wifi AP. The default is true
  // IAS.preSetWifi("MyWifiAP", "My Pass")  // Set the WiFi credentials without going to the config pages. For development only! Make sure to delete this preSet when you publish your App.


  // Register Wifi event handlers.
  // Taken from the ESP examples for more information about events follow these links:
  // https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/WiFiEvents/WiFiEvents.ino
  // https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClientEvents/WiFiClientEvents.ino
  WiFi.onEvent(WiFiEvent);

  /*
  IAS.onFirstBoot([]() {
    IAS.eraseEEPROM('P');                   // Optional! What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase
  });
  */

  IAS.begin();                              // Run IOTAppStory
  IAS.setCallHomeInterval(60);              // Call home interval in seconds(disabled by default), 0 = off, use 60s only for development. Please change it to at least 2 hours in production


  //-------- Your Setup starts from here ---------------

}



// ================================================ LOOP =================================================
void loop() {
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)


  //-------- Your Sketch starts from here ---------------
  // Once every connTime (dis)connect
  if (millis() - connEntry > connTime){

    if(oddEven == true){
      // Disconnect WiFi
      IAS.WiFiDisconnect();
    }else{
      // Connect WiFi | If you added multiple WiFi access points in de config pages. Connect to the strongest AP from the list.
      IAS.WiFiConnect();
      Serial.println(F("*-------------------------------------------------------------------------*"));
    }

    oddEven = !oddEven;
    connEntry = millis();
  }
}



// ================================================ OTHER FUNCTIONS =================================================
#if defined  ESP8266
  void WiFiEvent(WiFiEvent_t event){
  //  Serial.printf("[WiFi-event] event: %d\n", event);

    if(event == WL_CONNECTED){
      digitalWrite(IAS.dPinConv(LEDpin), LOW);
    }else{
      digitalWrite(IAS.dPinConv(LEDpin), HIGH);
    }
  }
#elif defined ESP32
  void WiFiEvent(WiFiEvent_t event){
  //  Serial.printf("[WiFi-event] event: %d\n", event);

    if(event == SYSTEM_EVENT_STA_CONNECTED){
      digitalWrite(IAS.dPinConv(LEDpin), LOW);
    }else{
      digitalWrite(IAS.dPinConv(LEDpin), HIGH);
    }
  }
#endif
