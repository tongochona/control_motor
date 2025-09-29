#ifndef __APP_DRIVER_H__
#define __APP_DRIVER_H__

#include "esp_err.h"
#include "ssd1306.h"

// Cấu hình I2C cho OLED
#define I2C_MASTER_SCL_IO           2      // GPIO số cho SCL
#define I2C_MASTER_SDA_IO           3      // GPIO số cho SDA  
#define I2C_MASTER_FREQ_HZ          100000  // Tần số I2C
#define I2C_MASTER_PORT             I2C_NUM_0
#define OLED_I2C_ADDRESS            0x3C    // Địa chỉ I2C mặc định của OLED

#define MOTOR_PWM_PIN 4
#define MOTOR_FORWARD_PIN 5
#define MOTOR_BACKWARD_PIN 6

//--- Encoder #1 (phản hồi) ---
#define ENC1_CLK_GPIO           18
#define ENC1_DT_GPIO            19
#define ENC1_SW_GPIO            -1
#define ENC1_REVERSE_DIR        0   // 1 để đảo chiều cho encoder #1

// --- Encoder #2 (phản hồi) ---
#define ENC2_CLK_GPIO           7
#define ENC2_DT_GPIO            9
#define ENC2_SW_GPIO            -1
#define ENC2_REVERSE_DIR        0   // 1 để đảo chiều cho encoder #2

// Debounce cạnh CLK (us)
#define ENCODER_DEBOUNCE_US     1500
// Bắt sườn lên để giảm rung
#define CLK_INTR_TYPE           GPIO_INTR_POSEDGE

// ==== DẢI GÓC 0..90° (bước 1°) ====
#define ANGLE_MIN               0
#define ANGLE_MAX               90
#define ANGLE_SPAN              (ANGLE_MAX - ANGLE_MIN + 1)  // 91

// Độ dài queue theo phác thảo
#define Q_DEPTH  

enum encoder
{
    DESIRED_ANGLE = 0,
    CURRENT_ANGLE,

};


#ifdef __cplusplus
extern "C" {
#endif 

void app_driver_init(void);

esp_err_t app_driver_motor_set_speed(uint8_t speed);
esp_err_t app_driver_motor_set_direction(bool direction);
esp_err_t app_driver_motor_stop(void);

uint16_t app_driver_encoder_get_count(int);

SSD1306_t* app_driver_get_oled_device(void);


#ifdef __cplusplus
}
#endif
#endif // __APP_DRIVER_H__