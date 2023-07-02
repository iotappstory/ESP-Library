
/*
  This is an initial sketch to be used as a "blueprint" to create apps which can be used with IOTappstory.com infrastructure
  Your code can be filled wherever it is marked.

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


//  Declaring some variables, we will later use IAS.addField() so we can change them later through our devices config menu 
char* testMsg   = "Hello from the IOTAppstory pubsub example ";   //  demo text, this is the payload that will be sent
char* inTopic   = "test/iotappstory";                             //  the topic we will subscribe to
char* outTopic  = "test/iotappstory";                             //  the topic we will publish to
char* broker    = "192.168.178.2";                                //  the broker server ip address or url
//  char* broker    = "test.mosquitto.org";
//  char* broker    = "mqtt.eclipse.org";
char* LEDpin    = "2";                                            //  we will blink an LED when we get a message





unsigned long lastMsg = 0;

char* deviceName = "IOTAppstory";
unsigned long now = 0;
// ================================================ SETUP ================================================
void setup() {

  IAS.onFirstBoot([]() {
    IAS.eraseEEPROM('P');                   //  Optional! What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase
  });

 
                                            //  Adding the variables to our devices config menu so we can change them without having to flash our device
  IAS.addField(LEDpin,  "LedPin", 2, 'P');  //  Make a config field so we can assign an LED pin
  IAS.addField(testMsg, "Payload", 50);     //  a text field to fill in our payload for test purposes
  IAS.addField(inTopic, "inTopic", 24);     //  a text field to set a topic we want to subscribe to
  IAS.addField(outTopic,"outTopic", 24);    //  a text field to set the topic we want to publish to
  IAS.addField(broker,  "Broker", 24);      //  a text field to set the broker address (ip or url)
  
  IAS.begin();                              //  Run IOTAppStory
  

  //-------- Your Setup starts from here ---------------
  
  PubSub.setServer(broker,1883);            //  Instantiate the pubsub client and give it a broker address and port number
  PubSub.setCallback(callback);             //  set a callback function to run when a we receive a message from a subscribed topic
    
  pinMode(IAS.dPinConv(LEDpin), OUTPUT);
  reconnect();                              //  Connect to the mqtt broker
}


// ================================================ LOOP =================================================
void loop() {
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)
  

  PubSub.loop();  //  this routine checks the connection to the mqtt broker
  //  publish a message to the broker every minute
  now = millis();
  if (now - lastMsg > 60000) {
    lastMsg = now;
    Publish(testMsg);
  }
  
}



void Publish(const char* mqttPayload){
  //  check if we are still connected to the broker
    if (PubSub.connected()) {
      Serial.println("Publishing...");
      Serial.println(outTopic);
      Serial.println(mqttPayload);
      PubSub.publish(outTopic, mqttPayload);
    } else {
      reconnect();
    }
  }

  void callback(char* topic, byte* payload, unsigned int length) {
    digitalWrite(2,HIGH);
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.print("length [");
    Serial.print(length);
    Serial.print("]");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
}


void reconnect() {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (PubSub.connect(deviceName)) {
      Serial.println("connected");
      // ... and resubscribe
      PubSub.subscribe(inTopic);
    } else {
      Serial.print("failed, rc= ");
      //  if connection failed, print reason of failure
      int state = PubSub.state();
      switch(state){
        case -4:
        Serial.print("MQTT_CONNECTION_TIMEOUT");
        Serial.println("- the server didn't respond within the keepalive time");
        break;
        
        case -3:
        Serial.print("MQTT_CONNECTION_LOST");
        Serial.println("- the network connection was broken");
        break;
        
        case -2:
        Serial.print("MQTT_CONNECT_FAILED");
        Serial.println("- the network connection failed");
        break;
        
        case -1:
        Serial.print("MQTT_DISCONNECTED");
        Serial.println("- the client is disconnected cleanly");
        break;
        
        case 0:
        Serial.print("MQTT_CONNECTED");
        Serial.println("- the client is connected");
        break;
        
        case 1:
        Serial.print("MQTT_CONNECT_BAD_PROTOCOL");
        Serial.println("- the server doesn't support the requested version of MQTT");
        break;
        
        case 2:
        Serial.print("MQTT_CONNECT_BAD_CLIENT_ID");
        Serial.println("- the server rejected the client identifier");
        break;
        
        case 3:
        Serial.print("MQTT_CONNECT_UNAVAILABLE");
        Serial.println("- the server was unable to accept the connection");
        break;
        
        case 4:
        Serial.print("MQTT_CONNECT_BAD_CREDENTIALS");
        Serial.println("- the username/password were rejected");
        break;
        
        case 5:
        Serial.print("MQTT_CONNECT_UNAUTHORIZED");
        Serial.println("- the client was not authorized to connect");
        break;
        }
    }
}