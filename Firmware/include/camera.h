#include "esp_camera.h"

class Camera {
    public:
        Camera();
        void init();
        sensor_t* getSensor();
        bool capture(bool (&process_function)(size_t, size_t, pixformat_t, uint8_t*, size_t) );
        void poweroff();
    private:
        sensor_t *s;
        
};