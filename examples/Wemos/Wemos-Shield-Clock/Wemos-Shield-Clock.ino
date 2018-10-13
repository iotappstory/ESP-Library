/*
  This is an initial sketch to be used as a "blueprint" to create apps which can be used with IOTappstory.com infrastructure
  Your code can be added wherever it is marked. 

  You will need the button & OLED shields!

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


#include <SSD1306.h>                                      // OLED library by Daniel Eichhorn
#include <IOTAppStory.h>                                  // IotAppStory.com library
#include <SNTPtime.h>                                     //


SNTPtime NTPch("ch.pool.ntp.org");                        // Initialize time
IOTAppStory IAS(COMPDATE, MODEBUTTON);                    // Initialize IotAppStory
SSD1306  display(0x3c, D2, D1);                           // Initialize OLED


// ================================================ VARS =================================================
strDateTime dateTime;

String deviceName = "wemosclock";

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

char* updTimer    = "1";                                  // 0 = false, 1 = true
char* updInt      = "60";                                 // every x sec
char* timeZone    = "1.0";



// ================================================ SETUP ================================================
void setup() {
  display.init();                                         // setup OLED and show "Wait"
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(48, 35, F("Wait"));
  display.display();


  #if defined  ESP8266
    String chipId  = String(ESP.getChipId());   // creat a unique deviceName for classroom situations (deviceName-123)
    chipId         = "-"+chipId.substring(chipId.length()-3);
    deviceName    += chipId;
  #endif
  
  IAS.preSetDeviceName(deviceName);                       // preset deviceName this is also your MDNS responder: http://deviceName.local


  IAS.addField(updTimer, "Update timer:Turn on", 1, 'C'); // These fields are added to the config wifimanager and saved to eeprom. Updated values are returned to the original variable.
  IAS.addField(updInt, "Update every", 8, 'I');           // reference to org variable | field label value | max char return | Optional "special field" char
  IAS.addField(timeZone, "Timezone", 4, 'Z');
                                                          


  // You can configure callback functions that can give feedback to the app user about the current state of the application.
  // In this example we use serial print to demonstrate the call backs. But you could use leds etc.
  
  IAS.onModeButtonShortPress([]() {
    Serial.println(F(" If mode button is released, I will enter in firmware update mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    dispTemplate_threeLineV2(F("Release"), F("for"), F("Updates"));
  });

  IAS.onModeButtonLongPress([]() {
    Serial.println(F(" If mode button is released, I will enter in configuration mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    dispTemplate_threeLineV2(F("Release"), F("for"), F("Config"));
  });

  IAS.onConfigMode([]() {
    dispTemplate_threeLineV2(F("Connect to"), F("Wi-Fi"), "xxxxx-" + chipId);
  });

  IAS.onFirmwareUpdateCheck([]() {
    dispTemplate_threeLineV2(F("Checking"), F("for"), F("Updates"));
  });

  IAS.onFirmwareUpdateDownload([]() {
    dispTemplate_threeLineV2(F("Download"), F("&"), F("Install App"));
  });

  IAS.onFirmwareUpdateError([]() {
    dispTemplate_threeLineV1(F("Update"), F("Error"), F("Check logs"));
  });
  

  IAS.begin('L');                                         // Optional parameter: What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase(default) | 'L' Leave intact

  IAS.setCallHome(atoi(updTimer));                        // Set to true to enable calling home frequently (disabled by default)
  IAS.setCallHomeInterval(atoi(updInt));                  // Call home interval in seconds, use 60s only for development. Please change it to at least 2 hours in production

  
  //-------- Your Setup starts from here ---------------


  while (!NTPch.setSNTPtime()) Serial.print(".");         // set internal clock
  
  Serial.print(F(" Time zone set to: "));                 // display timeZone
  Serial.print(timeZone);
  Serial.println(F(" You can change this in config."));
  Serial.println(F("*-------------------------------------------------------------------------*\n\n"));

}



// ================================================ LOOP =================================================
void loop() {
  IAS.loop();   // this routine handles the calling home functionality and reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration


  //-------- Your Sketch starts from here ---------------
  
  if (millis() > loopEntry + 1000 && digitalRead(MODEBUTTON) == HIGH) {
    dateTime = NTPch.getTime(atof(timeZone), 1); // get time from internal clock
    drawFace();
    drawArms(dateTime.hour, dateTime.minute, dateTime.second);
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
  display.drawString(32, 30, str2);
  display.drawString(32, 45, str3);
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
void drawArms(int h, int m, int s)
{
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
