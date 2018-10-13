<img src="https://github.com/iotappstory/ESP-Library/blob/master/readme.jpg"/>

Wifi & OTA update manager for IOT devices (ESP8266 and ESP32). ESP8266's need at least 1MB flash. You will need a free account at IOTAppStory.com

Wiki pages: https://iotappstory.com/wiki/arduino-esp
</br></br>

## Latest stable release 2.0.0
https://github.com/iotappstory/ESP-Library/releases/latest

## Arduino IDE librairy manager
<img src="https://github.com/iotappstory/ESP8266-Library/blob/master/arduinoIDE_lib_manager.jpg"/>
</br>

## Prerequisites
#### ESP8266 specific
- <a href="https://github.com/esp8266/Arduino" target="_blank">ESP8266</a> core for Arduino &gt;= 2.4.2
- <a href="https://github.com/me-no-dev/ESPAsyncTCP" target="_blank">Async TCP Library</a> for ESP8266

#### ESP32 specific
- <a href="https://github.com/espressif/arduino-esp32" target="_blank">Arduino core</a> for the ESP32 &gt;= 1.0.0
- <a href="https://github.com/me-no-dev/AsyncTCP" target="_blank">Async TCP</a> Library for ESP32

#### ESP8266 and ESP32
- <a href="https://www.arduino.cc/en/Main/Software" target="_blank">Arduino IDE</a> &gt;= 1.8.5
- <a href="https://github.com/me-no-dev/ESPAsyncWebServer" target="_blank">Async Web Server</a>
</br>

## Develop branch
If you want to fork or contribute to the library. Please send your pull request to the "develop" branch.</br></br>

## API 2.0.0

### `IOTAppStory(const char* compDate, const int modeButton)`

Tells IAS the compilation date and what digital input is the force-update/reset button. Note: the EEPROM size and number of firmware variables are limited to 1024 and 12 respectively. If additional resources are needed beyond these limits `EEPROM_SIZE` and `MAXNUMEXTRAFIELDS` can be defined / modified in `IOTAppStory.h`.

```c
#define COMPDATE __DATE__ __TIME__
#define MODE_BUTTON 0


#include <IOTAppStory.h>
IOTAppStory IAS(COMPDATE, MODEBUTTON);

setup () { ... }
loop () { ... }
```
</br>

### `preSet...()`
With preSet's you can set various options to influence how `begin()` sets things up. All calls to
these function's must be done before calling `begin()`.

#### `preSetBoardname(String)`
Set the boardname, this is used as your Wifi Ap name in WIFI_AP mode. And is also used for your MDNS responder: http://example-name.local

#### `preSetAutoUpdate(bool)`
Setting to `true` will make the device do an update-check immediately after calling `begin()`. The default is `true`

#### `preSetAutoConfig(bool)`
Set whether or not the device should go into config mode after after failing to connect to a Wifi AP. The default is `true`

#### `preSetWifi(String, String)`
Set the WiFi credentials without going through the captive portal. For development only! Make sure to delete this preSet when you publish your App.


Example of using preSet's:

```c
...

setup () {
    ...

    IAS.preSetBoardname("virginsoil-full");
    IAS.preSetAutoUpdate(true);
    IAS.preSetAutoConfig(true);
    IAS.preSetWifi("ssid","password");


    IAS.begin();
}
```
</br>

### `addField(char* var, string fieldLab, uint maxLen, char type = 'L')`
reference to org variable | html field label | max nr of char | Optional "special field" char

These fields are added to the "App Settings" page in config mode and saved to eeprom. Updated values are returned to the original variable.

<img src="https://github.com/iotappstory/ESP-Library/blob/master/config-app-settings.jpg"/>

By default added fields will be renderd like the input field "TEXTLINE" in the pic above. You can use the other field types by adding the optional "special field" char. For more info about these fields have a look at the "VirginSoil-Full" example.


Currently only char arrays are supported.
Use functions like atoi() and atof() to transform the char array to integers or floats
Use dPinConv() to convert Dpin numbers(pin-name) to integers (D6 > 14)




Example:

```c
...

char* LEDpin    = "D4";
char* lbl1      = "Light Show";

setup () {
    ...
    
    IAS.addField(LEDpin, "ledPin", 2, 'P');
    IAS.addField(lbl1, "Label 1", 16);
    
    IAS.begin();
}
```
</br>

