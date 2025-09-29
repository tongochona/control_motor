#include <stdio.h>
#include "esp_log.h"

#include "app_driver.h"
#include "motor_driver.h"
#include "encoder_driver.h"

#include "driver/i2c.h"
#include "ssd1306.h"

#define TAG "app_driver"


static ky040_handle_t s_enc1 = NULL;
static ky040_handle_t s_enc2 = NULL;

// Biến toàn cục cho device OLED
static SSD1306_t oled_dev;
static i2c_master_bus_handle_t i2c_bus_handle = NULL;

void app_driver_init(void)
{
    motor_config_t motor_config = {
        .pwm_pin = MOTOR_PWM_PIN,          // PWM output pin
        .forward_pin = MOTOR_FORWARD_PIN,     // Forward direction control pin
        .backward_pin = MOTOR_BACKWARD_PIN,     // Backward direction control pin
    };
//123
//tesst
    motor_driver_init(&motor_config);

    ESP_ERROR_CHECK(ky040_install_isr_service_once(0));
    ky040_config_t c1 = { ENC1_CLK_GPIO, ENC1_DT_GPIO, ENC1_SW_GPIO, ENC1_REVERSE_DIR, ENCODER_DEBOUNCE_US, ANGLE_MIN, ANGLE_MAX };
    ky040_config_t c2 = { ENC2_CLK_GPIO, ENC2_DT_GPIO, ENC2_SW_GPIO, ENC2_REVERSE_DIR, ENCODER_DEBOUNCE_US, ANGLE_MIN, ANGLE_MAX };
    ESP_ERROR_CHECK( ky040_create(&c1, &s_enc1) );
    ESP_ERROR_CHECK( ky040_create(&c2, &s_enc2) );

    ESP_LOGI(TAG, "Motor driver initialized");
    // Khởi tạo I2C cho OLED
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_PORT,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_handle));
    ESP_LOGI(TAG, "I2C master bus initialized (new driver)");

    #if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 0))
    // Cấu hình cho ESP-IDF >= 5.2
    oled_dev._i2c_bus_handle = i2c_bus_handle;
    
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x3C,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_config, &oled_dev._i2c_dev_handle));
#endif

    // Khởi tạo OLED
    ssd1306_init(&oled_dev, 128, 64);
    
    // Clear màn hình và hiển thị thông báo
    ssd1306_clear_screen(&oled_dev, false);
    ssd1306_display_text(&oled_dev, 0, "Motor Control", 13, false);
    
    ESP_LOGI(TAG, "OLED initialized");

}

// Hàm getter để lấy device OLED
SSD1306_t* app_driver_get_oled_device(void)
{
    return &oled_dev;
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