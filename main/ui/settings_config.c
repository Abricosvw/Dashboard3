// Settings Configuration Implementation
// Implements settings persistence and management functions

#include "settings_config.h"
#include <esp_log.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <string.h>
#include "../background_task.h"  // Фоновая задача для асинхронных операций

// Tag for logging
static const char *TAG = "SETTINGS_CONFIG";

// NVS namespace for settings
#define NVS_NAMESPACE "settings"

// Global settings structure
static touch_settings_t current_settings;

// Initialize default settings
void settings_init_defaults(touch_settings_t *settings) {
    if (settings == NULL) return;

    settings->touch_sensitivity_level = DEFAULT_TOUCH_SENSITIVITY;
    settings->demo_mode_enabled = DEFAULT_DEMO_MODE_ENABLED;
    settings->screen3_enabled = DEFAULT_SCREEN3_ENABLED;

    // Initialize all screen arcs to enabled
    for (int i = 0; i < SCREEN1_ARCS_COUNT; i++) {
        settings->screen1_arcs_enabled[i] = true;
    }
    for (int i = 0; i < SCREEN2_ARCS_COUNT; i++) {
        settings->screen2_arcs_enabled[i] = true;
    }
}

// Validate settings structure
bool settings_validate(touch_settings_t *settings) {
    if (settings == NULL) return false;

    if (settings->touch_sensitivity_level < MIN_TOUCH_SENSITIVITY ||
        settings->touch_sensitivity_level > MAX_TOUCH_SENSITIVITY) {
        return false;
    }

    return true;
}

// Print debug information
void settings_print_debug(touch_settings_t *settings) {
    if (settings == NULL) return;

    ESP_LOGI(TAG, "Settings Debug:");
    ESP_LOGI(TAG, "  Touch Sensitivity: %d", settings->touch_sensitivity_level);
    ESP_LOGI(TAG, "  Demo Mode: %s", settings->demo_mode_enabled ? "ON" : "OFF");
    ESP_LOGI(TAG, "  Screen3: %s", settings->screen3_enabled ? "ON" : "OFF");

    ESP_LOGI(TAG, "  Screen1 Arcs:");
    for (int i = 0; i < SCREEN1_ARCS_COUNT; i++) {
        ESP_LOGI(TAG, "    Arc %d: %s", i, settings->screen1_arcs_enabled[i] ? "ON" : "OFF");
    }

    ESP_LOGI(TAG, "  Screen2 Arcs:");
    for (int i = 0; i < SCREEN2_ARCS_COUNT; i++) {
        ESP_LOGI(TAG, "    Arc %d: %s", i, settings->screen2_arcs_enabled[i] ? "ON" : "OFF");
    }
}

// Demo mode functions
bool demo_mode_get_enabled(void) {
    return current_settings.demo_mode_enabled;
}

void demo_mode_set_enabled(bool enabled) {
    current_settings.demo_mode_enabled = enabled;
}

// Screen3 functions
bool screen3_get_enabled(void) {
    return current_settings.screen3_enabled;
}

void screen3_set_enabled(bool enabled) {
    current_settings.screen3_enabled = enabled;
}

// Screen arcs functions (остаются без изменений)
// ... (screen1_arc_get_enabled, set_enabled, etc.)

// =================================================================
// ИСПРАВЛЕННЫЕ ФУНКЦИИ / FIXED FUNCTIONS
// =================================================================

/**
 * @brief Медленная, блокирующая функция сохранения.
 * Вызывается ТОЛЬКО из фоновой задачи.
 */
void settings_save_to_nvs(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }

    err = nvs_set_blob(nvs_handle, "touch_settings", &current_settings, sizeof(touch_settings_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving settings to NVS: %s", esp_err_to_name(err));
    } else {
        err = nvs_commit(nvs_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "Settings saved to NVS successfully");
        }
    }

    nvs_close(nvs_handle);
}

/**
 * @brief Асинхронное сохранение настроек с использованием фоновой задачи
 * Не блокирует UI поток
 */
