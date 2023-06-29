
/*
  This sketch is made to show an example of using the popular pubsub mqtt client library
  made by Nick O'Leary : https://github.com/knolleary/pubsubclient

  Copyright (c) [2023] [Christiaan Broeders]

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

  IAS-mqtt-pubsub example V1.0.1
*/

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                    // Button pin on the esp for selecting modes. D3 for the Wemos!


#include <IOTAppStory.h>                // IotAppStory.com library
IOTAppStory IAS(COMPDATE, MODEBUTTON);  // Initialize IotAppStory

#include <PubSubClient.h>               //  include the pubsub library  https://github.com/knolleary/pubsubclient

WiFiClient wifi;                        //  The client we will be using. 
                                        //  No need to tell it the wifi ssid or credentials
                                        //  as IAS is already handling this in the background through the WiFi manager

PubSubClient PubSub(wifi);              //  Initialize PubSub and telling it what client is used

//  Declaring some variables, we will use IAS.addField() so we can change them later through our devices config menu 
char* testMsg = "Hello from the IOTAppStory PubSub demo!";  //  demo text, this is the payload that will be sent
char* inTopic = "test/iotappstory";                         //  the topic we will subscribe to
char* outTopic = "test/iotappstory";                        //  the topic we will publish to
char* broker = "test.mosquitto.org";                        //  the broker server address, in this case the mosquitto test server
char* LEDpin      = "2";                                    //  we will blink an LED when we get a message

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

char* deviceName = "IOTAppstoryPub";

// ================================================ SETUP ================================================
void setup() {
  /*
  IAS.onFirstBoot([]() {
    IAS.eraseEEPROM('P');                   // Optional! What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase
  });
  */
                                            // Adding the variables to our devices config menu so we can change them without having to flash our device
  IAS.addField(LEDpin, "LedPin", 2, 'P');
  IAS.addField(testMsg, "Payload", 40);
  IAS.addField(inTopic, "inTopic", 24);
  IAS.addField(outTopic, "outTopic", 24);
  IAS.addField(broker, "Broker", 24);
  IAS.begin();                              // Run IOTAppStory
  

  //-------- Your Setup starts from here ---------------
  
  PubSub.setServer(broker,1883);            //  Instantiate the pubsub client and give it a broker address and port number
  PubSub.setCallback(callback);             //  set a callback function to run when a we receive a message from a subscribed topic
    
  pinMode(IAS.dPinConv(LEDpin), OUTPUT);
}


// ================================================ LOOP =================================================
void loop() {
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)

  
  //-------- Your Sketch starts from here ---------------
  
  
  
  if (!PubSub.connected()) {
    reconnect();
  }
  PubSub.loop();  //  this routine checks the connection to the mqtt broker

  unsigned long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    Publish(testMsg);
  }
  
}



void Publish(const char* mqttPayload){
    if (PubSub.connected()) {
      Serial.println("Publishing...");
      Serial.println(outTopic);
      Serial.println(mqttPayload);
      PubSub.publish(outTopic, mqttPayload);
    } else {
      Serial.print("failed, rc=");
      Serial.print(PubSub.state());
      delay(1000);
    }
  }

  void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(IAS.dPinConv(LEDpin), !digitalRead(IAS.dPinConv(LEDpin)));   // Turn the LED on 
  } else {
    digitalWrite(IAS.dPinConv(LEDpin), !digitalRead(IAS.dPinConv(LEDpin)));   // Turn the LED off 
  }

}


void reconnect() {
  // Loop until we're reconnected
  while (!PubSub.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (PubSub.connect(deviceName)) {
      Serial.println("connected");
      // ... and resubscribe
      PubSub.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(PubSub.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}