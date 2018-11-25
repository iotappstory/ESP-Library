This is a beta for testing OTA updates to Nextion Displays</br></br>

## ESP8266 only
Currently this beta is esp8266 only. But the esp32 will follow!
</br></br>

## config.h defines
Do you want to OTA update your Nextion display? | true / false
NEXT_OTA true

Nextion reset pin | Default 5 / D1 | Use this pin to control a transistor or relay to "hard" reset your nextion
NEXT_RES 5

Nextion TX pin | Default 12 / D6
NEXT_TX 12

Nextion baudrate | 115200 / 57600 <- seems more stable
NEXT_BAUD	57600
</br></br>
