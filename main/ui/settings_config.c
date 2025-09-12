// Settings Configuration Implementation
#include "settings_config.h"
#include <esp_log.h>
#include <nvs_flash.h>
#include "sd_card_manager.h"
#include <stdio.h>
#include <nvs.h>
#include <string.h>
#include <stdlib.h>
#include "../background_task.h"

static const char *TAG = "SETTINGS_CONFIG";
#define NVS_NAMESPACE "settings"
static touch_settings_t current_settings;

// Helper to serialize settings to a JSON string
static void settings_to_json(const touch_settings_t* settings, char* buffer, size_t buffer_size) {
    // A more robust implementation would use a proper JSON library
    snprintf(buffer, buffer_size,
             "{\"sensitivity\":%d,\"demo_mode\":%s,\"screen3_enabled\":%s}",
             settings->touch_sensitivity_level,
             settings->demo_mode_enabled ? "true" : "false",
             settings->screen3_enabled ? "true" : "false");
}

// Helper to deserialize settings from a JSON string
static bool settings_from_json(const char* json_str, touch_settings_t* settings) {
    // This is a very basic parser. A real implementation should use a JSON library like cJSON.
    const char* sens_key = "\"sensitivity\":";
    const char* demo_key = "\"demo_mode\":";
    const char* s3_key = "\"screen3_enabled\":";

    char* sens_ptr = strstr(json_str, sens_key);
    char* demo_ptr = strstr(json_str, demo_key);
    char* s3_ptr = strstr(json_str, s3_key);

    if (sens_ptr && demo_ptr && s3_ptr) {
        settings->touch_sensitivity_level = atoi(sens_ptr + strlen(sens_key));
        settings->demo_mode_enabled = strstr(demo_ptr, "true") != NULL;
        settings->screen3_enabled = strstr(s3_ptr, "true") != NULL;
        return true;
    }
    return false;
}

void settings_init_defaults(touch_settings_t *settings) {
    if (settings == NULL) return;
    settings->touch_sensitivity_level = DEFAULT_TOUCH_SENSITIVITY;
    settings->demo_mode_enabled = DEFAULT_DEMO_MODE_ENABLED;
    settings->screen3_enabled = DEFAULT_SCREEN3_ENABLED;
    for (int i = 0; i < SCREEN1_ARCS_COUNT; i++) settings->screen1_arcs_enabled[i] = true;
    for (int i = 0; i < SCREEN2_ARCS_COUNT; i++) settings->screen2_arcs_enabled[i] = true;
}

/**
 * @brief Saves settings to the SD card.
 * This is a slow, blocking function and should only be called from a background task.
 */
void settings_save(void) {
    // Save to SD Card as JSON
    char json_buffer[256];
    settings_to_json(&current_settings, json_buffer, sizeof(json_buffer));
    if (sd_card_write_file("/sdcard/settings.json", json_buffer) == ESP_OK) {
        ESP_LOGI(TAG, "Settings saved to SD card successfully.");
    } else {
        ESP_LOGE(TAG, "Failed to save settings to SD card.");
    }
}

/**
 * @brief Queues a request to save the current settings in a background task.
 */
void trigger_settings_save(void) {
    background_task_t task = {
        .type = BG_TASK_SETTINGS_SAVE,
        .callback = NULL // No callback needed for this simple case
    };
    if (background_task_add(&task) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to queue settings save task. Queue might be full.");
    } else {
        ESP_LOGI(TAG, "Settings save queued for background processing.");
    }
}

/**
 * @brief Loads settings from the SD card. If it fails, loads defaults.
 */
esp_err_t settings_load(void) {
    // Try to load from SD card
    FILE* f = fopen("/sdcard/settings.json", "r");
    if (f != NULL) {
        char buffer[256] = {0};
        fread(buffer, 1, sizeof(buffer) - 1, f);
        fclose(f);
        if (settings_from_json(buffer, &current_settings)) {
            ESP_LOGI(TAG, "Settings loaded from SD card successfully.");
            return ESP_OK;
        } else {
            ESP_LOGW(TAG, "Failed to parse settings.json, using defaults.");
            settings_init_defaults(&current_settings);
            return ESP_FAIL;
        }
    }

    // If file doesn't exist or can't be opened, use defaults.
    ESP_LOGI(TAG, "settings.json not found on SD card, initializing with defaults.");
    settings_init_defaults(&current_settings);
    return ESP_ERR_NOT_FOUND;
}

// ... other functions like settings_validate, getters/setters, etc. remain the same ...

bool settings_validate(touch_settings_t *settings) { if (settings == NULL) return false; if (settings->touch_sensitivity_level < MIN_TOUCH_SENSITIVITY || settings->touch_sensitivity_level > MAX_TOUCH_SENSITIVITY) return false; return true; }
void settings_print_debug(touch_settings_t *settings) { if(settings == NULL) return; ESP_LOGI(TAG, "Settings Debug: Touch=%d, Demo=%s, Screen3=%s", settings->touch_sensitivity_level, settings->demo_mode_enabled ? "ON":"OFF", settings->screen3_enabled ? "ON":"OFF"); }
bool demo_mode_get_enabled(void) { return current_settings.demo_mode_enabled; }
void demo_mode_set_enabled(bool enabled) { current_settings.demo_mode_enabled = enabled; }
bool screen3_get_enabled(void) { return current_settings.screen3_enabled; }
void screen3_set_enabled(bool enabled) { current_settings.screen3_enabled = enabled; }
void settings_apply_changes(void) { ESP_LOGI(TAG, "Applying settings changes..."); }
void settings_reset_to_defaults(void) { ESP_LOGI(TAG, "Resetting settings to defaults in memory"); settings_init_defaults(&current_settings); settings_apply_changes(); }
bool screen1_arc_get_enabled(int arc_index) { if (arc_index < 0 || arc_index >= SCREEN1_ARCS_COUNT) return false; return current_settings.screen1_arcs_enabled[arc_index]; }
void screen1_arc_set_enabled(int arc_index, bool enabled) { if (arc_index < 0 || arc_index >= SCREEN1_ARCS_COUNT) return; current_settings.screen1_arcs_enabled[arc_index] = enabled; }
bool screen2_arc_get_enabled(int arc_index) { if (arc_index < 0 || arc_index >= SCREEN2_ARCS_COUNT) return false; return current_settings.screen2_arcs_enabled[arc_index]; }
void screen2_arc_set_enabled(int arc_index, bool enabled) { if (arc_index < 0 || arc_index >= SCREEN2_ARCS_COUNT) return; current_settings.screen2_arcs_enabled[arc_index] = enabled; }
void ui_Screen1_update_arcs_visibility(void) { ESP_LOGD(TAG, "Screen1 arcs visibility update requested"); }
void ui_Screen2_update_arcs_visibility(void) { ESP_LOGD(TAG, "Screen2 arcs visibility update requested"); }
void demo_mode_test_toggle(void) { current_settings.demo_mode_enabled = !current_settings.demo_mode_enabled; }
void demo_mode_status_report(void) { ESP_LOGI(TAG, "Demo Mode Status: %s", current_settings.demo_mode_enabled ? "ENABLED" : "DISABLED"); }
