#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "datapkt.h"

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic, *pRxCharacteristic;
bool deviceConnected = false;
static uint16_t id = 0; // Initialize to 0

// https://www.uuidgenerator.net/version7
#define SERVICE_UUID           "018ed48a-a65c-718d-a58e-dae287ec41fd"  
#define CHARACTERISTIC_UUID_TX "018ed48c-1ea0-73d5-ab67-af89e0d89179"
#define CHARACTERISTIC_UUID_RX "018ed48d-2f98-76de-bf56-ef87e9d89281"

// BLE callback: detect when nodes connect and disconnect from a device, 
// and when they get subscribed and read data
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
      deviceConnected = true;
      id = 0; // Reset ack on new connection
    };

    void onDisconnect(BLEServer* pServer) override {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        if (pCharacteristic == pRxCharacteristic) {
            std::string rxValue = pCharacteristic->getValue();
            if (rxValue.length() >= 2) {
                id = rxValue;
            }
        }
    }
};

static void initializeBLE() {
    BLEDevice::init("ESP32Cam");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY // send updates to client whenever characteristic changes
                      );
    pRxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_RX,
                        BLECharacteristic::PROPERTY_WRITE   // allows write to the ble server
                      );
                      
    pTxCharacteristic->addDescriptor(new BLE2902());    // used for enabling and disabling notifications from a BLE characteristic
    pRxCharacteristic->addDescriptor(new BLE2902());
    pRxCharacteristic->setCallbacks(new MyCallbacks());

    pService->start();
    pServer->getAdvertising()->start();
}

static bool process_function(size_t width, size_t height, pixformat_t format, uint8_t* buf, size_t len) {
    // Init server
    if (!pServer) {
        initializeBLE();
    }

    datapkt_t packet;

    unsigned long startTime = millis();
    while (!deviceConnected || ack == 0xFFFF) {
        if (millis() - startTime > 10000) { // 10 seconds timeout
            Serial.println("Timeout waiting for client ack.");
            return false; // go back to sleep
        }
    }

    // Conneted
    uint16_t totalPackets = (len / PACKET_MAX_DATA) + (len % PACKET_MAX_DATA > 0 ? 1 : 0);
    while(deviceConnected) {
        if (id != totalPackets) {
            packet.pktsize = sizeof(datapkt_t);
            packet.data_size = PACKET_MAX_DATA;
            packet.id = id;
            packet.total = totalPackets;
            packet.data = (buf + id*PACKET_MAX_DATA);
        }
        else {
            packet.data_size = len - id*PACKET_MAX_DATA;
            packet.pktsize = data_size + sizeof(datapkt_t) - PACKET_MAX_DATA;
            packet.id = id;
            packet.total = totalPackets;
            packet.data = (buf + id*PACKET_MAX_DATA);
        }
        Serial.write("Setting tx to packet " + std::to_string(id) + "\n");
        pTxCharacteristic->setValue((uint8_t*)&packet, packet.pktsize);
        pTxCharacteristic->notify();
        delay(10); // Prevent BLE congestion
    }
    Serial.write("Device disconnected. Returning.\n");


    return true;
}
