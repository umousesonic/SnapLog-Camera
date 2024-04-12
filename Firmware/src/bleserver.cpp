#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// https://www.uuidgenerator.net/version7
#define SERVICE_UUID           "018ed48a-a65c-718d-a58e-dae287ec41fd"  
#define CHARACTERISTIC_UUID_TX "018ed48c-1ea0-73d5-ab67-af89e0d89179"

// ble call back: detect when nodes connect and disconnect from a device, 
// and when they get subscribed and read data
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

static void setupBLE() {
    BLEDevice::init("ESP32Cam");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY  // send updates to client whenever characteristic changes
                      );
                      
    pTxCharacteristic->addDescriptor(new BLE2902()); 

    pService->start();
    pServer->getAdvertising()->start();
}

static bool process_function(size_t width, size_t height, pixformat_t format, uint8_t* buf, size_t len) {
    if (!deviceConnected) {
        Serial.println("Waiting for a client connection...");
        return false;
    }

    Serial.println("Client is connected, sending picture...");

    // Send the picture size first
    pTxCharacteristic->setValue((uint8_t*)&len, sizeof(size_t));
    pTxCharacteristic->notify();
    delay(10); // Prevent BLE congestion

    // Send the picture buffer in chunks
    for (size_t i = 0; i < len; i += 20) {
        pTxCharacteristic->setValue(&buf[i], (len - i) < 20 ? (len - i) : 20);
        pTxCharacteristic->notify();
        delay(10); // Prevent BLE congestion
    }

    Serial.println("Picture sent to client");
    return true;
}

void setup() {
    Serial.begin(115200);
    setupBLE();
}

void loop() {
    // Handle BLE connection status
    if (deviceConnected) {
        // Here you can send data if needed
    }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}


