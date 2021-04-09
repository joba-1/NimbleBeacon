# NimbleBeacon

ESP32 iBeacon with NimBLE library

## Prepare src/main.cpp
* Generate a UUID with linux tool uuidgen or with online service https://www.uuidgenerator.net/
* Change the BEACON_UUID
* If you have a button available to activate serial output, set BUTTON_PIN to its gpio number
* You can also adjust DEEP_SLEEP_DURATION_MS and ADVERTISING_DURATION_MS to your needs

Sends boot count in iBeacons major/minor fields. 
You can set it to whatever you want in setBeaconData().