### `callbacks...()`
Thanks to [izanette](https://github.com/izanette) you can now configure callback functions that do something or give feedback to the user about the current state of the application. We have added a few of our own and hope you like it!


#### `onFirstBoot(THandlerFunction)`
Called on the first boot of this app. Will only run once. After serial or ota upload.

#### `onModeButtonNoPress(THandlerFunction)`
Called when state is changed to idle. (mode button is not pressed)

#### `onModeButtonShortPress(THandlerFunction)`
Called when state is changed to short press.

#### `onModeButtonLongPress(THandlerFunction)`
Called when state is changed to long press.

#### `onModeButtonVeryLongPress(THandlerFunction)`
Called when state is changed to very long press.

#### `onFirmwareUpdateCheck(THandlerFunction)`
Called when the app is about to check for firmware updates.

#### `onFirmwareUpdateDownload(THandlerFunction)`
Called when the app starts to download updates.

#### `onFirmwareUpdateError(THandlerFunction)`
Called when downloading firmware updates end in error.

#### `onConfigMode(THandlerFunction)`
Called when the app enters configuration mode.

In this example we use serial print to demonstrate the call backs. But you could use leds etc:
```c
...

setup () {
    ...
    
    IAS.onModeButtonShortPress([]() {
        Serial.println(F(" If mode button is released, I will enter in firmware update mode."));
        Serial.println(F("*-------------------------------------------------------------------------*"));
    });

    IAS.onModeButtonLongPress([]() {
        Serial.println(F(" If mode button is released, I will enter in configuration mode."));
        Serial.println(F("*-------------------------------------------------------------------------*"));
    });
    
    IAS.begin();
}
```
</br>

### `begin(char eraseEeprom)`
Set up IAS and start all dependent services. 

If `eraseEeprom` is 'F' (full), the entire EEPROM (including wifi credentials and IAS activation code) will be
erased on first boot of the sketch/app.

If `eraseEeprom` is 'P' (partial)(DEFAULT), some of the EEPROM (excluding wifi credentials and IAS activation code) will be
erased on first boot of the sketch/app.

If `eraseEeprom` is 'L' (leave intact), none of the EEPROM (including wifi credentials and IAS activation code) will be
erased on first boot of the sketch/app.</br></br>

Example:

```c
...

setup () {
    ...
    
    IAS.begin('P');
}
```
</br>


### `setCallHome(bool)`
Set to 'true' to enable calling home frequently (disabled by default)</br></br>

### `setCallHomeInterval(int)`
Call home interval in seconds, use 60s only for development. Please change it to at least 2 hours in production.

Example:

```c
...

setup () {
    ...
    
    IAS.begin();
    
    IAS.setCallHome(true);
    IAS.setCallHomeInterval(60);
}
```
</br>

### `loop()`
This function ensures the library performs all the periodic tasks it needs to performm:

* [`buttonLoop()`](#buttonLoop())
* [`callHome()`](#callHome()): This will be called if you have previously called [`setCallHome(true)`](#setCallHome(bool)) and if the timer set by [`setCallHomeInterval()`](#setCallHomeInterval(int)) (default = 7200 seconds or 2 hours) has expired.

It is essential that this function called on the first line of your main sketch's `loop()`.

### `buttonLoop()`
Checks if the button is depressed and what mode to enter when once it is released. Also returns the buttonstate in `ModeButtonState` format:
```
ModeButtonNoPress,        // mode button is not pressed
ModeButtonShortPress,     // short press - will enter in firmwareupdate mode
ModeButtonLongPress,      // long press - will enter in configurationmode
ModeButtonVeryLongPress,  // very long press - won't do anything (but the app developer might want to do something)
ModeButtonFirmwareUpdate, // about to enter in firmware update mode
ModeButtonConfigMode      // about to enter in configuration mode
```

### `callHome(bool spiffs)`
Calls IOTAppStory.com to check for updates. The `setCallHomeInterval()` function mentioned above already handles calling home at a certain interval. But if you would like to decide yourself under which circumstances and when to call home. This is for you.

Write whatever conditions you want and simply add the callHome() function.

If `spiffs` is true, the call also checks if there is a new filesystem image to download.</br></br>

```c
...

void loop() {
    IAS.loop();
    
    if(batLvl >= 20 && lstUpd != today){
        IAS.callHome();
        lstUpd = today;
    }
}
```
</br>

### `dPinConv(...)`
Use dPinConv() to convert Dpin & Ppin numbers to integers (D6 > 14)

We advise to use the original Gpio numbers used by the ESP. However a lot of our users use boards like the “Wemos” & “NodeMCU”. They tend to use the silkscreen numbering like D6, D2 etc.

Normally this would be handled by the defined ”const” in your boards [pins_arduino.h](https://github.com/esp8266/Arduino/tree/master/variants) file during compilation. However when entered after compilation, like with the added fields in the html config pages “addField(.. .. .. ..)”, these values get returned as ”String” instead of being translated to a “int”

And if you originally developed your code for these “Special ESP’s”. This part makes makes it compatible when compiling for "Generic ESP's"

<img src="https://github.com/iotappstory/ESP-Library/blob/master/pin_mapping.jpg"/>
</br>

## IAS WIKI
https://iotappstory.com/wiki/arduino-esp
</br></br>

## Contributions and thanks
Thanks to [msiebuhr](https://github.com/msiebuhr) for this readme file.

And thankyou to all of you who made a [pull request](https://github.com/iotappstory/ESP-Library/graphs/contributors)
