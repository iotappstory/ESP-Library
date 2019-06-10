/*
  Warning: This is not for first time users. Use the IASLoader instead!

  This sketch is meant for adding large quantities of devices to an existing project(product)
  without having to do/repeat the initial setup.(Wifi, registration etc.)

  This sketch:
  - Erases EEPROM
  - Sets up initial hardcode WiFi connection
  - Contacts IOTAppStory.com over https
  - Registers your device with IOTAppStory.com
  - Adds the device to a project(product)
  - Saves the device activation code
  - Saves generated device name (eg. XXXX-22, XXXX-23 ...)
  - Saves the Fingerprint or Certificate (depends on config.h settings)
  - Saves the WiFi credentials (optional)
  - Calls home to get the latest firmware
  
  You will need:
  - IOTAppStory.com account
  - Existing project(product) with at least 1 added device of the same 
    type as the devices you want to use this sketch for.
  - Project hash
  - Local WiFi credentials

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
*/

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                      // Button pin on the esp for selecting modes. D3 for the Wemos!


#include <IOTAppStory.h>                  // IOTAppStory.com library
IOTAppStory IAS(COMPDATE, MODEBUTTON);    // Initialize IOTAppStory



// ================================================ VARS =================================================
const char* ssid          = "YourSSID";   // Wifi SSID
const char* password      = "password";   // WiFi password
const bool  saveWifiCred  = true;         // true : Save the Wifi credentials for future use
                                          // false: only use Wifi credentials for adding this device and doing the initial update

const char* host          = "iotappstory.com";
const char* url           = "/ota/addtoproject.php";
const char* hash          = "EXAMPLE1234567890abcdefghij1234EXAMPLEghij1234567890abcdefghij12";

#if defined  ESP8266 && HTTPS_8266_TYPE == FNGPRINT
  // Use web browser to view and copy SHA1 fingerprint of the certificate
  const char fingerprint[] PROGMEM = "34 6d 0a 26 f0 40 3a 0a 1b f1 ca 8e c8 0c f5 14 21 83 7c b1";
