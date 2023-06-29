/*
  This is an initial sketch to be used as a "blueprint" to create apps which can be used with IOTappstory.com infrastructure
  Your code can be added wherever it is marked.

  You will need either the button & OLED shields! or the LOLIN oled v2.1.0 shield!

  Copyright (c) [2018] [Andreas Spiess]

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
#define MODEBUTTON D3                                     // Button pin on the esp for selecting modes. 0 for Generic devices!


#include <IOTAppStory.h>                                  // IotAppStory.com library
#include <ezTime.h>                                       // https://github.com/ropg/ezTime
#include <SSD1306.h>                                      // OLED library by Daniel Eichhorn
#include <LOLIN_I2C_BUTTON.h>                             // Wemos button library for wemos lolin oled shield v2.1.0 https://github.com/wemos/LOLIN_OLED_I2C_Button_Library
I2C_BUTTON button(0x31);                                  // I2C address 0x31


IOTAppStory IAS(COMPDATE, MODEBUTTON);                    // Initialize IotAppStory
Timezone myTZ;                                            // Initialize ezTime
SSD1306  display(0x3c, D2, D1);                           // Initialize OLED



// ================================================ VARS =================================================
String deviceName = "wemosclock";
String chipId;

int screenW       = 64;
int screenH       = 48;
int clockCenterX  = screenW / 2;
int clockCenterY  = ((screenH - 16) / 2) + 16;            // top yellow part is 16 px height
int clockRadius   = 23;
int x             = 30;
int y             = 10;

unsigned long loopEntry;

// We want to be able to edit these example variables below from the wifi config manager
// Currently only char arrays are supported.
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)

char* updInt      = "60";                                 // every x sec
char* timeZone    = "Europe/Amsterdam";



// ================================================ SETUP ================================================
void setup() {
  if(IAS.boardMode == 'N'){                             // setup OLED and show "Loading" Only in normal mode! Preserve heap for config mode.
    display.init();
    display.flipScreenVertically();
    bootScreen(46, 14, F("Loading"),F("/wbmp/logo.WBMP"));
    display.display();
  }

  // creat a unique deviceName for classroom situations (deviceName-123)
  chipId      = String(ESP_GETCHIPID);
  chipId      = "-"+chipId.substring(chipId.length()-3);
  deviceName += chipId;

  IAS.preSetDeviceName(deviceName);                       // preset deviceName this is also your MDNS responder: http://deviceName.local
  IAS.addField(updInt, "Update every", 8, 'I');           // These fields are added to the config wifimanager and saved to eeprom. Updated values are returned to the original variable.
  IAS.addField(timeZone, "Timezone", 48, 'Z');            // reference to org variable | field label value | max char return | Optional "special field" char

  // You can configure callback functions that can give feedback to the app user about the current state of the application.
  // In this example we use serial print to demonstrate the call backs. But you could use leds etc.

                                  //  default mode button not connected on LOLIN oled v2.1.0 shield
  IAS.onModeButtonShortPress([]() {
    Serial.println(F(" If mode button is released, I will enter in firmware update mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    dispTemplate_threeLineV2(F("Release"), F("for"), F("Updates"));
  });
                                  //  default mode button not connected on LOLIN oled v2.1.0 shield
  IAS.onModeButtonLongPress([]() {
    Serial.println(F(" If mode button is released, I will enter in configuration mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    dispTemplate_threeLineV2(F("Release"), F("for"), F("Config"));
  });

  IAS.onConfigMode([]() {
    //  if wifi clientmode display ip addr
    if(WiFi.isConnected()){
      dispTemplate_threeLineV1(F("Connect to"), String(WiFi.SSID()) , WiFi.localIP().toString() );
      }else{
      dispTemplate_threeLineV2(F("Connect to"), F("Wi-Fi"), "xxxxx-" + chipId);
      }
  });

  IAS.onFirmwareUpdateCheck([]() {
    dispTemplate_threeLineV2(F("Checking"), F("for"), F("Updates"));
  });

  IAS.onFirmwareUpdateDownload([]() {
    dispTemplate_threeLineV2(F("Download"), F("&"), F("Install App"));
    delay(1000);
  });

  IAS.onFirmwareUpdateProgress([](int written, int total){
    String perc = String(written / (total / 100)) + "%";
    dispTemplate_progressBarV1(F("Installing"), perc, written, total );
    Serial.print(".");
  });

  IAS.onFirmwareUpdateError([](String statusMessage) {
    dispTemplate_threeLineV1(F("Update"), F("Error"), F("Check logs"));
  });
  /*
  IAS.onFirstBoot([]() {
    IAS.eraseEEPROM('P');                   // Optional! What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase
  });
  */

  IAS.begin();                                            // Run IOTAppStory
  IAS.setCallHomeInterval(atoi(updInt));                  // Call home interval in seconds(disabled by default), 0 = off, use 60s only for development. Please change it to at least 2 hours in production


  //-------- Your Setup starts from here ---------------


  delay(500);
  Serial.print(F(" Time zone set to: "));                 // display timeZone
  Serial.print(timeZone);
  Serial.println(F(" You can change this in config."));
  Serial.println(F("*-------------------------------------------------------------------------*\n\n"));

  //setDebug(INFO);
  waitForSync();
  myTZ.setLocation(timeZone);                             // Set timezone
}



