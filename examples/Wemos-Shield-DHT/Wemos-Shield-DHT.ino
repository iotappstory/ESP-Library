/* DHT Pro Shield - Simple

   Example testing APPNAME for various DHT humidity/temperature sensors
   Written by ladyada, public domain

   Depends on Adafruit DHT Arduino library
   https://github.com/adafruit/DHT-sensor-library
*/

#define APPNAME "WemosTempSensor"
#define VERSION "V1.1.0"
#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON D3


#include <IOTAppStory.h>
#include <DHT.h>
#include <Wire.h>           // Include Wire if you're using I2C
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library
#include <IOTAppStory.h>


#define PIN_RESET 255       //
#define DC_JUMPER 0         // I2C Addres: 0 - 0x3C, 1 - 0x3D
#define DHTPIN D4           // what pin we're connected to


// Uncomment whatever type you're using!
//#define DHTTYPE DHT11       // DHT 11
//#define DHTTYPE DHT21     // DHT 21 (AM2301)
#define DHTTYPE DHT22     // DHT 22  (AM2302)


// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor


// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);
MicroOLED oled(PIN_RESET, DC_JUMPER); // Example I2C declaration

IOTAppStory IAS(APPNAME, VERSION, COMPDATE, MODEBUTTON);



// ================================================ VARS =================================================
int tempEntry;

// We want to be able to edit these example variables below from the wifi config manager
// Currently only char arrays are supported.
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)

char* scale = "0"; // 0 = Celsius, 1 = Fahrenheit



// ================================================ SETUP ================================================
void setup() {
  IAS.serialdebug(true);                                // 1st parameter: true or false for serial debugging. Default: false
  //IAS.serialdebug(true,115200);                       // 1st parameter: true or false for serial debugging. Default: false | 2nd parameter: serial speed. Default: 115200


  dht.begin();                                          // Initialize dht sensor
  oled.begin();                                         // Initialize the OLED
  oled.clear(PAGE);                                     // Clear the display's internal memory
  oled.clear(ALL);                                      // Clear the library's display buffer
  
  
  IAS.preSetBoardname("wemos-dht");                     // preset Boardname this is also your MDNS responder: http://virginSoil-full.local
  IAS.preSetAutoUpdate(true);                           // automaticUpdate (true, false)


  IAS.addField(scale, "scale", "Celc Fahr (0,1)", 1);   // These fields are added to the config wifimanager and saved to eeprom. Updated values are returned to the original variable.

 
  IAS.begin(true,'P');                                  // 1st parameter: true or false to view BOOT STATISTICS
                                                        // 2nd parameter: Wat to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase(default) | 'L' Leave intact

  IAS.setCallHome(true);                                // Set to true to enable calling home frequently (disabled by default)
  IAS.setCallHomeInterval(60);                          // Call home interval in seconds, use 60s only for development. Please change it to at least 2 hours in production


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

}



// ================================================ LOOP =================================================
void loop() {
  IAS.buttonLoop();   // this routine handles the calling home functionality and reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration


  //-------- Your Sketch starts from here ---------------
  
  float h, t, f;
  if (millis() > tempEntry + 2000) { // Wait a few seconds between measurements.

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    do {
      h = dht.readHumidity();
    } while (isnan(h));
    
    // Read temperature as Celsius (the default)
    do {
      t = dht.readTemperature();
    } while (isnan(t));
    
    // Read temperature as Fahrenheit (isFahrenheit = true)
    do {
      f = dht.readTemperature(true);
    } while (isnan(f));

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) Serial.println("Failed to read from DHT sensor!");

    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

    if(atoi(scale) == 0){
      displayTemp(t, F("Celsius"));
    }else{
      displayTemp(f, F("Fahrenh"));
    }

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F(" %\t"));
    Serial.print(F("Temperature: "));
    Serial.print(t);
    Serial.print(F(" *C "));
    Serial.print(f);
    Serial.print(F(" *F\t"));
    Serial.print(F("Heat index: "));
    Serial.print(hic);
    Serial.print(F(" *C "));
    Serial.print(hif);
    Serial.println(F(" *F"));
    tempEntry = millis();
  }

}

void displayTemp(float temp, String scale) {
  oled.setFontType(2);    // Set the text to medium/7-segment (5 columns, 3 rows worth of characters).
  oled.setCursor(0, 5);
  oled.print(temp);       // Print a float
  oled.setCursor(0, 25);  // Set the text cursor to the upper-left of the screen.
  oled.setFontType(1);
  oled.print(scale);
  oled.display();         // Draw to the screen
}