#else
  // Use web browser to save a copy of the root certificate
  const char ROOT_CA[] = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIF2DCCA8CgAwIBAgIQTKr5yttjb+Af907YWwOGnTANBgkqhkiG9w0BAQwFADCB\n" \
    "hTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4G\n" \
    "A1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNV\n" \
    "BAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAwMTE5\n" \
    "MDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBhTELMAkGA1UEBhMCR0IxGzAZBgNVBAgT\n" \
    "EkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMR\n" \
    "Q09NT0RPIENBIExpbWl0ZWQxKzApBgNVBAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNh\n" \
    "dGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCR\n" \
    "6FSS0gpWsawNJN3Fz0RndJkrN6N9I3AAcbxT38T6KhKPS38QVr2fcHK3YX/JSw8X\n" \
    "pz3jsARh7v8Rl8f0hj4K+j5c+ZPmNHrZFGvnnLOFoIJ6dq9xkNfs/Q36nGz637CC\n" \
    "9BR++b7Epi9Pf5l/tfxnQ3K9DADWietrLNPtj5gcFKt+5eNu/Nio5JIk2kNrYrhV\n" \
    "/erBvGy2i/MOjZrkm2xpmfh4SDBF1a3hDTxFYPwyllEnvGfDyi62a+pGx8cgoLEf\n" \
    "Zd5ICLqkTqnyg0Y3hOvozIFIQ2dOciqbXL1MGyiKXCJ7tKuY2e7gUYPDCUZObT6Z\n" \
    "+pUX2nwzV0E8jVHtC7ZcryxjGt9XyD+86V3Em69FmeKjWiS0uqlWPc9vqv9JWL7w\n" \
    "qP/0uK3pN/u6uPQLOvnoQ0IeidiEyxPx2bvhiWC4jChWrBQdnArncevPDt09qZah\n" \
    "SL0896+1DSJMwBGB7FY79tOi4lu3sgQiUpWAk2nojkxl8ZEDLXB0AuqLZxUpaVIC\n" \
    "u9ffUGpVRr+goyhhf3DQw6KqLCGqR84onAZFdr+CGCe01a60y1Dma/RMhnEw6abf\n" \
    "Fobg2P9A3fvQQoh/ozM6LlweQRGBY84YcWsr7KaKtzFcOmpH4MN5WdYgGq/yapiq\n" \
    "crxXStJLnbsQ/LBMQeXtHT1eKJ2czL+zUdqnR+WEUwIDAQABo0IwQDAdBgNVHQ4E\n" \
    "FgQUu69+Aj36pvE8hI6t7jiY7NkyMtQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB\n" \
    "/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAArx1UaEt65Ru2yyTUEUAJNMnMvl\n" \
    "wFTPoCWOAvn9sKIN9SCYPBMtrFaisNZ+EZLpLrqeLppysb0ZRGxhNaKatBYSaVqM\n" \
    "4dc+pBroLwP0rmEdEBsqpIt6xf4FpuHA1sj+nq6PK7o9mfjYcwlYRm6mnPTXJ9OV\n" \
    "2jeDchzTc+CiR5kDOF3VSXkAKRzH7JsgHAckaVd4sjn8OoSgtZx8jb8uk2Intzna\n" \
    "FxiuvTwJaP+EmzzV1gsD41eeFPfR60/IvYcjt7ZJQ3mFXLrrkguhxuhoqEwWsRqZ\n" \
    "CuhTLJK7oQkYdQxlqHvLI7cawiiFwxv/0Cti76R7CZGYZ4wUAc1oBmpjIXUDgIiK\n" \
    "boHGhfKppC3n9KUkEEeDys30jXlYsQab5xoq2Z0B15R97QNKyvDb6KkBPvVWmcke\n" \
    "jkk9u+UJueBPSZI9FoJAzMxZxuY67RIuaTxslbH9qh17f4a+Hg4yRvv7E491f0yL\n" \
    "S0Zj/gA0QHDBw7mh3aZw4gSzQbzpgJHqZJx64SIDqZxubw5lT2yHh17zbqD5daWb\n" \
    "QOhTsiedSrnAdyGN/4fy3ryM7xfft0kL0fJuMAsaDk527RH89elWsn2/x20Kk4yl\n" \
    "0MC2Hb46TpSi125sC8KKfPog88Tk5c0NqMuRkrF8hey1FGlmDoLnzc7ILaZRfyHB\n" \
    "NVOFBkpdn627G190\n" \
    "-----END CERTIFICATE-----\n";
#endif



