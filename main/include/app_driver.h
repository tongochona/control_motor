#ifndef __APP_DRIVER_H__
#define __APP_DRIVER_H__

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif 

void app_driver_init(void);

esp_err_t app_driver_motor_set_speed(uint8_t speed);
esp_err_t app_driver_motor_set_direction(bool direction);

#ifdef __cplusplus
}
#endif
#endif // __APP_DRIVER_H__