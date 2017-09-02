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
*/

#define APPNAME "WebAppToggleBtn"
#define VERSION "V2.0.0"
#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0


#include <IOTAppStory.h>
IOTAppStory IAS(APPNAME,VERSION,COMPDATE,MODEBUTTON);

#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
ESP8266WebServer server(80);

// ================================================ EXAMPLE VARS =========================================
// Variables will change:
int ledState = LOW;					// the current state of the output pin
int buttonState;						// the current reading from the input pin
int lastButtonState = LOW;	// the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;	// the last time the output pin was toggled
long debounceDelay = 50;		// the debounce time; increase if the output flickers

// We want to be able to edit these example variables from the wifi config manager
// Currently only char arrays are supported. (Keep in mind that html form fields always return Strings)
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)
char* LEDpin = "12";
char* btnDefPin = "14";



// ================================================ EXTRA FUNCTIONS ================================================
//format bytes
String formatBytes(size_t bytes){
	if (bytes < 1024){
		return String(bytes)+"B";
	} else if(bytes < (1024 * 1024)){
		return String(bytes/1024.0)+"KB";
	} else if(bytes < (1024 * 1024 * 1024)){
		return String(bytes/1024.0/1024.0)+"MB";
	} else {
		return String(bytes/1024.0/1024.0/1024.0)+"GB";
	}
}

String getContentType(String filename){
	if(server.hasArg("download")) return "application/octet-stream";
	else if(filename.endsWith(".htm")) return "text/html";
	else if(filename.endsWith(".html")) return "text/html";
	else if(filename.endsWith(".css")) return "text/css";
	else if(filename.endsWith(".js")) return "application/javascript";
	else if(filename.endsWith(".png")) return "image/png";
	else if(filename.endsWith(".gif")) return "image/gif";
	else if(filename.endsWith(".jpg")) return "image/jpeg";
	else if(filename.endsWith(".ico")) return "image/x-icon";
	else if(filename.endsWith(".xml")) return "text/xml";
	else if(filename.endsWith(".pdf")) return "application/x-pdf";
	else if(filename.endsWith(".zip")) return "application/x-zip";
	else if(filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}

bool handleFileRead(String path){
	Serial.println("handleFileRead: " + path);
	if(path.endsWith("/")) path += "index.htm";
	String contentType = getContentType(path);
	String pathWithGz = path + ".gz";
	if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
		if(SPIFFS.exists(pathWithGz)) path += ".gz";
		File file = SPIFFS.open(path, "r");
		size_t sent = server.streamFile(file, contentType);
		file.close();
		return true;
	}
	return false;
}



// ================================================ SETUP ================================================
void setup() {
	IAS.serialdebug(true);																// 1st parameter: true or false for serial debugging. Default: false | When set to true or false serialdebug can be set from wifi config manager
	//IAS.serialdebug(true,115200);												// 1st parameter: true or false for serial debugging. Default: false | 2nd parameter: serial speed. Default: 115200
	/* TIP! delete the above lines when not used */

	IAS.preSetConfig("webtoggle");												// preset Boardname

	IAS.addField(LEDpin, "ledpin", "Led Pin", 2);					// These fields are added to the config wifimanager and saved to eeprom. Updated values are returned to the original variable.
	IAS.addField(btnDefPin, "btnpin", "Button Pin", 2);		// reference to org variable | field name | field label value | max char return

	IAS.begin(true);                                      // 1st parameter: true or false to view BOOT STATISTICS | 2nd parameter: true or false to erase eeprom on first boot of the app


	//-------- Sketch Specific starts from here ---------------
	pinMode(IAS.dPinConv(LEDpin), OUTPUT);
	pinMode(IAS.dPinConv(btnDefPin), INPUT);


	//called when the url is not defined here return 404
	server.onNotFound([](){
		if(!handleFileRead(server.uri())){
			server.send(404, "text/plain", "FileNotFound");
		}
	});

	// When the button is pressed in the WebApp							<<<<<<<<<<<<--------------- <<<-------------------- <<<-----------
	server.on("/btn", HTTP_GET, [](){
		Serial.println("");
		Serial.println("WebApp button pressed");

		// toggle ledState
		ledState = !ledState;
		Serial.println("Changed led status to ("+String(ledState)+") on pin ("+String(LEDpin)+")");
		Serial.println("");

		// update LEDpin
		digitalWrite(IAS.dPinConv(LEDpin), ledState);

		// create json return
		String json = "{";
		json += "\"ledState\":\""+String(ledState)+"\"";
		json += "}";

		// return json to WebApp
		server.send(200, "text/json", json);
		json = String();
	});

	server.on("/getState", HTTP_GET, [](){
		// create json return
		String json = "{";
		json += "\"ledState\":\""+String(ledState)+"\"";
		json += "}";

		// return json to WebApp
		server.send(200, "text/json", json);
		json = String();
	});

	// start the HTTP server
	server.begin();
	Serial.print("HTTP server started at: ");
	Serial.println(WiFi.localIP());
	Serial.println("");
	SPIFFS.begin();
	{
		Dir dir = SPIFFS.openDir("/");
		while (dir.next()) {    
			String fileName = dir.fileName();
			size_t fileSize = dir.fileSize();
			Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
		}
		Serial.println("");
	}
}



// ================================================ LOOP =================================================
void loop() {
	// this routine handles the reaction of the Flash button. If short press: update of skethc, long press: Configuration
	IAS.buttonLoop();


	//-------- Sketch Specific starts from here ---------------
	server.handleClient();

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
				Serial.println("Hardware button pressed. Pin ("+String(btnDefPin)+")");
				Serial.println("Changed led status to ("+String(ledState)+") on pin ("+String(LEDpin)+")");
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
