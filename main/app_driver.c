#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

#include "motor_driver.h"
#include "ssd1306.h"
#include "app_driver.h"

#define TAG "app_driver"

// Cấu hình I2C cho OLED
#define I2C_MASTER_SCL_IO           22      // GPIO số cho SCL
#define I2C_MASTER_SDA_IO           21      // GPIO số cho SDA  
#define I2C_MASTER_FREQ_HZ          400000  // Tần số I2C
#define I2C_MASTER_PORT             I2C_NUM_0
#define OLED_I2C_ADDRESS            0x3C    // Địa chỉ I2C mặc định của OLED

// Biến toàn cục cho device OLED
static SSD1306_t oled_dev;

void app_driver_init(void)
{
    // Khởi tạo motor driver
    motor_config_t motor_config = {
        .pwm_pin = 5,          // PWM output pin
        .forward_pin = 6,     // Forward direction control pin
        .backward_pin = 7,     // Backward direction control pin
    };
    motor_driver_init(&motor_config);
    ESP_LOGI(TAG, "Motor driver initialized");

    // Khởi tạo I2C cho OLED
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_PORT, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_PORT, conf.mode, 0, 0, 0));
    
    // Cấu hình device OLED theo đúng định nghĩa struct
    oled_dev._address = OLED_I2C_ADDRESS;
    oled_dev._i2c_num = I2C_MASTER_PORT;  // SỬA THÀNH _i2c_num
    oled_dev._flip = false;
    
    // Khởi tạo OLED với 3 tham số đúng theo định nghĩa
    ssd1306_init(&oled_dev, 128, 64); // dev, width, height
    
    // Clear màn hình và hiển thị thông báo khởi tạo
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