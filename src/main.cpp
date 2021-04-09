/*
   Based on Neil Kolban example for IDF:
   https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by pcbreflux
   Slightly restructured by JoBa1
*/

/*
   Create a BLE server that will send periodic iBeacon frames.
   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create advertising data with major/minor representing boot count hi/lo
   3. Start advertising.
   4. wait
   5. Stop advertising.
   6. deep sleep

*/

#include <Arduino.h>

#include "sys/time.h"
#include "NimBLEBeacon.h"
#include "NimBLEDevice.h"
#include "esp_sleep.h"

#define BUTTON_PIN 39 // to activate serial output
#define DEEP_SLEEP_DURATION_MS 500 // sleep x milliseconds and then wake up
#define ADVERTISING_DURATION_MS 100 // advertise x milliseconds and then sleep

RTC_DATA_ATTR static uint32_t last_ms; // remember last boot in RTC Memory
RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory

// See https://www.uuidgenerator.net/ for generating UUIDs
// or use linux tool uuidgen
#define BEACON_UUID "49d3b970-625b-40e3-b96b-32466f297fdb"

void setBeaconData(BLEAdvertising *pAdv) {

  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4C00); // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));
  oBeacon.setMajor((bootcount & 0xFFFF0000) >> 16);
  oBeacon.setMinor(bootcount & 0xFFFF);

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED 0x04

  std::string strServiceData = "";

  strServiceData += (char)26;   // Len
  strServiceData += (char)0xFF; // Type
  strServiceData += oBeacon.getData();
  oAdvertisementData.addData(strServiceData);

  pAdv->setAdvertisementData(oAdvertisementData);
  pAdv->setScanResponseData(oScanResponseData);
  pAdv->setAdvertisementType(BLE_GAP_CONN_MODE_NON);
}

void setup() {
  // Current time since boot
  struct timeval now;
  gettimeofday(&now, NULL);
  uint32_t curr_ms = now.tv_sec * 1000 + now.tv_usec / 1000;

  // Print to serial if button is pressed or bootcount is low
  pinMode(BUTTON_PIN, INPUT);
  bool print = digitalRead(BUTTON_PIN) == LOW || bootcount < 10;

  // Print some useful infos
  bootcount++;
  if (print) {
    Serial.begin(115200);
    Serial.println("\nStart JoBa iBeacon " BEACON_UUID);
    Serial.printf("Since reset %d wakeups\n", bootcount);
    Serial.printf("Last reset %lds ago\n", now.tv_sec);
    Serial.printf("Deep sleep cycle was %u ms\n", curr_ms - last_ms);
  }
  last_ms = curr_ms;

  // Advertise iBeacon
  BLEDevice::init("JoBa");
  if( print ) {
    Serial.printf("BLE device address is %s\n", BLEDevice::getAddress().toString().c_str());
  }
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  setBeaconData(pAdvertising);
  pAdvertising->start();
  if( print ) {
    Serial.printf("Advertising for %d ms\n", ADVERTISING_DURATION_MS);
  }
  delay(ADVERTISING_DURATION_MS);
  pAdvertising->stop();

  // Go to sleep
  if( print ) {
    Serial.printf("Enter deep sleep for %d ms\n\n", DEEP_SLEEP_DURATION_MS);
  }
  esp_deep_sleep(1000LL * DEEP_SLEEP_DURATION_MS);

  if (print) {
    Serial.println("Deep sleep failed!");
  }
}

void loop() {}