// UI Screen Manager - Handles switching between screens
#include "ui_screen_manager.h"
#include "ui.h"
#include "lvgl.h"
#include "screens/ui_Screen3.h"
#include "screens/ui_Screen4.h"
#include "screens/ui_Screen6.h"
#include "settings_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Touch state variables
extern bool touch_active;
extern uint8_t touch_sensitivity_level;


// Touch timing variables
static uint32_t touch_start_time = 0;

// Touch point variables
static lv_point_t touch_start_point = {0, 0};
static lv_point_t last_touch_point = {0, 0};

// Current screen tracking
static screen_id_t current_screen = SCREEN_1;


// Initialize screen manager
void ui_screen_manager_init(void)
{
    ESP_LOGI("SCREEN_MANAGER", "Initializing UI screen manager...");
    
    // Set initial screen
    current_screen = SCREEN_1;
    
    ESP_LOGI("SCREEN_MANAGER", "UI screen manager initialized successfully");
}

// Check if can switch to Screen3
bool ui_can_switch_to_screen3(void)
{
    bool enabled = screen3_get_enabled();
    ESP_LOGI("SCREEN_MANAGER", "Screen3 access check: %s", enabled ? "ENABLED" : "DISABLED");
    return enabled;
}

// Check if a screen is enabled
static bool ui_is_screen_enabled(screen_id_t screen_id)
{
    switch (screen_id) {
        case SCREEN_1:
        case SCREEN_2:
        case SCREEN_4:
        case SCREEN_6:
            return true; // These screens are always enabled
        case SCREEN_3:
            return screen3_get_enabled(); // Check Screen3 status
        default:
            return false;
    }
}

// Get next enabled screen in specified direction
screen_id_t ui_get_next_enabled_screen(screen_id_t current_screen, bool forward)
{
    screen_id_t screens[] = {SCREEN_1, SCREEN_2, SCREEN_3, SCREEN_4, SCREEN_6};
    int num_screens = sizeof(screens) / sizeof(screens[0]);
    int current_index = -1;

    // Find current screen index
    for (int i = 0; i < num_screens; i++) {
        if (screens[i] == current_screen) {
            current_index = i;
            break;
        }
    }

    if (current_index == -1) {
        ESP_LOGW("SCREEN_MANAGER", "Current screen not found in navigation array");
        return SCREEN_1; // Fallback to first screen
    }

    // Search for next enabled screen
    int search_index = current_index;
    int iterations = 0;

    while (iterations < num_screens) {
        if (forward) {
            search_index = (search_index + 1) % num_screens;
        } else {
            search_index = (search_index - 1 + num_screens) % num_screens;
        }

        if (ui_is_screen_enabled(screens[search_index])) {
            ESP_LOGI("SCREEN_MANAGER", "Next enabled screen found: %d (from %d, direction: %s)",
                     screens[search_index], current_screen, forward ? "forward" : "backward");
            return screens[search_index];
        }

        iterations++;
    }

    // If no enabled screen found (shouldn't happen), return current screen
    ESP_LOGW("SCREEN_MANAGER", "No enabled screen found, staying on current screen");
    return current_screen;
}

// Get previous enabled screen (alias for backward direction)
screen_id_t ui_get_prev_enabled_screen(screen_id_t current_screen, bool forward)
{
    return ui_get_next_enabled_screen(current_screen, forward);
}

// Switch to next enabled screen in specified direction
void ui_switch_to_next_enabled_screen(bool forward)
{
    screen_id_t current_screen = ui_get_current_screen();
    screen_id_t next_screen = ui_get_next_enabled_screen(current_screen, forward);

    if (next_screen != current_screen) {
        ESP_LOGI("SCREEN_MANAGER", "Switching from screen %d to next enabled screen %d (direction: %s)",
                 current_screen, next_screen, forward ? "forward" : "backward");
        ui_switch_to_screen(next_screen);
    } else {
        ESP_LOGW("SCREEN_MANAGER", "No next enabled screen found, staying on current screen");
    }
}

