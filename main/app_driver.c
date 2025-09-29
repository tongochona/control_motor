#include <stdio.h>
#include "esp_log.h"

#include "app_driver.h"
#include "motor_driver.h"
#include "encoder_driver.h"

#define TAG "app_driver"

static ky040_handle_t s_enc1 = NULL;
static ky040_handle_t s_enc2 = NULL;

void app_driver_init(void)
{
    motor_config_t motor_config = {
        .pwm_pin = MOTOR_PWM_PIN,          // PWM output pin
        .forward_pin = MOTOR_FORWARD_PIN,     // Forward direction control pin
        .backward_pin = MOTOR_BACKWARD_PIN,     // Backward direction control pin
    };

    motor_driver_init(&motor_config);

    ESP_ERROR_CHECK(ky040_install_isr_service_once(0));
    ky040_config_t c1 = { ENC1_CLK_GPIO, ENC1_DT_GPIO, ENC1_SW_GPIO, ENC1_REVERSE_DIR, ENCODER_DEBOUNCE_US, ANGLE_MIN, ANGLE_MAX };
    ky040_config_t c2 = { ENC2_CLK_GPIO, ENC2_DT_GPIO, ENC2_SW_GPIO, ENC2_REVERSE_DIR, ENCODER_DEBOUNCE_US, ANGLE_MIN, ANGLE_MAX };
    ESP_ERROR_CHECK( ky040_create(&c1, &s_enc1) );
    ESP_ERROR_CHECK( ky040_create(&c2, &s_enc2) );

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

esp_err_t app_driver_motor_stop(void)
{
    return motor_stop();
}

uint16_t app_driver_encoder_get_count(int encoder)
{
    if (encoder == DESIRED_ANGLE){
        return ky040_get_angle(s_enc1);
    }
    return ky040_get_angle(s_enc2);
}