void trigger_settings_save(void) {
    // Callback функция для уведомления о завершении сохранения
    void settings_save_callback(esp_err_t result) {
        if (result == ESP_OK) {
            ESP_LOGI(TAG, "Settings saved to NVS asynchronously - SUCCESS");
        } else {
            ESP_LOGE(TAG, "Failed to save settings asynchronously: %s", esp_err_to_name(result));
        }
    }

    // Используем фоновую задачу для асинхронного сохранения
    esp_err_t ret = background_nvs_save_async(NVS_NAMESPACE, "touch_settings",
                                             &current_settings, sizeof(touch_settings_t),
                                             settings_save_callback, NULL);

    if (ret != ESP_OK) {
        // Если не удалось добавить в очередь, просто логируем ошибку.
        // UI не должен блокироваться.
        ESP_LOGE(TAG, "Failed to queue settings save task: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Settings save queued for background processing");
    }
}

// Load settings from NVS (остается без изменений)
esp_err_t settings_load_from_nvs(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "NVS not found, initializing with defaults.");
        settings_init_defaults(&current_settings);
        return ESP_ERR_NVS_NOT_FOUND;
    }

    size_t required_size = sizeof(touch_settings_t);
    err = nvs_get_blob(nvs_handle, "touch_settings", &current_settings, &required_size);
    if (err != ESP_OK || required_size != sizeof(touch_settings_t)) {
        ESP_LOGW(TAG, "Failed to load settings or size mismatch, using defaults.");
        settings_init_defaults(&current_settings);
    } else {
        ESP_LOGI(TAG, "Settings loaded from NVS successfully");
        if (!settings_validate(&current_settings)) {
            ESP_LOGW(TAG, "Loaded settings are invalid, using defaults");
            settings_init_defaults(&current_settings);
        }
    }

    nvs_close(nvs_handle);
    return err;
}

// Apply current settings changes
void settings_apply_changes(void) {
    ESP_LOGI(TAG, "Applying settings changes...");
    // Здесь должен быть код, который применяет настройки к UI, например,
    // обновляет анимации в зависимости от demo_mode_enabled
    // ui_Screen1_update_animations(demo_mode_get_enabled());
    // ui_Screen2_update_animations(demo_mode_get_enabled());
}

/**
 * @brief Сбрасывает настройки в памяти к значениям по умолчанию.
 * НЕ сохраняет их в NVS. Сохранение должно быть вызвано отдельно.
 */
void settings_reset_to_defaults(void) {
    ESP_LOGI(TAG, "Resetting settings to defaults in memory");
    settings_init_defaults(&current_settings);
    settings_apply_changes();
}

// Остальные функции (screen1_arc_get_enabled и т.д.) остаются без изменений.
// ...
bool screen1_arc_get_enabled(int arc_index) { if (arc_index < 0 || arc_index >= SCREEN1_ARCS_COUNT) return false; return current_settings.screen1_arcs_enabled[arc_index]; }
void screen1_arc_set_enabled(int arc_index, bool enabled) { if (arc_index < 0 || arc_index >= SCREEN1_ARCS_COUNT) return; current_settings.screen1_arcs_enabled[arc_index] = enabled; }
bool screen2_arc_get_enabled(int arc_index) { if (arc_index < 0 || arc_index >= SCREEN2_ARCS_COUNT) return false; return current_settings.screen2_arcs_enabled[arc_index]; }
void screen2_arc_set_enabled(int arc_index, bool enabled) { if (arc_index < 0 || arc_index >= SCREEN2_ARCS_COUNT) return; current_settings.screen2_arcs_enabled[arc_index] = enabled; }
void ui_Screen1_update_arcs_visibility(void) { ESP_LOGD(TAG, "Screen1 arcs visibility update requested"); }
void ui_Screen2_update_arcs_visibility(void) { ESP_LOGD(TAG, "Screen2 arcs visibility update requested"); }
void demo_mode_test_toggle(void) { current_settings.demo_mode_enabled = !current_settings.demo_mode_enabled; }
void demo_mode_status_report(void) { ESP_LOGI(TAG, "Demo Mode Status: %s", current_settings.demo_mode_enabled ? "ENABLED" : "DISABLED"); }
