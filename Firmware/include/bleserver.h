// #ifndef BLESERVER_H
// #define BLESERVER_H
#include "datapkt.h"
#include "esp_camera.h"

static void initializeBLE(const char* deviceName);
bool process_function(size_t width, size_t height, pixformat_t format, uint8_t* buf, size_t len);


// #endif // BLESERVER_H


