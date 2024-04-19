#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "camera.h"
#include "bleserver.h"
#include <esp_bt.h>
#include "driver/adc.h"
#include "global.h"

// #define SLEEP_TIME 10             // seconds
#define uS_TO_S_FACTOR 1000000  
// put declarations here:
Camera *cam;
RTC_DATA_ATTR int bootCount = 0;
uint32_t wake_interval = 20; 



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // delay(1000); //Take some time to open up the Serial Monitor


  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason){
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); 
    esp_wifi_bt_power_domain_on();
    cam->poweron();
    cam = new Camera();
    cam->capture(process_function);
    break;
  }

  esp_sleep_enable_timer_wakeup(wake_interval * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(wake_interval) + " Seconds");
  Serial.println("Going to sleep now");
  Serial.flush(); 
  cam->poweroff();
  esp_wifi_bt_power_domain_off();
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
  esp_deep_sleep_start();
  
}

void loop() {// This code will never run  
}