// ================================================ SETUP ================================================
void setup() {
  Serial.println(FPSTR(SER_DEV));       // print divider
  IAS.eraseEEPROM('F');                 // Optional! What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase
                                        // This should not be necessary on new devices, but may be for existing (test) devices that have used EEPROM
  
  // setup Wifi connection
  Serial.printf("\n Connecting to %s\n ", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print(F("\n WiFi connected\n IP address: "));
  Serial.println(WiFi.localIP());
  
  {
    // Synchronize the internal clock useing SNTP | used for verifying certificates on the ESP8266
    #if defined  ESP8266 && HTTPS_8266_TYPE == CERTIFICATE
      IAS.setClock();
    #endif
    
    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client;
    Serial.printf("\n Connecting to %s\n", host);
  
    #if defined  ESP8266
      #if HTTPS_8266_TYPE == FNGPRINT
        Serial.printf(" Using fingerprint '%s'\n", fingerprint);
        client.setFingerprint(fingerprint);
      #else
        client.setCACert((uint8_t*)ROOT_CA, strlen(ROOT_CA));
      #endif
    #elif defined ESP32
      client.setCACert(ROOT_CA);
    #endif
    
    if (!client.connect(host, 443)) {
      Serial.println(F(" Connection failed"));
      return;
    }
  
    // request url from host
    Serial.printf(" Requesting URL: %s%s\n", host, url);
    client.print(String("GET ") + url + "?hash=" + hash + F(" HTTP/1.1") +
                 F("\r\nHost: ") + host + 
                 F("\r\nUser-Agent: ESP-http-Update") +
                 F("\r\nx-ESP-STA-MAC: ") + WiFi.macAddress() +
                 F("\r\nx-ESP-LOCIP: ") + WiFi.localIP().toString() +
                 F("\r\nx-ESP-SKETCH-MD5: ") + ESP.getSketchMD5() +
                 F("\r\nx-ESP-FLASHCHIP-ID: ") + ESP_GETFLASHCHIPID +
                 F("\r\nx-ESP-CHIP-ID: ") + ESP_GETCHIPID +
                 F("\r\nx-ESP-FLASHCHIP-SIZE: ") + ESP.getFlashChipSize() +
                 F("\r\nConnection: close\r\n\r\n"));
  
    // process server respons header & return error if not 200 - OK
    int code;
    String actcode, deviceName;
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      
      if(line.startsWith(F("HTTP/1.1 "))) {
          line.remove(0, 9);
          code = line.substring(0, 3).toInt();
  
          if(code != 200){
            line.remove(0, 4);
            Serial.print(F(" Request error!\n Code\t\t: "));
            Serial.print(code);
            Serial.print(F("\n Message\t: "));
            Serial.println(line);
            break;
          }
      }else if(line.startsWith(F("actcode: "))) {
          line.remove(0, 9);
          actcode = line;
      }else if(line.startsWith(F("devname: "))) {
          line.remove(0, 9);
          deviceName = line;
      }else if (line == "\r") {
          Serial.println(F("\n Headers received"));
          break;
      }
    }
  
    // Close connection
    client.stop();
    Serial.println(F(" Closing connection"));
  
  
    // If server respons code is OK and we received the activation code
    if(code == 200 && actcode != ""){
      Serial.println(F("\n Success! Added device to IAS.\n Updating device..."));
  
      // create new config struc
      configStruct newConfig;

      // read config | this gets the default settings
      IAS.readConfig(newConfig);
      {
        // copy activation code to newConfig struct
        actcode.toCharArray(newConfig.actCode, 7);
        Serial.println(F(" - Added activation code."));
        
        // copy deviceName to newConfig struct
        deviceName.toCharArray(newConfig.deviceName, STRUCT_BNAME_SIZE);
        Serial.print(" - Added device name: ");
        Serial.println(deviceName);

        // copy fingerprint to newConfig struct OR certificate to SPIFFS
        #if defined  ESP8266 && HTTPS_8266_TYPE == FNGPRINT
          strcpy(newConfig.sha1, fingerprint);
          Serial.println(F(" - Added fingerprint"));
        #endif
        
        // write config to EEPROM
        IAS.writeConfig(newConfig);
        delay(100);

        // if set write certificate to SPIFFS for future use
        #if defined  ESP32 || HTTPS_8266_TYPE == CERTIFICATE
          
          // Mount SPIFFS
          if(!ESP_SPIFFSBEGIN){
            Serial.println(F("- Failed to mount SPIFFS"));
          }

          // open new SPIFFS file for writing data
          File fsUploadFile;
          fsUploadFile = SPIFFS.open("/cert/iasRootCa.cer", "w"); /// close file

          // write certificate(hardcoded char array) to SPIFFS file
          if(fsUploadFile.write((uint8_t *)ROOT_CA, strlen(ROOT_CA)) != strlen(ROOT_CA)){
            Serial.println(F(" - Failed to write certificate to SPIFFS"));
          }
    
          // close SPIFFS FILE
          fsUploadFile.close();
    
          Serial.println(F(" - Added Certificate to SPIFFS"));
        #endif
      }
      
      // write WiFi credentials to EEPROM
      if(saveWifiCred){
        WiFiConnector WiFiConn;
        WiFiConn.addAPtoEEPROM(ssid, password, 1);
        Serial.println(F(" - Added WiFi credentials"));
      }
  
      // print divider
      Serial.println(FPSTR(SER_DEV));
    }else{
      return;
    }
  }
  
  // callback for showing update progress
  IAS.onFirmwareUpdateProgress([](int written, int total){
      Serial.print(".");
  });

  // callhome and get the latest firmware
  IAS.callHome();

  // if callhome failed retry every 60 sec
  IAS.setCallHomeInterval(60);
}



// ================================================ LOOP =================================================
void loop() {
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)
}
