#include <stdio.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
// #include "driver/i2c.h"
#include "ssd1306.h"
#include <string.h>

#include "app_driver.h"

#define I2C_SDA_GPIO 4
#define I2C_SCL_GPIO 5
#define I2C_RESET_GPIO -1   // Nếu không có chân RESET thì để -1
#define LOG_MSG_LEN 128

#define TAG "app_main"

typedef struct
{
    uint8_t current;
    uint8_t desired;
} angle_data_t;

typedef struct
{
    uint16_t speed;
    bool direction; // true for forward, false for backward
} motor_command_t;

typedef struct
{
    const char *name_task;
    QueueHandle_t queue_handle;
} task_info_t;

typedef struct{
    char msg[LOG_MSG_LEN];
} log_message_t;

QueueHandle_t xQueueLog;
QueueHandle_t xQueueDisplay_handle;

void vTaskProcessed(void *pvParameters);
void vTaskControlMotor(void *pvParameters);
void vTaskErrorHandle(void *pvParameters);
void vTaskSendDesiredAngle(void *pvParameters);
void vTaskSendCurrentAngle(void *pvParameters);
void vTaskDisplay(void *pvParameters);
void vTaskDebug(void *pvParameters);
//void vTaskOled(void *pvParameters);

TaskHandle_t xTaskErrorHandle_handle = NULL;

QueueHandle_t xQueueControl_handle = NULL;
QueueHandle_t xQueueFeedback_handle = NULL;
QueueHandle_t xQueueSpeed_handle = NULL;
QueueHandle_t xQueueError_handle = NULL;
QueueHandle_t xQueueDisplay_handle = NULL;

void app_main(void)
{

    ESP_LOGI(TAG, "Application driver initialization");
    app_driver_init();

    xQueueControl_handle = xQueueCreate(3, sizeof(uint8_t));
    xQueueFeedback_handle = xQueueCreate(3, sizeof(uint8_t));
    xQueueSpeed_handle = xQueueCreate(3, sizeof(motor_command_t));
    xQueueError_handle = xQueueCreate(2, sizeof(task_info_t));
    xQueueDisplay_handle = xQueueCreate(3, sizeof(angle_data_t));
    xQueueLog = xQueueCreate(10, sizeof(log_message_t));
    //xQueueOled = xQueueCreate(3, sizeof(angle_data_t));

    xTaskCreate(vTaskSendDesiredAngle, "Task Send Desired Angle", 2048, &xQueueControl_handle, 4, NULL);
    xTaskCreate(vTaskSendCurrentAngle, "Task Send Current Angle", 2048, &xQueueFeedback_handle, 5, NULL);
    xTaskCreate(vTaskProcessed, "Task Processed", 2048, NULL, 6, NULL);
    xTaskCreate(vTaskControlMotor, "Task Control Motor", 2048, NULL, 4, NULL);
    xTaskCreate(vTaskErrorHandle, "Task Error Handle", 2048, NULL, 1, &xTaskErrorHandle_handle);
    xTaskCreate(vTaskDisplay, "Task Display", 2048, NULL, 3, NULL);
    xTaskCreate(vTaskDebug, "uart_debug_task", 2048, NULL, 3, NULL);
    //xTaskCreate(vTaskOled, "oled_task", 4096, NULL, 3, NULL);
}

// void vTaskSendAngle(void *pvParameters)
// {

//     QueueHandle_t xQueueAngle_handle = *(QueueHandle_t *)pvParameters;
//     uint32_t angle = 0;
//     char* task_name = pcTaskGetName(NULL);
//     printf("Task Name: %s\n", task_name); // Example usage of task_name
//     while (1)
//     {
//         // Simulate sending desired and current angles
//         angle = rand() % 360; // Random angle between 0 and 359
//         // angle = (angle + 10) % 360; // Example angle increment
//         if (xQueueSend(xQueueAngle_handle, &angle, 0) != pdPASS)
//         {
//             // Handle error: queue full
//             task_info_t err = {
//                 .name_task = pcTaskGetName(NULL),
//                 .queue_handle = xQueueAngle_handle};
//             xQueueSend(xQueueError_handle, &err, 0);
//             vTaskPrioritySet(xTaskErrorHandle_handle, 7); // Increase priority of error handling task
//         }

//         vTaskDelay(pdMS_TO_TICKS(1000)); // Send angle every second
//     }
// }

