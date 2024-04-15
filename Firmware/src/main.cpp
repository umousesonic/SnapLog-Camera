#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "camera.h"
#include "bleserver.h"

#define SLEEP_TIME 20             // seconds
#define uS_TO_S_FACTOR 1000000  
// put declarations here:
Camera *cam;
RTC_DATA_ATTR int bootCount = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // delay(1000); //Take some time to open up the Serial Monitor
  
  cam = new Camera();
  cam->capture(process_function);

  // ++bootCount;
  // Serial.println("Boot number: " + String(bootCount));

  // esp_sleep_wakeup_cause_t wakeup_reason;
  // wakeup_reason = esp_sleep_get_wakeup_cause();
  // switch(wakeup_reason){
  //   case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); 
  //   cam = new Camera();
  //   // cam->capture(TODO);
  //   break;
  //   default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  // }

  // esp_sleep_enable_timer_wakeup(SLEEP_TIME * uS_TO_S_FACTOR);
  // Serial.println("Setup ESP32 to sleep for every " + String(SLEEP_TIME) + " Seconds");
}

void loop() {// This code will never run  
}