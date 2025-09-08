// Settings Configuration
// Defines default values and configuration for touch screen settings

#ifndef SETTINGS_CONFIG_H
#define SETTINGS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Default settings values
#define DEFAULT_TOUCH_SENSITIVITY        5
#define DEFAULT_DEMO_MODE_ENABLED        true
#define DEFAULT_SCREEN3_ENABLED          true

// Touch sensitivity range
#define MIN_TOUCH_SENSITIVITY    1
#define MAX_TOUCH_SENSITIVITY    10

// Screen arcs count
#define SCREEN1_ARCS_COUNT       5
#define SCREEN2_ARCS_COUNT       4

// Settings structure
typedef struct {
    uint8_t touch_sensitivity_level;
    bool demo_mode_enabled;
    bool screen3_enabled;
    bool screen1_arcs_enabled[SCREEN1_ARCS_COUNT];
    bool screen2_arcs_enabled[SCREEN2_ARCS_COUNT];
} touch_settings_t;

// Function declarations
void settings_init_defaults(touch_settings_t * settings);
bool settings_validate(touch_settings_t * settings);
void settings_print_debug(touch_settings_t * settings);

// Demo mode control functions
bool demo_mode_get_enabled(void);
void demo_mode_set_enabled(bool enabled);
void demo_mode_test_toggle(void); // For testing purposes
void demo_mode_status_report(void); // Report current status

// Screen3 control functions
bool screen3_get_enabled(void);
void screen3_set_enabled(bool enabled);

// Screen arcs control functions
bool screen1_arc_get_enabled(int arc_index);
void screen1_arc_set_enabled(int arc_index, bool enabled);
bool screen2_arc_get_enabled(int arc_index);
void screen2_arc_set_enabled(int arc_index, bool enabled);

// Screen arcs update functions
void ui_Screen1_update_arcs_visibility(void);
void ui_Screen2_update_arcs_visibility(void);

// Settings persistence functions
void settings_save_to_nvs(void);
void trigger_settings_save(void);  // Асинхронное сохранение с фоновой задачей
void settings_apply_changes(void);
void settings_reset_to_defaults(void);
esp_err_t settings_load_from_nvs(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
