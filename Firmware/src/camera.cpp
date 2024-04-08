#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "camera_pin.h"
#include "camera.h"


Camera::Camera() {
    init();
}


void Camera::init() {
    camera_config_t config;
    // config.ledc_channel =   CameraPin::LEDC_CHANNEL_0;
    // config.ledc_timer =     CameraPin::LEDC_TIMER_0;
    config.pin_d0 =         CameraPin::Y2_GPIO_NUM;
    config.pin_d1 =         CameraPin::Y3_GPIO_NUM;
    config.pin_d2 =         CameraPin::Y4_GPIO_NUM;
    config.pin_d3 =         CameraPin::Y5_GPIO_NUM;
    config.pin_d4 =         CameraPin::Y6_GPIO_NUM;
    config.pin_d5 =         CameraPin::Y7_GPIO_NUM;
    config.pin_d6 =         CameraPin::Y8_GPIO_NUM;
    config.pin_d7 =         CameraPin::Y9_GPIO_NUM;
    config.pin_xclk =       CameraPin::XCLK_GPIO_NUM;
    config.pin_pclk =       CameraPin::PCLK_GPIO_NUM;
    config.pin_vsync =      CameraPin::VSYNC_GPIO_NUM;
    config.pin_href =       CameraPin::HREF_GPIO_NUM;
    config.pin_sccb_sda =   CameraPin::SIOD_GPIO_NUM;
    config.pin_sccb_scl =   CameraPin::SIOC_GPIO_NUM;
    config.pin_pwdn =       CameraPin::PWDN_GPIO_NUM;
    config.pin_reset =      CameraPin::RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;

    // Set picture quality and buffer mode
    config.jpeg_quality = 4;    // Highest possible quality
    config.fb_count = 1;    // No need for higher frame rate, this is fine and will cost less cpu
    config.grab_mode = CAMERA_GRAB_LATEST;

    // Begin init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    // Get sensor
    s = esp_camera_sensor_get();
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1); // flip it back
        s->set_brightness(s, 1); // up the brightness just a bit
        s->set_saturation(s, -2); // lower the saturation
    } else if (s->id.PID == OV2640_PID) {
        s->set_vflip(s, 1); //flip it back
    } else if (s->id.PID == GC0308_PID) {
        s->set_hmirror(s, 0);
    } else if (s->id.PID == GC032A_PID) {
        s->set_vflip(s, 1);
    }
    // s->set_vflip(s, 1);
    // s->set_hmirror(s, 1);
}

sensor_t* Camera::getSensor() {
    return s;
}

bool Camera::capture(bool (&process_function)(size_t, size_t, pixformat_t, uint8_t*, size_t) ) {
    // acquire a frame
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) { return false; }
    bool result = process_function(fb->width, fb->height, fb->format, fb->buf, fb->len);
    
    // release buffer
    esp_camera_fb_return(fb);
    return result;
}