// Switch to specified screen
void ui_switch_to_screen(screen_id_t screen_id)
{
    if (!touch_active) {
        ESP_LOGW("SCREEN_MANAGER", "Cannot switch screens: touch screen is disabled");
        return;
    }

    ESP_LOGI("SCREEN_MANAGER", "Switching to screen %d", screen_id);

    // Check if target screen is enabled
    if (!ui_is_screen_enabled(screen_id)) {
        ESP_LOGW("SCREEN_MANAGER", "Screen %d is disabled, finding next enabled screen", screen_id);
        // Find next enabled screen in forward direction from current screen
        screen_id_t next_enabled = ui_get_next_enabled_screen(current_screen, true);
        if (next_enabled != screen_id && ui_is_screen_enabled(next_enabled)) {
            ESP_LOGI("SCREEN_MANAGER", "Redirecting from disabled %d to next enabled screen: %d", screen_id, next_enabled);
            screen_id = next_enabled;
        } else {
            ESP_LOGW("SCREEN_MANAGER", "Cannot find any enabled screen to redirect to");
            return;
        }
    }

    switch (screen_id) {
        case SCREEN_1:
            lv_scr_load(ui_Screen1);
            current_screen = SCREEN_1;
            ESP_LOGI("SCREEN_MANAGER", "Switched to SCREEN_1");
            break;

        case SCREEN_2:
            lv_scr_load(ui_Screen2);
            current_screen = SCREEN_2;
            ESP_LOGI("SCREEN_MANAGER", "Switched to SCREEN_2");
            break;

        case SCREEN_3:
            if (!ui_can_switch_to_screen3()) {
                ESP_LOGW("SCREEN_MANAGER", "Cannot switch to SCREEN_3: Screen3 is disabled in settings");
                // Try to find next enabled screen
                screen_id_t next_enabled = ui_get_next_enabled_screen(current_screen, true);
                if (next_enabled != SCREEN_3 && ui_is_screen_enabled(next_enabled)) {
                    ESP_LOGI("SCREEN_MANAGER", "Redirecting from disabled SCREEN_3 to: %d", next_enabled);
                    ui_switch_to_screen(next_enabled);
                }
                return;
            }
            lv_scr_load(ui_Screen3);
            current_screen = SCREEN_3;
            ESP_LOGI("SCREEN_MANAGER", "Switched to SCREEN_3");
            break;

        case SCREEN_4:
            lv_scr_load(ui_Screen4);
            current_screen = SCREEN_4;
            ESP_LOGI("SCREEN_MANAGER", "Switched to SCREEN_4");
            break;

        case SCREEN_6:
            lv_scr_load(ui_Screen6);
            current_screen = SCREEN_6;
            ESP_LOGI("SCREEN_MANAGER", "Switched to SCREEN_6");
            break;

        default:
            ESP_LOGW("SCREEN_MANAGER", "Unknown screen ID: %d", screen_id);
            break;
    }
}

// Get current screen
screen_id_t ui_get_current_screen(void)
{
    return current_screen;
}


// Cleanup function
void ui_screen_manager_cleanup(void)
{
    ESP_LOGI("SCREEN_MANAGER", "Cleaning up UI screen manager...");
    
    // Clean up any allocated resources
    // For now, just log the cleanup process
    
    ESP_LOGI("SCREEN_MANAGER", "UI screen manager cleanup completed");
}

// ============================================================================
// ФУНКЦИИ ПРОВЕРКИ ГРАНИЦ ЭКРАНОВ
// ============================================================================

