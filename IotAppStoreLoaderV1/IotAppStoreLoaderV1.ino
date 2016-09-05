/* This sketch connects to the iopappstore and loads the assigned firmware down. The assignment is done on the server based on the MAC address of the board

    On the server, you need PHP script "iotappstore.php" and the bin files are in the .\bin folder

    This work is based on the ESPhttpUpdate examples

  Copyright (c) [2016] [Andreas Spiess]

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

  Version 1.0

*/

#include <credentials.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <EEPROM.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#define firmware "iotupdater"
#define firmware_version firmware"_001"


#define update_server "iotappstore.org"
#define update_uri "/iotappstorev10.php"

#define SSIDBASE 200
#define PASSWORDBASE 220

char ssid[20];
char password[20];

String initSSID = mySSID;
String initPassword = myPASSWORD;

bool readCredentials() {
  EEPROM.begin(512);
  if (EEPROM.read(SSIDBASE - 1) != 0x5)  {
    Serial.println(EEPROM.read(SSIDBASE - 1), HEX);
    initSSID.toCharArray(ssid, initSSID.length() + 1);
    for (int ii = 0; ii <= initSSID.length(); ii++) EEPROM.write(SSIDBASE + ii, ssid[ii]);

    initPassword.toCharArray(password, initPassword.length() + 1);
    for (int ii = 0; ii <= initPassword.length(); ii++) EEPROM.write(PASSWORDBASE + ii, password[ii]);
    EEPROM.write(SSIDBASE - 1, 0x35);
  }
  int i = 0;
  do {
    ssid[i] = EEPROM.read(SSIDBASE + i);
    i++;
  } while (ssid[i - 1] > 0 && i < 20);

  if (i == 20) Serial.println("ssid loaded");
  i = 0;
  do {
    password[i] = EEPROM.read(PASSWORDBASE + i);
    i++;
  } while (password[i - 1] != 0 && i < 20);
  if (i == 20) Serial.println("Pass loaded");
  EEPROM.end();
}

void printMacAddress() {
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  for (int i = 0; i < 5; i++) {
    Serial.print(mac[i], HEX);
    Serial.print(":");
  }
  Serial.println(mac[5], HEX);
}


void iotUpdater(bool debug) {
  if (debug) {
    printMacAddress();
    Serial.println("start flashing......");
    Serial.println(update_server);
    Serial.println(update_uri);
    Serial.println(firmware_version);
  }

  t_httpUpdate_return ret = ESPhttpUpdate.update(update_server, 80, update_uri, firmware_version);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      if (debug) Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      if (debug) Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      if (debug) Serial.println("HTTP_UPDATE_OK");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Start");
  readCredentials();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("connected");

  iotUpdater(true);
}


void loop() {
  yield();
}
