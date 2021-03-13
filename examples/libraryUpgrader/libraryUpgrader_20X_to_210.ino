or /*
  This sketch is an example of how you can upgrade from the previous library(2.0.X) 
  to the new 2.1.0 library EEPROM layout. If successful all the previous settings
  like Wifi, IAS device registration and added fields get "rewritten" to the new 
  EEPROM layout. 

  You can use this sketch as an intermediate update or add this code to your next 
  firmware update. It's wise to test this locally first! Make sure to include the 
  upgrade helper file in your sketch folder. And do NOT use presets!

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

  libraryUpgrader V2.1.0
*/

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                              // Button pin on the esp for selecting modes. D3 for the Wemos!


#include <IOTAppStory.h>                          // IotAppStory.com library
#include <EEPROM.h>                               // Default EEPROM library
IOTAppStory IAS(COMPDATE, MODEBUTTON);            // Initialize IOTAppStory



// ================================================ CHANGE THESE VALUES =========================================
// Study the added fields in your current app you want to upgrade:
// IAS.addField(LEDpin, "ledpin", 2, 'P');
// IAS.addField(blinkTime, "Blinktime(mS)", 5, 'N');
// reference to org variable | field label value | max char return | Field type

#define WRITECHANGES                true          // you might want to do a practice run and turn this to false
#define NROFADDEDFIELDS             8             // How many added fields should we search for?
#define LARGESTLENGTH               5             // What is de largest "max char return"

int fieldMaxLength[NROFADDEDFIELDS] = {2, 5};     // Max returns as used in the added fields.
char fieldType[NROFADDEDFIELDS]     = {'p', 'n'}; // Field types as used in the added fields. (L if none was entered!)



// ================================================ OLD struct & defines =========================================
#define OLD_STRUCT_CHAR_ARRAY_SIZE  50
#define OLD_STRUCT_PASSWORD_SIZE    64
#define OLD_STRUCT_BNAME_SIZE       30
#define OLD_STRUCT_COMPDATE_SIZE    20

typedef struct oldConfig_v20X{
  char actCode[7];                              // saved IotAppStory activation code
  char appName[33];
  char appVersion[12];
  char ssid[3][OLD_STRUCT_CHAR_ARRAY_SIZE];     // 3x SSID
  char password[3][OLD_STRUCT_PASSWORD_SIZE];   // 3x PASS
  char deviceName[OLD_STRUCT_BNAME_SIZE];
  char compDate[OLD_STRUCT_COMPDATE_SIZE];      // saved compile date time
  #if defined  ESP8266                          // the esp32 in lib 2.0.X uses a certificate stored in PROGMEM
    char sha1[60];
  #endif
  #if CFG_AUTHENTICATE == true
    char cfg_pass[17];
  #endif
  const char magicBytes[4] = MAGICBYTES;
};


// This certificate gets copied to a file in SPIFFS: /cert/iasRootCa.cer
const char ROOT_CA[] = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIEMjCCAxqgAwIBAgIBATANBgkqhkiG9w0BAQUFADB7MQswCQYDVQQGEwJHQjEb\n" \
  "MBkGA1UECAwSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHDAdTYWxmb3JkMRow\n" \
  "GAYDVQQKDBFDb21vZG8gQ0EgTGltaXRlZDEhMB8GA1UEAwwYQUFBIENlcnRpZmlj\n" \
  "YXRlIFNlcnZpY2VzMB4XDTA0MDEwMTAwMDAwMFoXDTI4MTIzMTIzNTk1OVowezEL\n" \
  "MAkGA1UEBhMCR0IxGzAZBgNVBAgMEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\n" \
  "BwwHU2FsZm9yZDEaMBgGA1UECgwRQ29tb2RvIENBIExpbWl0ZWQxITAfBgNVBAMM\n" \
  "GEFBQSBDZXJ0aWZpY2F0ZSBTZXJ2aWNlczCCASIwDQYJKoZIhvcNAQEBBQADggEP\n" \
  "ADCCAQoCggEBAL5AnfRu4ep2hxxNRUSOvkbIgwadwSr+GB+O5AL686tdUIoWMQua\n" \
  "BtDFcCLNSS1UY8y2bmhGC1Pqy0wkwLxyTurxFa70VJoSCsN6sjNg4tqJVfMiWPPe\n" \
  "3M/vg4aijJRPn2jymJBGhCfHdr/jzDUsi14HZGWCwEiwqJH5YZ92IFCokcdmtet4\n" \
  "YgNW8IoaE+oxox6gmf049vYnMlhvB/VruPsUK6+3qszWY19zjNoFmag4qMsXeDZR\n" \
  "rOme9Hg6jc8P2ULimAyrL58OAd7vn5lJ8S3frHRNG5i1R8XlKdH5kBjHYpy+g8cm\n" \
  "ez6KJcfA3Z3mNWgQIJ2P2N7Sw4ScDV7oL8kCAwEAAaOBwDCBvTAdBgNVHQ4EFgQU\n" \
  "oBEKIz6W8Qfs4q8p74Klf9AwpLQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQF\n" \
  "MAMBAf8wewYDVR0fBHQwcjA4oDagNIYyaHR0cDovL2NybC5jb21vZG9jYS5jb20v\n" \
  "QUFBQ2VydGlmaWNhdGVTZXJ2aWNlcy5jcmwwNqA0oDKGMGh0dHA6Ly9jcmwuY29t\n" \
  "b2RvLm5ldC9BQUFDZXJ0aWZpY2F0ZVNlcnZpY2VzLmNybDANBgkqhkiG9w0BAQUF\n" \
  "AAOCAQEACFb8AvCb6P+k+tZ7xkSAzk/ExfYAWMymtrwUSWgEdujm7l3sAg9g1o1Q\n" \
  "GE8mTgHj5rCl7r+8dFRBv/38ErjHT1r0iWAFf2C3BUrz9vHCv8S5dIa2LX1rzNLz\n" \
  "Rt0vxuBqw8M0Ayx9lt1awg6nCpnBBYurDC/zXDrPbDdVCYfeU0BsWO/8tqtlbgT2\n" \
  "G9w84FoVxp7Z8VlIMCFlA2zs6SFz7JsDoeA3raAVGI/6ugLOpyypEBMs1OUIJqsi\n" \
  "l2D4kF501KKaU73yqWjgom7C12yxow+ev+to51byrvLjKzg6CYG1a4XXvi3tPxq3\n" \
  "smPi9WIsgtRqAEFQ8TmDn5XpNpaYbg==\n" \
  "-----END CERTIFICATE-----\n";


// ================================================ SETUP ================================================
void setup() {
  
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

  IAS.onFirstBoot([]() {
    Serial.println("IAS.onFirstBoot()");
    runUpgrade();                           // Run the library upgrader
  });
  
  
  
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


}
