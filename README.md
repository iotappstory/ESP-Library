This is a beta for testing OTA updates to Nextion Displays using the IOTAppStory.com infrastructure.</br></br>

## Prerequisites
[ESPNexUpload](https://github.com/Onno-Dirkzwager/ESPNexUpload)
This is a heavily modified fork of [Nredor](https://github.com/Nredor)'s library and with a bit of luck he might accept my [pullrequest](https://github.com/Nredor/ESPNexUpload/pull/2) and make it available in the Arduino Library Manager.
</br></br>


## ESP8266 & ESP32
Previous versions of this beta only supported the esp8266. Since 31-12-2018 we also support the ESP32 so update your library if necessary.
</br></br>


## config.h defines
Do you want to OTA update your Nextion display? | true / false
</br>NEXT_OTA true

Nextion reset pin | Default 5 / D1 | Use this pin to control a transistor or relay to "hard" reset your nextion
</br>NEXT_RES 5

Nextion TX pin | Default 12 / D6
</br>NEXT_TX 12

Nextion baudrate | 115200
</br>NEXT_BAUD	115200
</br></br>
