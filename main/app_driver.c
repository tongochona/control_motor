#include <stdio.h>
#include "esp_log.h"


#include "motor_driver.h"

#define TAG "app_driver"

void app_driver_init(void)
{
    motor_config_t motor_config = {
        .pwm_pin = 18,          // PWM output pin
        .forward_pin = 19,     // Forward direction control pin
        .backward_pin = 21     // Backward direction control pin
    };
    motor_driver_init(&motor_config);
    ESP_LOGI(TAG, "Motor driver initialized");

}

esp_err_t app_driver_motor_set_speed(uint8_t speed)
{
    return motor_set_speed(speed);
}
esp_err_t app_driver_motor_set_direction(bool direction)
{
    return motor_set_direction(direction);
}