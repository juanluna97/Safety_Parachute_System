/*Main code to run the Safety Parachute System
Author: Juan Luna
*/
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_LSM303.h>

#define BUZZER 18
#define MOSFET 5

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 5        /* Time ESP32 will go to sleep (in seconds) */

#define BMP_SCK 13
#define BMP_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

#define FAULTY_WIRING "Wiring is faulty"
#define BATTERY_FAILURE "Battery's life is critically low"
#define SYSTEM_FAILURE "The Watchdog System has detected an error"
#define CONNECTION_FAILURE "The system was unable to connect"

#define SERVICE_UUID_ALTITUDE "ad92e76e-b69f-47ed-b9b7-3b32a8af3fbf"
#define SERVICE_UUID_ACCELERATION "b4c9bead-52dd-402e-abaa-2a81ec3e8a75"
#define SERVICE_UUID_TIMER "a8979325-265d-4092-a3cc-172c5f08634a
#define SERVICE_UUID_DEPLOYMENT "75c34e42-10e0-4a2a-b754-139c24266586"

#define CHARACTERISTIC_UUID_ALTITUDE "b9312052-4925-46cb-a6a7-de8228b959bc"
#define CHARACTERISTIC_UUID_ACCELERATIONX "3fbdda9e-66c3-4d19-9d9c-e51a7652e7d2"
#define CHARACTERISTIC_UUID_ACCELERATIONY "12a295f0-a5df-4201-9382-1ba9980fa54b"
#define CHARACTERISTIC_UUID_ACCELERATIONZ "28d95fab-1bc0-45fe-a018-730251f63279"
#define CHARACTERISTIC_UUID_TIMER "b7aa57e8-605d-4ded-9c69-3024f13fe502"
#define CHARACTERISTIC_UUID_DEPLOYMENT "80ad1283-0eb0-4826-a788-c24b41f5df7b"

Adafruit_BMP3XX bmp;
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
RTC_DATA_ATTR int bootCount = 0;
unsigned long myTime;
bool deployment = false;

class Altitude : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();
  };

  class Acceleration : public BLECharacteristicCallbacks
  {
    void onWrite(BLECharacteristic *pCharacteristic)
    {
      std::string value = pCharacteristic->getValue();
    };

    class Timer : public BLECharacteristicCallbacks
    {
      void onWrite(BLECharacteristic *pCharacteristic)
      {
        std::string value = pCharacteristic->getValue();
      };

      class Deployment : public BLECharacteristicCallbacks
      {
        void onWrite(BLECharacteristic *pCharacteristic)
        {
          std::string value = pCharacteristic->getValue();
          /*This section is to deploy the parachute in the case of a manual deployment.*/
          if (value.length() > 0)
          {
            if (value[0] == 'y')
            {
              digitalWrite(MOSFET, HIGH);
              tone(BUZZER, 1000, 10000)
            }

            if (value[0] == 'n')
              digitalWrite(MOSFET, LOW);
          }
        }
      };

      class MyCallbacks : public BLECharacteristicCallbacks
      {
        void onWrite(BLECharacteristic *pCharacteristic)
        {
          std::string value = pCharacteristic->getValue();
          /*This section is to deploy the parachute in the case of a manual deployment.*/
          if (value.length() > 0)
          {
            if (value[0] == 'y')
            {
              digitalWrite(MOSFET, HIGH);
              tone(BUZZER, 1000, 10000)
            }

            if (value[0] == 'n')
              digitalWrite(MOSFET, LOW);
          }
        }
      };

      /*This section is used to wakeup the ESP32 from sleep mode*/

      void print_wakeup_reason()
      {
        esp_sleep_wakeup_cause_t wakeup_reason;

        wakeup_reason = esp_sleep_get_wakeup_cause();

        switch (wakeup_reason)
        {
        case ESP_SLEEP_WAKEUP_EXT0:
          Serial.println("Wakeup caused by external signal using RTC_IO");
          break;
        case ESP_SLEEP_WAKEUP_TIMER:
          Serial.println("Wakeup caused by timer");
          break;
        default:
          Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
          break;
        }
      }

      void setup()
      {
        Serial.begin(9600);

        myTime = millis();

        /*This section is for the deployment pin*/
        pinMode(MOSFET, OUTPUT);

        /*This section is for the buzzer*/
        pinMode(BUZZER, OUTPUT);

        /*This section is for the BMP388*/
        bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
        bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
        bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
        bmp.setOutputDataRate(BMP3_ODR_50_HZ);

        /*This section is for the LSM303DLHC*/

        /*This section is used to start the BLE Server and advertice information*/
        BLEDevice::init("Safety Parachute System");

        /*Service 1 is responsible for sending the altitude of the system.*/
        BLEServer *pServer = BLEDevice::createServer();
        BLEService *pService1 = pServer->createService(SERVICE_UUID_ALTITUDE);
        BLECharacteristic *pCharacteristic = pService1->createCharacteristic(
            CHARACTERISTIC_UUID_ALTITUDE,
            BLECharacteristic::PROPERTY_READ |
                BLECharacteristic::PROPERTY_WRITE);

        /*Service 2 is responsible for sending the acceleration of the system.*/
        BLEService *pService2 = pServer->createService(SERVICE_UUID_ACCELERATION);
        BLECharacteristic *pCharacteristic = pService2->createCharacteristic(
            CHARACTERISTIC_UUID_ACCELERATIONX,
            CHARACTERISTIC_UUID_ACCELERATIONY,
            CHARACTERISTIC_UUID_ACCELERATIONZ,
            BLECharacteristic::PROPERTY_READ |
                BLECharacteristic::PROPERTY_WRITE);

        /*Service 3 is responsible for sending the time of the flight*/
        BLEService *pService3 = pServer->createService(SERVICE_UUID_ACCELERATION);
        BLECharacteristic *pCharacteristic = pService3->createCharacteristic(
            CHARACTERISTIC_UUID_TIMER,
            BLECharacteristic::PROPERTY_READ |
                BLECharacteristic::PROPERTY_WRITE);

        /*Service 4 is responsble for sending the verification of the manual deployment signal. */
        BLEService *pService4 = pServer->createService(SERVICE_UUID_ACCELERATION);
        BLECharacteristic *pCharacteristic = pService4->createCharacteristic(
            CHARACTERISTIC_UUID_DEPLOYMENT,
            BLECharacteristic::PROPERTY_READ |
                BLECharacteristic::PROPERTY_WRITE);

        /*Initiating servers and sending information via BLE*/
        pCharacteristic->setCallbacks(new Altitude());
  pCharacteristic->setValue(bmp.readAltitude(SEALEVELPRESSURE_HPA);
  pService1->start();
  pCharacteristic->setCallbacks(new Acceleration());
  pCharacteristic->setValue(acceleration.x,acceleration.y,acceleration.z);
  pService2->start();
  pCharacteristic->setCallbacks(new Timer());
  pCharacteristic->setValue(myTime);
  pService3->start();
  pCharacteristic->setCallbacks(new cla());
  pCharacteristic->setValue(deployment);
  pService4->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
      }

      void loop()
      {
      }