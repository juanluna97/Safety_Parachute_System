#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_LSM303.h>


int deletethis= 1;

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        if (value[0] == 'y')
          digitalWrite(5, HIGH);
      }
    }
};