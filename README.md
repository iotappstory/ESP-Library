This is a beta for testing OTA updates to Nextion Displays. If you want to test this beta comment your IAS username in this <a href="https://github.com/iotappstory/ESP-Library/issues/86">issue</a> and we will add a nextion upload field to your account so you can test ota updates to your screen.</br></br>

## Prerequisites
#### ESP8266
- <a href="https://github.com/Nredor/ESPNexUpload" target="_blank">ESPNexUpload</a>
- SoftwareSerial
</br></br>


## ESP8266 only
Currently this beta is esp8266 only. But the esp32 will follow!
</br></br>


## config.h defines
Do you want to OTA update your Nextion display? | true / false
</br>NEXT_OTA true

Nextion reset pin | Default 5 / D1 | Use this pin to control a transistor or relay to "hard" reset your nextion
</br>NEXT_RES 5

Nextion TX pin | Default 12 / D6
</br>NEXT_TX 12

Nextion baudrate | 115200 / 57600 <- seems more stable
</br>NEXT_BAUD	57600
</br></br>

## Note on baudrates
- Nextion <-> ESP8266 communication seems to be quite fault prone. 
- With ESP8266 arduino core v2.4.0: Doesn't work with 115200 baudrate. 
57600 seems to work fine. 
- With ESP8266 arduino core v2.3.0: 115200 works fine. 
