<img src="https://github.com/iotappstory/ESP8266-Library/blob/master/readme.jpg"/>

Download and update Infrastructure for IOT devices, currenlty the ESP8266. You will need an account at IOTAppStory.com

Wiki pages: https://iotappstory.com/wiki

## Latest release 1.0.6
https://github.com/iotappstory/ESP8266-Library/releases/latest

## Develop branch

If you want to fork or contribute to the library. Please send your pull request to the "develop" branch.

## API

### `IOTAppStory(char* appName, char* appVersion, char* compDate, char* modeButton)`

Tells IAS the name of the application, its version, compilation date and what
digital input is the force-update/reset button. Note: the EEPROM size and number of firmware variables are limited to 1024 and 12 respectively. If additional resources are needed beyond these limits `EEPROM_SIZE` and `MAXNUMEXTRAFIELDS` can be defined / modified in `IOTAppStory.h`.

```c
#define APPNAME my_app
#define VERSION V1.0.0
#define COMPDATE __DATE__ __TIME__
#define MODE_BUTTON D3


#include <IOTAppStory.h>
IOTAppStory IAS(APPNAME, VERSION, COMPDATE, MODEBUTTON);

setup () { ... }
loop () { ... }
```

### `serialdebug(bool enabled, int speed)`

Set if sending debug over serial is enabled and, if so, at what speed to send
data.

### `preSetConfig(...)`

Sets various options to influence how `begin()` sets things up. All calls to
this function must be done before calling `begin()`.

#### `preSetConfig(bool automaticUpdate)`

Setting to `true` will make the device do an update-check immediately after
calling `begin()`.

#### `preSetConfig(string boardName, bool automaticUpdate)`

Set the device hostname. This is important as devices by default has APPNAME as
hostname (set by the `IOTAppStory` constructor), which can lead to all sorts of
networking issues.

Examples of setting it:
```c
#define APPNAME my_awesome_app
IOTAppStory(APPNAME, ...);

begin () {
    // C++-style appname + mac
    IAS.preSetConfig(APPNAME"_" + WiFi.macAddress());

    // C-style w. sprintf (ex. "ESP_DEF123")
    char hostString[16] = {0};
    sprintf(hostString, "ESP_%06X", ESP.getChipId());
    IAS.preSetConfig(hostString, false);

    // Now start it up
    IAS.start();
}
```

#### `preSetConfig(string ssid, string password, bool automaticUpdate)`

Set the WiFi credentials without going through the captive portal.

#### `preSetConfig(string ssid, string password, string boardName, bool automaticUpdate)`

As the two above; sets WiFi credentials and boardName.


#### `preSetConfig(string ssid, string password, string boardName, string url1, string url2, bool automaticUpdate)`

As above + set urls for contacting IOTAppStory.com. Generally not needed.


### `addField(char* var, string fieldName, string fieldVar, uint maxLen)`

These fields are added to the config wifimanager and saved to eeprom.  Updated
values are returned to the original variable.

```c
char* LEDpin = "D4";

setup () {
    IAS.addField(LEDpin, "ledpin", "ledPin", 2);
    IAS.begin();
}

loop () {
    IAS.buttonLoop();
    // dPinConv() converts pin-name to appropriate number
    pinMode(IAS.dPinConv(LEDpin), OUTPUT);
}
```

### `begin(bool bootstat, char ea)`

Set up IAS and start all dependent services. 

If `bootstat` is true, the code will keep track of number of boots and print
contents of RTC memory.

If `ea` is 'F' (full), the entire EEPROM (including wifi credentials and IAS activation code) will be
erased on first boot of the sketch/app.

If `ea` is 'P' (partial), some of the EEPROM (excluding wifi credentials and IAS activation code) will be
erased on first boot of the sketch/app.

If `ea` is 'L' (leave intact), none of the EEPROM (including wifi credentials and IAS activation code) will be
erased on first boot of the sketch/app.

### `buttonLoop()`

Checks if the button is depressed and what mode to enter when once it is
released. Call in `loop()`.

### `callHome(bool spiffs)`

Calls IOTAppStory.com to check for updates. OK to call every ~5 minutes in
development, but production setups should call at most every two hours.

If `spiffs` is true, the call also checks if there is a new filesystem image to
download.

### `dPinConv(...)`

See `addField()`

## Contributions and thanks

For Wifi AP management we forked and modified the WifiManager from [kentaylor](https://github.com/kentaylor/WiFiManager) which in its turn was a fork from [tzapu](https://github.com/tzapu/WiFiManager)

Thanks to [msiebuhr](https://github.com/msiebuhr) for this readme file.

And thankyou to all of you who made a [pull request](https://github.com/iotappstory/ESP8266-Library/graphs/contributors)
