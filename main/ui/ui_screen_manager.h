// UI Screen Manager - Handles switching between screens
// Supports basic touch functionality, swipe gestures, and button navigation

#ifndef UI_SCREEN_MANAGER_H
#define UI_SCREEN_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// Screen IDs
typedef enum {
    SCREEN_1 = 0,      // Main ECU Dashboard
    SCREEN_2 = 1,      // Additional Gauges
    SCREEN_3 = 2,      // CAN Bus Terminal
    SCREEN_4 = 3,      // MRE Gauges Page 1
    SCREEN_5 = 4,      // MRE Gauges Page 2
    SCREEN_6 = 5       // Device Parameters Settings
} screen_id_t;

// Screen management functions
void ui_screen_manager_init(void);
void ui_switch_to_screen(screen_id_t screen_id);
screen_id_t ui_get_current_screen(void);
bool ui_can_switch_to_screen3(void);
screen_id_t ui_get_next_enabled_screen(screen_id_t current_screen, bool forward);
screen_id_t ui_get_prev_enabled_screen(screen_id_t current_screen, bool forward);
void ui_switch_to_next_enabled_screen(bool forward);


// Cleanup function
void ui_screen_manager_cleanup(void);

// ============================================================================
// ФУНКЦИИ ПРОВЕРКИ ГРАНИЦ ЭКРАНОВ
// ============================================================================

// Проверка границ экрана - все элементы должны быть внутри 800x480
bool ui_check_screen_bounds(int x, int y, int width, int height, const char* element_name);

// Проверка всех датчиков Screen1
void ui_validate_screen1_bounds(void);

// Проверка всех датчиков Screen2
void ui_validate_screen2_bounds(void);

// Общая проверка всех экранов
void ui_validate_all_screen_bounds(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
