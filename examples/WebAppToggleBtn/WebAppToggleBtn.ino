/* 
  This is an initial sketch to be used as a "blueprint" to create apps which can be used with IOTAppStory.com infrastructure
  Your code can be filled wherever it is marked.

  This sketch is based on:
  VirginSoil sketch [Andreas Spiess]
  FSWebServer - Example WebServer with SPIFFS backend for esp8266 [Hristo Gochkov]
  Arduino Debounce [David A. Mellis]

  Copyright (c) [2017] [Onno Dirkzwager]

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

  Don't forget to upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload it to IOTAppStory.com

  WebAppToggleBtn V3.0.2
*/

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                                  // Button pin on the esp for selecting modes. D3 for the Wemos!


#include <IOTAppStory.h>

#if defined ESP8266
  #include <ESPAsyncTCP.h>                            // https://github.com/me-no-dev/ESPAsyncTCP
#elif defined ESP32
  #include <AsyncTCP.h>                               // https://github.com/me-no-dev/AsyncTCP
  #include <SPIFFS.h>
#endif

#include <ESPAsyncWebServer.h>                        // https://github.com/me-no-dev/ESPAsyncWebServer
#include <FS.h>


IOTAppStory IAS(COMPDATE,MODEBUTTON);                 // Initialize IotAppStory
AsyncWebServer server(80);                            // Initialize AsyncWebServer

//called when the url is not defined here return 404
void onRequest(AsyncWebServerRequest *request){
  //Handle Unknown Request
  request->send(404);
}



// ================================================ VARS =================================================
String deviceName = "webtoggle";
String chipId;

// Variables will change:
int ledState = LOW;					// the current state of the output pin
int buttonState;						// the current reading from the input pin
int lastButtonState = LOW;	// the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;	// the last time the output pin was toggled
long debounceDelay = 50;		// the debounce time; increase if the output flickers

// We want to be able to edit these example variables below from the wifi config manager
// Currently only char arrays are supported. (Keep in mind that html form fields always return Strings)
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)

char* LEDpin      = "2";                                  // The value given here is the default value and can be overwritten by values saved in configuration mode
char* btnDefPin   = "14";                                 // button pin
char* updInt      = "60";                                 // every x sec



// ================================================ SETUP ================================================
void setup() {
  
  // creat a unique deviceName for classroom situations (deviceName-123)
  chipId      = String(ESP_GETCHIPID);
  chipId      = "-"+chipId.substring(chipId.length()-3);
  deviceName += chipId;
	
	IAS.preSetDeviceName(deviceName);											  // preset deviceName this is also your MDNS responder: http://webtoggle-123.local
  IAS.preSetAutoUpdate(true);                             // automaticUpdate (true, false)


  IAS.addField(updInt, "Update every", 8, 'I');           // These fields are added to the config wifimanager and saved to eeprom. Updated values are returned to the original variable.
  IAS.addField(LEDpin, "Led Pin", 2, 'P');                // reference to org variable | field label value | max char return | Optional "special field" char
	IAS.addField(btnDefPin, "Button Pin", 2, 'P');


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
  IAS.setCallHomeInterval(atoi(updInt));                  // Call home interval in seconds(disabled by default), 0 = off, use 60s only for development. Please change it to at least 2 hours in production


  //-------- Your Setup starts from here ---------------


  if(!SPIFFS.begin()){
      Serial.println(F(" SPIFFS Mount Failed"));
      return;
  }
  
	pinMode(IAS.dPinConv(LEDpin), OUTPUT);
	pinMode(IAS.dPinConv(btnDefPin), INPUT);




	// When the button is pressed in the WebApp							<<<<<<<<<<<<--------------- <<<-------------------- <<<-----------
	server.on("/btn", HTTP_GET, [](AsyncWebServerRequest *request){
		Serial.println(F("\n WebApp button pressed"));

		// toggle ledState
		ledState = !ledState;
		Serial.println(" Changed led status to ("+String(ledState)+") on pin ("+String(LEDpin)+")\n");

		// update LEDpin
		digitalWrite(IAS.dPinConv(LEDpin), ledState);

		// create json return
		String json = "{";
		json += "\"ledState\":\""+String(ledState)+"\"";
		json += "}";

		// return json to WebApp
		request->send(200, F("text/json"), json);
		json = String();
	});

	server.on("/getState", HTTP_GET, [](AsyncWebServerRequest *request){
		// create json return
		String json = "{";
		json += "\"ledState\":\""+String(ledState)+"\"";
		json += "}";

		// return json to WebApp
		request->send(200, F("text/json"), json);
		json = String();
	});

  server.serveStatic("/", SPIFFS, "/");


  
  server.onNotFound(onRequest);
  
	// start the HTTP server
	server.begin();
	Serial.print(F(" HTTP server started at: "));
	Serial.println(WiFi.localIP());
	Serial.println("");
}



// ================================================ LOOP =================================================
void loop() {
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)


  //-------- Your Sketch starts from here ---------------


	// read the state of the pushbutton value:
	int reading = digitalRead(IAS.dPinConv(btnDefPin));


	// check to see if you just pressed the button
	// (i.e. the input went from LOW to HIGH),  and you've waited
	// long enough since the last press to ignore any noise:

	// If the switch changed, due to noise or pressing:
	if (reading != lastButtonState) {
		// reset the debouncing timer
		lastDebounceTime = millis();
	}

	if ((millis() - lastDebounceTime) > debounceDelay) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:

		// if the button state has changed:
		if (reading != buttonState) {
			buttonState = reading;

			// only toggle the LED if the new button state is HIGH
			if (buttonState == HIGH) {
				ledState = !ledState;
				Serial.println("");
				Serial.println(" Hardware button pressed. Pin ("+String(btnDefPin)+")");
				Serial.println(" Changed led status to ("+String(ledState)+") on pin ("+String(LEDpin)+")");
				Serial.println("");
			}
		}
	}

	// set the LED:
	digitalWrite(IAS.dPinConv(LEDpin), ledState);

	// save the reading.  Next time through the loop,
	// it'll be the lastButtonState:
	lastButtonState = reading;
}



// ================================================ EXTRA FUNCTIONS ================================================
