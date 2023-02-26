#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID        "02afd1d9-f889-4f49-acc7-33b34a620adb"
#define CHARACTERISTIC_UUID "3604da64-2145-422a-89c6-c540392c3a6a"

std::string encode_packet(uint16_t data);

std::string encode_payload();

#define LENGTH 30

struct CircularQueue {
  uint16_t data[LENGTH];
  int start = 0;
  int capacity = 0;

  const uint16_t &operator[](int idx) const {
    return data[(start + idx) % LENGTH];
  }

  uint16_t &operator[](int idx) { return data[(start + idx) % LENGTH]; }

  void append(uint16_t val) {
    if (capacity == LENGTH) {
      data[start] = val;
      start = (start + 1) % LENGTH;
      return;
    }
    data[capacity++] = val;
  }
};

CircularQueue queue;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        Serial.println(value.c_str());
        if (value == "1") {
          std::string data = encode_payload();
          Serial.println(data.c_str());
          pCharacteristic->setValue(data);
          pCharacteristic->notify();
          Serial.println("Sent results");
        }
        else {
          Serial.println("Wrong value");
        }
        Serial.println("*********");
      }
    }
};

void setup() {
  Serial.begin(115200);
  srand((unsigned) time(NULL));
  // Create the BLE Device
  BLEDevice::init("ESP32-Edge");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->setCallbacks(new MyCallbacks());
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    uint16_t packet = rand() % 4096;
    queue.append(packet);
    // notify changed value
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    delay(1000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
}

std::string encode_packet(uint16_t data) {
  return std::to_string(data) + '@';
}

std::string encode_payload() {
  std::string payload = "";
  for (int i = 0; i < queue.capacity; i++) {
    payload += encode_packet(queue[i]);
  }
  return payload;
}
