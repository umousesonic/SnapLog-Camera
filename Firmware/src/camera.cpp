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
    // poweron();
    // delay(1000);
    camera_config_t config;
    config.ledc_channel =   LEDC_CHANNEL_0;
    config.ledc_timer =     LEDC_TIMER_0;
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
    config.jpeg_quality = 10;    // Highest possible quality
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

    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 0);     // -2 to 2
    s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
    s->set_aec2(s, 0);           // 0 = disable , 1 = enable
    s->set_ae_level(s, 0);       // -2 to 2
    s->set_aec_value(s, 600);    // 0 to 1200
    s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);       // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
    s->set_bpc(s, 0);            // 0 = disable , 1 = enable
    s->set_wpc(s, 1);            // 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
    s->set_lenc(s, 1);           // 0 = disable , 1 = enable
    s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
    s->set_vflip(s, 0);          // 0 = disable , 1 = enable
    s->set_dcw(s, 1);            // 0 = disable , 1 = enable
    s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
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

void Camera::poweroff() {
    // gpio_set_level((gpio_num_t)CameraPin::PWDN_GPIO_NUM, 1);
    if (CameraPin::PWDN_GPIO_NUM < 0) return;
    digitalWrite(CameraPin::PWDN_GPIO_NUM, HIGH);
    gpio_hold_en((gpio_num_t)CameraPin::PWDN_GPIO_NUM);
}

void Camera::poweron() {
    if (CameraPin::PWDN_GPIO_NUM < 0) return;
    gpio_hold_dis((gpio_num_t)CameraPin::PWDN_GPIO_NUM);
    digitalWrite(CameraPin::PWDN_GPIO_NUM, LOW);
    // digitalWrite(CameraPin::RESET_GPIO_NUM, LOW);
    // delay(20);
    // digitalWrite(CameraPin::RESET_GPIO_NUM, HIGH);
}
