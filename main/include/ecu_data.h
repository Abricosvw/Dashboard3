#ifndef ECU_DATA_H
#define ECU_DATA_H

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

// ECU Data structure matching React types
typedef struct {
    float map_pressure;          // MAP Pressure in kPa (100-250)
    float wastegate_position;    // Wastegate position in % (0-100)
    float tps_position;          // TPS position in % (0-100)
    float engine_rpm;            // Engine RPM (0-7000)
    float target_boost;          // Target boost in kPa (100-250)
    bool tcu_protection_active;  // TCU protection active
    bool tcu_limp_mode;         // TCU limp mode
    float torque_request;        // Torque request in %
    uint64_t timestamp;          // Timestamp in milliseconds
} ecu_data_t;

// System settings
typedef struct {
    float max_boost_limit;       // Maximum boost limit
    float max_rpm_limit;         // Maximum RPM limit
    bool audio_alerts_enabled;   // Audio alerts enabled
    char ecu_address[32];        // ECU address
} system_settings_t;

// Connection status
typedef struct {
    bool connected;
    char message[128];
} connection_status_t;

// Data stream entry
typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_SUCCESS,
    LOG_ERROR
} log_type_t;

typedef struct {
    uint64_t timestamp;
    char message[256];
    log_type_t type;
} data_stream_entry_t;

// Function prototypes
void ecu_data_init(void);
void ecu_data_update(ecu_data_t *data);
ecu_data_t* ecu_data_get(void);
char* ecu_data_to_json(const ecu_data_t *data);
bool ecu_data_from_json(const char *json_str, ecu_data_t *data);
void ecu_data_simulate(ecu_data_t *data);

// System settings functions
void system_settings_init(void);
system_settings_t* system_settings_get(void);
void system_settings_save(const system_settings_t *settings);

// Logging functions
void data_stream_add_entry(const char *message, log_type_t type);
void data_stream_clear(void);
char* data_stream_to_json(void);

// Simple data functions for WiFi server
char* ecu_data_to_string(const ecu_data_t *data);
char* data_stream_to_string(void);

#ifdef __cplusplus
}
#endif

#endif // ECU_DATA_H