// Проверка границ экрана - все элементы должны быть внутри 800x480
bool ui_check_screen_bounds(int x, int y, int width, int height, const char* element_name)
{
    // Жесткие границы экрана
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 480;
    
    // Проверяем, не выходит ли элемент за границы
    if (x < 0 || y < 0 || 
        x + width > SCREEN_WIDTH || 
        y + height > SCREEN_HEIGHT) {
        
        ESP_LOGW("BOUNDS_CHECK", "❌ %s выходит за границы экрана!", element_name);
        ESP_LOGW("BOUNDS_CHECK", "   Позиция: x=%d, y=%d, размер: %dx%d", x, y, width, height);
        ESP_LOGW("BOUNDS_CHECK", "   Границы экрана: 0-800 x 0-480");
        return false; // Элемент выходит за границы
    }
    
    ESP_LOGI("BOUNDS_CHECK", "✅ %s в пределах границ экрана", element_name);
    ESP_LOGI("BOUNDS_CHECK", "   Позиция: x=%d, y=%d, размер: %dx%d", x, y, width, height);
    return true; // Элемент в пределах границ
}

// Проверка всех датчиков Screen1
void ui_validate_screen1_bounds(void)
{
    ESP_LOGI("BOUNDS_CHECK", "🔍 Проверка границ Screen1...");
    
    // Проверяем каждый датчик
    ui_check_screen_bounds(15, 7, 250, 225, "MAP Pressure");
    ui_check_screen_bounds(275, 7, 250, 225, "Wastegate");
    ui_check_screen_bounds(535, 7, 250, 225, "TPS Position");
    ui_check_screen_bounds(15, 247, 250, 225, "Engine RPM");
    ui_check_screen_bounds(275, 247, 250, 225, "Target Boost");
    ui_check_screen_bounds(535, 247, 250, 225, "TCU Status");
    
    ESP_LOGI("BOUNDS_CHECK", "✅ Проверка границ Screen1 завершена");
}

// Проверка всех датчиков Screen2
void ui_validate_screen2_bounds(void)
{
    ESP_LOGI("BOUNDS_CHECK", "🔍 Проверка границ Screen2...");
    
    // Проверяем каждый датчик (размер 200x180)
    ui_check_screen_bounds(15, 7, 200, 180, "Oil Pressure");
    ui_check_screen_bounds(225, 7, 200, 180, "Oil Temp");
    ui_check_screen_bounds(15, 197, 200, 180, "Water Temp");
    ui_check_screen_bounds(225, 197, 200, 180, "Fuel Pressure");
    
    ESP_LOGI("BOUNDS_CHECK", "✅ Проверка границ Screen2 завершена");
}

// Проверка всех элементов Settings
void ui_validate_settings_bounds(void)
{
    ESP_LOGI("BOUNDS_CHECK", "🔍 Проверка границ Settings...");
    
    // Проверяем основные элементы
    ui_check_screen_bounds(0, 20, 800, 24, "Title");
    ui_check_screen_bounds(50, 80, 80, 30, "Touch Gauges Switch");
    ui_check_screen_bounds(50, 120, 80, 30, "Demo Mode Switch");
    ui_check_screen_bounds(50, 160, 80, 30, "Screen3 Switch");
    ui_check_screen_bounds(50, 270, 200, 18, "Screen1 Title");
    ui_check_screen_bounds(50, 300, 150, 20, "MAP Checkbox");
    
    ESP_LOGI("BOUNDS_CHECK", "✅ Проверка границ Settings завершена");
}

// Общая проверка всех экранов
void ui_validate_all_screen_bounds(void)
{
    ESP_LOGI("BOUNDS_CHECK", "🚀 НАЧАЛО ПРОВЕРКИ ВСЕХ ГРАНИЦ ЭКРАНОВ");
    ESP_LOGI("BOUNDS_CHECK", "==========================================");
    
    ui_validate_screen1_bounds();
    ui_validate_screen2_bounds();
    ui_validate_settings_bounds();
    
    ESP_LOGI("BOUNDS_CHECK", "==========================================");
    ESP_LOGI("BOUNDS_CHECK", "✅ ПРОВЕРКА ВСЕХ ГРАНИЦ ЗАВЕРШЕНА");
}


