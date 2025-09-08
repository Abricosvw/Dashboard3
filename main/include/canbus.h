#ifndef CANBUS_H
#define CANBUS_H

#include "esp_err.h"
#include "driver/twai.h"
#include "include/ecu_data.h"

#ifdef __cplusplus
extern "C" {
#endif

// CAN bus configuration for ESP32-S3
#define CAN_TX_PIN GPIO_NUM_20  // TXD0 pin
#define CAN_RX_PIN GPIO_NUM_19  // RXD0 pin

// Standard ECU CAN IDs (example values)
#define CAN_ID_ENGINE_RPM     0x201
#define CAN_ID_MAP_PRESSURE   0x202
#define CAN_ID_TPS_POSITION   0x203
#define CAN_ID_WASTEGATE_POS  0x204
#define CAN_ID_TARGET_BOOST   0x205
#define CAN_ID_TCU_STATUS     0x206

// CAN message structure
typedef struct {
    uint32_t id;
    uint8_t data[8];
    uint8_t dlc;
    bool rtr;
    bool ext_id;
} can_message_t;

// Function prototypes
esp_err_t canbus_init(void);
esp_err_t canbus_start(void);
esp_err_t canbus_stop(void);
esp_err_t canbus_send_message(const can_message_t *message);
esp_err_t canbus_receive_message(can_message_t *message, uint32_t timeout_ms);
void canbus_parse_ecu_data(const can_message_t *message, ecu_data_t *ecu_data);
void canbus_task(void *pvParameters);

// Data validation functions
bool canbus_validate_engine_rpm(float rpm);
bool canbus_validate_map_pressure(float pressure);
bool canbus_validate_tps_position(float position);
bool canbus_validate_wastegate_position(float position);
bool canbus_validate_target_boost(float pressure);
bool canbus_validate_torque_request(float torque);
bool canbus_validate_message_data(const can_message_t *message);

// Helper functions
uint16_t canbus_get_uint16(const uint8_t *data, int offset);
uint32_t canbus_get_uint32(const uint8_t *data, int offset);
float canbus_get_float(const uint8_t *data, int offset);

#ifdef __cplusplus
}
#endif

#endif // CANBUS_H
