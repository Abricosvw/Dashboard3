#include "include/canbus.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include "include/web_server.h"
#include "include/can_websocket.h"
#include "lvgl.h"
#include "ui/screens/ui_Screen3.h"

static const char *CAN_TAG = "CANBUS";

// CAN bus configuration
static twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
static twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
static twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

static bool canbus_initialized = false;
static bool canbus_running = false;

esp_err_t canbus_init(void)
{
    if (canbus_initialized) {
        ESP_LOGW(CAN_TAG, "CAN bus already initialized");
        return ESP_OK;
    }
    
    // Install TWAI driver
    esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
    if (ret != ESP_OK) {
        ESP_LOGE(CAN_TAG, "Failed to install TWAI driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    canbus_initialized = true;
    ESP_LOGI(CAN_TAG, "CAN bus initialized successfully");
    return ESP_OK;
}

esp_err_t canbus_start(void)
{
    if (!canbus_initialized) {
        ESP_LOGE(CAN_TAG, "CAN bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (canbus_running) {
        ESP_LOGW(CAN_TAG, "CAN bus already running");
        return ESP_OK;
    }
    
    esp_err_t ret = twai_start();
    if (ret != ESP_OK) {
        ESP_LOGE(CAN_TAG, "Failed to start TWAI: %s", esp_err_to_name(ret));
        return ret;
    }
    
    canbus_running = true;
    ESP_LOGI(CAN_TAG, "CAN bus started successfully");
    return ESP_OK;
}

esp_err_t canbus_stop(void)
{
    if (!canbus_running) {
        ESP_LOGW(CAN_TAG, "CAN bus not running");
        return ESP_OK;
    }
    
    esp_err_t ret = twai_stop();
    if (ret != ESP_OK) {
        ESP_LOGE(CAN_TAG, "Failed to stop TWAI: %s", esp_err_to_name(ret));
        return ret;
    }
    
    canbus_running = false;
    ESP_LOGI(CAN_TAG, "CAN bus stopped");
    return ESP_OK;
}

esp_err_t canbus_send_message(const can_message_t *message)
{
    if (!canbus_running) {
        ESP_LOGE(CAN_TAG, "CAN bus not running");
        return ESP_ERR_INVALID_STATE;
    }
    
    twai_message_t twai_msg = {0};
    twai_msg.identifier = message->id;
    twai_msg.data_length_code = message->dlc;
    twai_msg.rtr = message->rtr;
    twai_msg.extd = message->ext_id;
    memcpy(twai_msg.data, message->data, message->dlc);
    
    esp_err_t ret = twai_transmit(&twai_msg, pdMS_TO_TICKS(100));
    if (ret != ESP_OK) {
        ESP_LOGW(CAN_TAG, "Failed to transmit message: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t canbus_receive_message(can_message_t *message, uint32_t timeout_ms)
{
    if (!canbus_running) {
        ESP_LOGE(CAN_TAG, "CAN bus not running");
        return ESP_ERR_INVALID_STATE;
    }
    
    twai_message_t twai_msg;
    esp_err_t ret = twai_receive(&twai_msg, pdMS_TO_TICKS(timeout_ms));
    
    if (ret == ESP_OK) {
        message->id = twai_msg.identifier;
        message->dlc = twai_msg.data_length_code;
        message->rtr = twai_msg.rtr;
        message->ext_id = twai_msg.extd;
        memcpy(message->data, twai_msg.data, twai_msg.data_length_code);
    }
    
    return ret;
}

void canbus_parse_ecu_data(const can_message_t *message, ecu_data_t *ecu_data)
{
    if (!message || !ecu_data) return;

    // Validate message before parsing
    if (!canbus_validate_message_data(message)) {
        ESP_LOGW(CAN_TAG, "Invalid CAN message data for ID: 0x%03X", message->id);
        return;
    }

    float parsed_value = 0.0f;

    switch (message->id) {
        case CAN_ID_ENGINE_RPM:
            // Parse RPM from CAN data (example: 2 bytes, 0.25 RPM/bit)
            parsed_value = canbus_get_uint16(message->data, 0) * 0.25f;
            if (canbus_validate_engine_rpm(parsed_value)) {
                ecu_data->engine_rpm = parsed_value;
                ESP_LOGD(CAN_TAG, "Received RPM: %.1f", ecu_data->engine_rpm);
            } else {
                ESP_LOGW(CAN_TAG, "Invalid RPM value: %.1f", parsed_value);
            }
            break;

        case CAN_ID_MAP_PRESSURE:
            // Parse MAP pressure (example: 2 bytes, 0.1 kPa/bit, offset 100)
            parsed_value = canbus_get_uint16(message->data, 0) * 0.1f + 100.0f;
            if (canbus_validate_map_pressure(parsed_value)) {
                ecu_data->map_pressure = parsed_value;
                ESP_LOGD(CAN_TAG, "Received MAP: %.1f kPa", ecu_data->map_pressure);
            } else {
                ESP_LOGW(CAN_TAG, "Invalid MAP pressure: %.1f kPa", parsed_value);
            }
            break;

        case CAN_ID_TPS_POSITION:
            // Parse TPS position (example: 1 byte, 0.4%/bit)
            parsed_value = message->data[0] * 0.4f;
            if (canbus_validate_tps_position(parsed_value)) {
                ecu_data->tps_position = parsed_value;
                ESP_LOGD(CAN_TAG, "Received TPS: %.1f%%", ecu_data->tps_position);
            } else {
                ESP_LOGW(CAN_TAG, "Invalid TPS position: %.1f%%", parsed_value);
            }
            break;

        case CAN_ID_WASTEGATE_POS:
            // Parse wastegate position (example: 1 byte, 0.4%/bit)
            parsed_value = message->data[0] * 0.4f;
            if (canbus_validate_wastegate_position(parsed_value)) {
                ecu_data->wastegate_position = parsed_value;
                ESP_LOGD(CAN_TAG, "Received Wastegate: %.1f%%", ecu_data->wastegate_position);
            } else {
                ESP_LOGW(CAN_TAG, "Invalid wastegate position: %.1f%%", parsed_value);
            }
            break;

        case CAN_ID_TARGET_BOOST:
            // Parse target boost (example: 2 bytes, 0.1 kPa/bit, offset 100)
            parsed_value = canbus_get_uint16(message->data, 0) * 0.1f + 100.0f;
            if (canbus_validate_target_boost(parsed_value)) {
                ecu_data->target_boost = parsed_value;
                ESP_LOGD(CAN_TAG, "Received Target Boost: %.1f kPa", ecu_data->target_boost);
            } else {
                ESP_LOGW(CAN_TAG, "Invalid target boost: %.1f kPa", parsed_value);
            }
            break;

        case CAN_ID_TCU_STATUS:
            // Parse TCU status (example: bits in first byte)
            ecu_data->tcu_protection_active = (message->data[0] & 0x01) != 0;
            ecu_data->tcu_limp_mode = (message->data[0] & 0x02) != 0;
            parsed_value = message->data[1] * 0.4f; // Second byte

            if (canbus_validate_torque_request(parsed_value)) {
                ecu_data->torque_request = parsed_value;
                ESP_LOGD(CAN_TAG, "Received TCU Status: Protection=%d, Limp=%d, Torque=%.1f%%",
                         ecu_data->tcu_protection_active, ecu_data->tcu_limp_mode, ecu_data->torque_request);
            } else {
                ESP_LOGW(CAN_TAG, "Invalid torque request: %.1f%%", parsed_value);
            }
            break;

        default:
            ESP_LOGD(CAN_TAG, "Unknown CAN ID: 0x%03X", message->id);
            break;
    }
}

void canbus_task(void *pvParameters)
{
    ESP_LOGI(CAN_TAG, "CAN bus task started");

    can_message_t message;
    ecu_data_t *ecu_data = ecu_data_get();
    uint32_t consecutive_errors = 0;
    uint32_t message_count = 0;
    uint32_t last_message_time = xTaskGetTickCount();

    while (1) {
        esp_err_t ret = canbus_receive_message(&message, 100); // 100ms timeout

        if (ret == ESP_OK) {
            consecutive_errors = 0; // Reset error counter on successful receive
            message_count++;
            last_message_time = xTaskGetTickCount();

            // Parse received CAN message and update ECU data
            canbus_parse_ecu_data(&message, ecu_data);

            // Update global ECU data
            ecu_data_update(ecu_data);

            // Update WebSocket data for broadcast
            uint8_t tcu_status = 0; // 0=OK, 1=WARN, 2=ERROR
            if (ecu_data->tcu_limp_mode) {
                tcu_status = 2;
            } else if (ecu_data->tcu_protection_active) {
                tcu_status = 1;
            }

            // Update WebSocket CAN data
            update_websocket_can_data(
                (uint16_t)ecu_data->engine_rpm,
                (uint16_t)(ecu_data->map_pressure * 100), // Convert to fixed point
                (uint8_t)ecu_data->tps_position,
                (uint8_t)ecu_data->wastegate_position,
                (uint16_t)(ecu_data->target_boost * 100), // Convert to fixed point
                tcu_status
            );

            // Send raw CAN message to Screen3 sniffer
            ui_process_real_can_message(message.id, message.data, message.dlc);

            // Log data stream entry for important messages (less frequently)
            if (message_count % 50 == 0) {
                if (message.id == CAN_ID_ENGINE_RPM || message.id == CAN_ID_MAP_PRESSURE) {
                    char log_msg[64];
                    snprintf(log_msg, sizeof(log_msg), "CAN: ID=0x%03X, %d messages processed", message.id, message_count);
                    data_stream_add_entry(log_msg, LOG_INFO);
                }
            }
        } else if (ret == ESP_ERR_TIMEOUT) {
            // Timeout is normal, but check if we haven't received data for too long
            uint32_t current_time = xTaskGetTickCount();
            if ((current_time - last_message_time) > pdMS_TO_TICKS(5000)) { // 5 seconds
                ESP_LOGW(CAN_TAG, "No CAN messages received for 5 seconds");
                data_stream_add_entry("No CAN data received - check ECU connection", LOG_WARNING);
                last_message_time = current_time; // Reset to avoid spam
            }
            continue;
        } else {
            consecutive_errors++;
            ESP_LOGW(CAN_TAG, "CAN receive error: %s (consecutive: %d)", esp_err_to_name(ret), consecutive_errors);
            data_stream_add_entry("CAN bus communication error", LOG_WARNING);

            // Try to recover based on error type and consecutive errors
            if (consecutive_errors >= 5) {
                ESP_LOGE(CAN_TAG, "Too many consecutive CAN errors, attempting recovery");

                // Stop and restart CAN bus
                canbus_stop();
                vTaskDelay(pdMS_TO_TICKS(1000)); // Wait 1 second

                esp_err_t restart_ret = canbus_start();
                if (restart_ret == ESP_OK) {
                    ESP_LOGI(CAN_TAG, "CAN bus restarted successfully");
                    data_stream_add_entry("CAN bus recovered", LOG_SUCCESS);
                    consecutive_errors = 0;
                } else {
                    ESP_LOGE(CAN_TAG, "Failed to restart CAN bus: %s", esp_err_to_name(restart_ret));
                    data_stream_add_entry("CAN bus recovery failed", LOG_ERROR);
                    // Wait longer before next attempt
                    vTaskDelay(pdMS_TO_TICKS(5000));
                }
            } else {
                // Less aggressive recovery for fewer errors
                vTaskDelay(pdMS_TO_TICKS(500));
            }
        }

        // Small delay to prevent overwhelming the system
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Data validation functions
bool canbus_validate_engine_rpm(float rpm)
{
    return (rpm >= 0.0f && rpm <= 8000.0f);
}

bool canbus_validate_map_pressure(float pressure)
{
    return (pressure >= 0.0f && pressure <= 300.0f);
}

bool canbus_validate_tps_position(float position)
{
    return (position >= 0.0f && position <= 100.0f);
}

bool canbus_validate_wastegate_position(float position)
{
    return (position >= 0.0f && position <= 100.0f);
}

bool canbus_validate_target_boost(float pressure)
{
    return (pressure >= 0.0f && pressure <= 300.0f);
}

bool canbus_validate_torque_request(float torque)
{
    return (torque >= 0.0f && torque <= 100.0f);
}

bool canbus_validate_message_data(const can_message_t *message)
{
    if (!message || message->dlc > 8) {
        return false;
    }

    // Basic sanity checks
    switch (message->id) {
        case CAN_ID_ENGINE_RPM:
        case CAN_ID_MAP_PRESSURE:
        case CAN_ID_TPS_POSITION:
        case CAN_ID_WASTEGATE_POS:
        case CAN_ID_TARGET_BOOST:
        case CAN_ID_TCU_STATUS:
            return true; // These IDs are known and should be validated in parsing

        default:
            ESP_LOGD(CAN_TAG, "Unknown CAN ID: 0x%03X", message->id);
            return false;
    }
}

// Helper functions for data parsing
uint16_t canbus_get_uint16(const uint8_t *data, int offset)
{
    return (data[offset] << 8) | data[offset + 1];
}

uint32_t canbus_get_uint32(const uint8_t *data, int offset)
{
    return (data[offset] << 24) | (data[offset + 1] << 16) |
           (data[offset + 2] << 8) | data[offset + 3];
}

float canbus_get_float(const uint8_t *data, int offset)
{
    union {
        uint32_t u;
        float f;
    } converter;

    converter.u = canbus_get_uint32(data, offset);
    return converter.f;
}
