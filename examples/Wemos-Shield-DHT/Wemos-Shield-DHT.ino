/* DHT Pro Shield - Simple

   Example testing sketch for various DHT humidity/temperature sensors
   Written by ladyada, public domain

   Depends on Adafruit DHT Arduino library
   https://github.com/adafruit/DHT-sensor-library
*/

#define SKETCH "WemosTempSensor"
#define VERSION "V1.0.0"
#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON D3

#include <IOTAppStory.h>
#include <DHT.h>
#include <Wire.h>  // Include Wire if you're using I2C
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library
#include <IOTAppStory.h>

#define DHTPIN D4     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302)

#define PIN_RESET 255  //
#define DC_JUMPER 0  // I2C Addres: 0 - 0x3C, 1 - 0x3D

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

IOTAppStory IAS(SKETCH, VERSION, COMPDATE, MODEBUTTON);

int tempEntry;
unsigned long iotEntry = millis();

void setup() {
  IAS.serialdebug(true);                  // 1st parameter: true or false for serial debugging. Default: false
  //IAS.serialdebug(true,115200);         // 1st parameter: true or false for serial debugging. Default: false | 2nd parameter: serial speed. Default: 115200


  //-------- Your Setup starts from here ---------------

  dht.begin();
  // Before you can start using the OLED, call begin() to init
  // all of the pins and configure the OLED.
  oled.begin();
  oled.clear(ALL); // will clear out the OLED's graphic memory.
  oled.clear(PAGE);  // Clear the display's memory (gets rid of artifacts)
 IAS.begin(true);           // true to view BOOT STATISTICS, has to be placed after oled.begin()

}

void loop() {
  float h, t, f;

  IAS.buttonLoop();

  if (millis() > iotEntry + 30000) {             // only for development. Please change it to at least 2 hours in production
    IAS.callHome();
    iotEntry = millis();
  }

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

    displayTemp(t);

    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" *C ");
    Serial.print(hif);
    Serial.println(" *F");
    tempEntry = millis();
  }

}

void displayTemp(float temp) {
  oled.setFontType(2);  // Set the text to medium/7-segment (5 columns, 3 rows worth of characters).
  oled.setCursor(0, 5);
  oled.print(temp);  // Print a float
  oled.setCursor(0, 25);  // Set the text cursor to the upper-left of the screen.
  oled.setFontType(1);
  oled.print("Celsius");
  oled.display(); // Draw to the screen
}