void vTaskSendDesiredAngle(void *pvParameters)
{

    QueueHandle_t xQueueAngle_handle = *(QueueHandle_t *)pvParameters;
    uint32_t angle = 0;
    char* task_name = pcTaskGetName(NULL);
    printf("Task Name: %s\n", task_name); // Example usage of task_name
    while (1)
    {
        // Simulate sending desired and current angles
        angle = 30; // Random angle between 0 and 359
        // angle = (angle + 10) % 360; // Example angle increment
        if (xQueueSend(xQueueAngle_handle, &angle, 0) != pdPASS)
        {
            // Handle error: queue full
            task_info_t err = {
                .name_task = pcTaskGetName(NULL),
                .queue_handle = xQueueAngle_handle};
            xQueueSend(xQueueError_handle, &err, 0);
            vTaskPrioritySet(xTaskErrorHandle_handle, 7); // Increase priority of error handling task
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Send angle every second
    }
}

void vTaskSendCurrentAngle(void *pvParameters)
{

    QueueHandle_t xQueueAngle_handle = *(QueueHandle_t *)pvParameters;
    uint32_t angle = 0;
    char* task_name = pcTaskGetName(NULL);
    printf("Task Name: %s\n", task_name); // Example usage of task_name
    while (1)
    {
        // Simulate sending desired and current angles
        angle = 20; // Random angle between 0 and 359
        // angle = (angle + 10) % 360; // Example angle increment
        if (xQueueSend(xQueueAngle_handle, &angle, 0) != pdPASS)
        {
            // Handle error: queue full
            task_info_t err = {
                .name_task = pcTaskGetName(NULL),
                .queue_handle = xQueueAngle_handle};
            xQueueSend(xQueueError_handle, &err, 0);
            vTaskPrioritySet(xTaskErrorHandle_handle, 7); // Increase priority of error handling task
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Send angle every second
    }
}

void vTaskProcessed(void *pvParameters)
{
    uint8_t desired_angle_pre = 0;
    uint8_t current_angle_pre = 0;
    uint8_t desired_angle = 0;
    uint8_t current_angle = 0;
    uint16_t motor_speed = 0;
    angle_data_t angle_data;
    motor_command_t motor_command;
    while (1)
    {
        char* task_name = pcTaskGetName(NULL);
        
        if(xQueueReceive(xQueueControl_handle, &desired_angle, portMAX_DELAY) == pdPASS){
            ESP_LOGI(task_name, "Received Desired Angle: %d", desired_angle);
            desired_angle_pre = desired_angle;
        }else{
            desired_angle = desired_angle_pre;
        }
        if(xQueueReceive(xQueueFeedback_handle, &current_angle, portMAX_DELAY) == pdPASS){
            ESP_LOGI(task_name, "Received Current Angle: %d", current_angle);
            current_angle_pre = current_angle;
        }else{
            current_angle = current_angle_pre;
        }


        angle_data.current = current_angle;
        angle_data.desired = desired_angle;
        // Process control algorithm here (e.g., PID) to determine motor speed and direction
        motor_speed = (desired_angle > current_angle) ? (desired_angle - current_angle) : (current_angle - desired_angle);
        motor_command.speed = 1023;
        motor_command.direction = (desired_angle >= current_angle) ? true : false; // true for forward, false for backward
        // Send motor command to motor control task
        if (xQueueSend(xQueueSpeed_handle, &motor_command, 0) != pdPASS)
        {
            // Handle error: queue full
            task_info_t err = {
                .name_task = pcTaskGetName(NULL),
                .queue_handle = xQueueSpeed_handle};
            xQueueSend(xQueueError_handle, &err, 0);
            vTaskPrioritySet(xTaskErrorHandle_handle, 7); // Increase priority of error handling task
            printf("Failed to send motor command to queue\n");
        };
        if (xQueueSend(xQueueDisplay_handle, &angle_data, 0) != pdPASS)
        {
            // Handle error: queue full
            task_info_t err = {
                .name_task = pcTaskGetName(NULL),
                .queue_handle = xQueueFeedback_handle};
            xQueueSend(xQueueError_handle, &err, 0);
            vTaskPrioritySet(xTaskErrorHandle_handle, 7); // Increase priority of error handling task
            printf("Failed to send angle data to queue\n");
        }
    }
}

void vTaskControlMotor(void *pvParameters)
{
    motor_command_t motor_command;
    while (1)
    {
        if (xQueueReceive(xQueueSpeed_handle, &motor_command, portMAX_DELAY) == pdPASS)
        {
            app_driver_motor_set_direction(motor_command.direction);
            app_driver_motor_set_speed(motor_command.speed);
            ESP_LOGI("Task Control Motor", "Motor Speed: %d, Direction: %s", motor_command.speed, motor_command.direction ? "Forward" : "Backward");
        }
    }
}

void vTaskErrorHandle(void *pvParameters)
{
    task_info_t task_infor;

    while (1)
    {
        if (xQueueReceive(xQueueError_handle, &task_infor, portMAX_DELAY) == pdPASS)
        {
            if (task_infor.name_task != NULL)
            {
                // Handle the error (e.g., log it, reset the queue, notify user)
                ESP_LOGE(TAG, "Error in queue handling");
                ESP_LOGE(TAG, "Task Name: %s", task_infor.name_task);

                // Reset the queue or take appropriate action
                xQueueReset(task_infor.queue_handle);
                // After handling the error, lower the task priority back to normal
                vTaskPrioritySet(NULL, 1); // Assuming 1 is the normal priority
            }
        }
    }
}


// DEBUG TASK
void vTaskDebug(void *pvParameters)
{
    log_message_t log_msg;
    while(1)
    {
        if(xQueueReceive(xQueueLog, &log_msg, portMAX_DELAY) == pdTRUE)
        {
            printf("[DEBUG] %s\n", log_msg.msg);
        }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
}

// OLED TASK
void vTaskDisplay(void *pvParameters)
{
    ESP_LOGI(TAG, "OLED Task started");
    
    // Lấy device OLED từ app_driver (đã được khởi tạo sẵn)
    SSD1306_t *dev = app_driver_get_oled_device();
    
    // Clear màn hình và hiển thị thông báo
    ssd1306_clear_screen(dev, false);
    ssd1306_display_text(dev, 0, "System Ready", 12, false);
    
    angle_data_t angle_data;
    char buffer[32];

    while(1)
    {
        if(xQueueReceive(xQueueDisplay_handle, &angle_data, portMAX_DELAY) == pdTRUE)
        {
            //ssd1306_clear_screen(&dev, false);
            snprintf(buffer, sizeof(buffer), "Current: %d", angle_data.current);
            ssd1306_display_text(dev, 0, buffer, strlen(buffer), false);

            ssd1306_clear_line(dev, 1, false);
            snprintf(buffer, sizeof(buffer), "Desired: %d", angle_data.desired);
            ssd1306_display_text(dev, 1, buffer, strlen(buffer), false);
        }
        taskYIELD();
    }
}

