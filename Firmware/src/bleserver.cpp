#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Arduino.h>

#include "bleserver.h"

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic, *pRxCharacteristic;
bool deviceConnected = false;
static uint16_t pkt_id = 0x0000; // Initialize to 0
static datapkt_t packet;

// https://www.uuidgenerator.net/version7
#define SERVICE_UUID           "018ed48a-a65c-718d-a58e-dae287ec41fd"  
#define CHARACTERISTIC_UUID_TX "018ed48c-1ea0-73d5-ab67-af89e0d89179"
#define CHARACTERISTIC_UUID_RX "018ed48d-2f98-76de-bf56-ef87e9d89281"

// BLE callback: detect when nodes connect and disconnect from a device, 
// and when they get subscribed and read data
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
      deviceConnected = true;
      pkt_id = 0x0000; // Reset ack on new connection
    };

    void onDisconnect(BLEServer* pServer) override {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        if (pCharacteristic == pRxCharacteristic) {
            uint16_t *rxValue = (uint16_t*)pCharacteristic->getData();
            if (rxValue != NULL) {
                pkt_id = *(rxValue) + 1;
                // pkt_id = (rxValue[0]<<8 | rxValue[1]) + 1;
                // Serial.write("Setting tx to packet ");
                // Serial.write(std::to_string(pkt_id).c_str());
                // Serial.write("\n");
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

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();
}

bool process_function(size_t width, size_t height, pixformat_t format, uint8_t* buf, size_t len) {
    // Init server
    if (!pServer) {
        initializeBLE();
    }

    

    unsigned long startTime = millis();
    while (!deviceConnected) {
        if (millis() - startTime > 60000) { // 10 seconds timeout
            Serial.println("Timeout waiting for client ack.");
            return false; // go back to sleep
        }
    }

    // Conneted
    uint16_t totalPackets = (len / PACKET_MAX_DATA) + (len % PACKET_MAX_DATA > 0 ? 1 : 0);
    Serial.write("Total packets=");
    Serial.write(std::to_string(totalPackets).c_str());
    Serial.write("\n");
    
    while(deviceConnected) {
        if (pkt_id != totalPackets) {

            packet.pktsize = PACKET_MAX_DATA + 12;
            packet.data_size = PACKET_MAX_DATA;
            packet.id = pkt_id;
            packet.total = totalPackets;
            // packet.pktdata = (uint8_t (*)[PACKET_MAX_DATA]);
            memcpy(packet.pktdata, (uint8_t*)(buf + pkt_id*PACKET_MAX_DATA), packet.data_size);

            
        }
        else {
            packet.data_size = len - (pkt_id-1)*PACKET_MAX_DATA;
            packet.pktsize = packet.data_size + PACKET_MAX_DATA + 12 - PACKET_MAX_DATA;
            packet.id = pkt_id;
            packet.total = totalPackets;
            // packet.pktdata = (uint8_t (*)[PACKET_MAX_DATA])(buf + pkt_id*PACKET_MAX_DATA);
            memcpy(packet.pktdata, (uint8_t*)(buf + pkt_id*PACKET_MAX_DATA), packet.data_size);
        }
        
        pTxCharacteristic->setValue((uint8_t*)&packet, packet.pktsize);
        pTxCharacteristic->notify();
        // delay(10); // Prevent BLE congestion
    }
    Serial.write("Device disconnected. Returning.\n");


    return true;
}