// ================================================ LOOP =================================================
void loop() {
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)


  //-------- Your Sketch starts from here ---------------
  buttonLoop(); // check wemos lolin oled shield v2.1.0 buttons for press event for wemos oled display shield

  if (millis() > loopEntry + 1000 && digitalRead(MODEBUTTON) == HIGH) {
    drawFace();
    drawArms(myTZ.dateTime("H").toInt(), myTZ.dateTime("i").toInt(), myTZ.dateTime("s").toInt());
    display.display();

    loopEntry = millis();
  }
}



// ================================================ Extra functions ======================================
void dispTemplate_threeLineV1(String str1, String str2, String str3) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, str1);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 32, str2);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(96, 51, str3);
  display.display();
}

void dispTemplate_threeLineV2(String str1, String str2, String str3) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 13, str1);
  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 24, str2);
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(96, 51, str3);
  display.display();
}

// Draw an analog clock face
void drawFace() {
  display.clear();
  display.drawCircle(clockCenterX + x, clockCenterY + y, 2);
  //
  //hour ticks
  for ( int z = 0; z < 360; z = z + 30 ) {
    //Begin at 0° and stop at 360°
    float angle = z ;
    angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
    int x2 = ( clockCenterX + ( sin(angle) * clockRadius ) );
    int y2 = ( clockCenterY - ( cos(angle) * clockRadius ) );
    int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    display.drawLine( x2 + x , y2 + y , x3 + x , y3 + y);
  }
}

// Draw the clock's three arms: seconds, minutes, hours.
void drawArms(int h, int m, int s) {
  // display second hand
  float angle = s * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  display.drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display minute hand
  angle = m * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  display.drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display hour hand
  angle = h * 30 + int( ( m / 12 ) * 6 )   ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  display.drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
}


void dispTemplate_progressBarV1(String str1, String str2, int written , int total) {
  int progress = (written / (total / 100));
  display.clear();

  if(progress < 100){
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(32, 13, str1);
    display.setFont(ArialMT_Plain_24);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 24, str2);
    display.drawProgressBar(32, 56, 63, 6, progress);

  }else if(progress == 100){
    bootScreen(46,14,F("rebooting"),F("/wbmp/logo.WBMP"));
  }
  display.display();
}
   // 0 None
   // 1 Short Press
   // 2 Long Press
   // 3 Double Press
   // 4 Hold
   // check wemos lolin oled shield v2.1.0 buttons for press event for wemos oled display shield
void buttonLoop() {
  if (button.get() == 0) {
                                                  //  if button A has been pressed once
    if (button.BUTTON_A == 1) {
      dispTemplate_threeLineV1(F("Press twice"), F("for"), F("call home"));
    }
    //  if button A has been double pressed
    if (button.BUTTON_A == 3) {
      IAS.callHome();                             //  check the IAS server for updates
    }
                                                  //  if button B has been pressed once
    if (button.BUTTON_B == 1) {
      dispTemplate_threeLineV1(F("Press twice"), F("for"), F("config mode"));
    }
                                                  //  if button B has been double pressed
    if (button.BUTTON_B == 3) {
      IAS.espRestart('C');                        //  restart in config mode
    }
  }
}


void drawWbmp(uint8_t xMove, uint8_t yMove, Stream &file) {
  uint8_t data = file.read();               //  first byte 0 = wbmp
          data = file.read();               //  second byte 0 = monochrome
  uint8_t width = file.read();              //  3rd byte = img width max 255 px
  uint8_t height = file.read();             //  4th byte = img height max 255 px

  for(uint8_t y = 0; y < height; y++) {
    for(uint8_t x = 0; x < width; x++ ) {
      if (x & 7) {
        data <<= 1; // Move a bit
      } else {  // Read new data every 8 bit
        data = file.read();
      }
      // if there is a bit draw it
      if (data & 0x80) {
        display.setPixel(xMove + x, yMove + y);
      }
    }
  }
}

void bootScreen(int8_t xMove, int8_t yMove,String str1, const String fileName){
  if(!SPIFFS.begin()){
     Serial.println(F("\n\n SPIFFS Mount Failed"));
     return;
  }
  File file = SPIFFS.open(fileName,"r");
  if (!file) {
     Serial.println(F("\n\n Failed to open file"));
     return;
  }

  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 48, str1);
  drawWbmp(xMove, yMove, file);
  display.display();

  file.close();
  SPIFFS.end();
}
