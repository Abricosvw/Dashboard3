
// ECU Dashboard Screen with 6 animated gauges
// Based on test project structure

#include "../ui.h"
#include "ui_Screen1.h"
#include "ui_Screen3.h"
#include "../ui_screen_manager.h"
#include "esp_log.h"
#include <stdio.h>

lv_obj_t * ui_Screen1 = NULL;
lv_obj_t * ui_Arc_MAP = NULL;
lv_obj_t * ui_Arc_Wastegate = NULL;
lv_obj_t * ui_Arc_TPS = NULL;
lv_obj_t * ui_Arc_RPM = NULL;
lv_obj_t * ui_Arc_Boost = NULL;
// Intake Air Temp убран, возвращен TCU
lv_obj_t * ui_LED_TCU = NULL;
lv_obj_t * ui_Label_TCU_Status = NULL;

// Touch cursor for Screen 1
lv_obj_t * ui_Touch_Cursor_Screen1 = NULL;

lv_obj_t * ui_Label_MAP_Value = NULL;
lv_obj_t * ui_Label_Wastegate_Value = NULL;
lv_obj_t * ui_Label_TPS_Value = NULL;
lv_obj_t * ui_Label_RPM_Value = NULL;
lv_obj_t * ui_Label_Boost_Value = NULL;
// Intake Air Temp label убран

static lv_anim_t anim_map;
static lv_anim_t anim_wastegate;
static lv_anim_t anim_tps;
static lv_anim_t anim_rpm;
static lv_anim_t anim_boost;


// Forward declarations for touch cursor animation callbacks
static void fade_anim_cb_screen1(void * var, int32_t v);
static void fade_ready_cb_screen1(lv_anim_t * a);
static void screen1_prev_screen_btn_event_cb(lv_event_t * e);
static void screen1_next_screen_btn_event_cb(lv_event_t * e);
static void swipe_handler_screen1(lv_event_t * e);

// Forward declarations for splash screen animation callbacks - REMOVED UNUSED FUNCTIONS

static void anim_value_cb(void * var, int32_t v)
{
    lv_arc_set_value((lv_obj_t *)var, v);
    
    if(var == ui_Arc_MAP) {
        lv_label_set_text_fmt(ui_Label_MAP_Value, "%d", v);
    }
    else if(var == ui_Arc_Wastegate) {
        lv_label_set_text_fmt(ui_Label_Wastegate_Value, "%d", v);
    }
    else if(var == ui_Arc_TPS) {
        lv_label_set_text_fmt(ui_Label_TPS_Value, "%d", v);
    }
    else if(var == ui_Arc_RPM) {
        lv_label_set_text_fmt(ui_Label_RPM_Value, "%d", v);
        
        // Update Arc RPM color based on RPM range
        if(v >= 0 && v < 5000) {
            // 0-5000: голубой/cyan
            lv_obj_set_style_arc_color(ui_Arc_RPM, lv_color_hex(0x00D4FF), LV_PART_INDICATOR);
        }
        else if(v >= 5000 && v < 6500) {
            // 5000-6500: желтый
            lv_obj_set_style_arc_color(ui_Arc_RPM, lv_color_hex(0xFFD700), LV_PART_INDICATOR);
        }
        else if(v >= 6500 && v <= 8000) {
            // 6500-8000: красный
            lv_obj_set_style_arc_color(ui_Arc_RPM, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
        }
        
        // Update TCU status based on RPM
        if(v > 5500) {
            lv_led_set_color(ui_LED_TCU, lv_color_hex(0xFF0000));
            lv_label_set_text(ui_Label_TCU_Status, "ERROR");
            lv_obj_set_style_text_color(ui_Label_TCU_Status, lv_color_hex(0xFF0000), 0);
        }
        else if(v > 4500) {
            lv_led_set_color(ui_LED_TCU, lv_color_hex(0xFFAA00));
            lv_label_set_text(ui_Label_TCU_Status, "WARNING");
            lv_obj_set_style_text_color(ui_Label_TCU_Status, lv_color_hex(0xFFAA00), 0);
        }
        else {
            lv_led_set_color(ui_LED_TCU, lv_color_hex(0x00FF00));
            lv_label_set_text(ui_Label_TCU_Status, "OK");
            lv_obj_set_style_text_color(ui_Label_TCU_Status, lv_color_hex(0x00FF00), 0);
        }
    }
    else if(var == ui_Arc_Boost) {
        lv_label_set_text_fmt(ui_Label_Boost_Value, "%d", v);
    }
    // Intake Air Temp обработка убрана
}

static void create_gauge(lv_obj_t * parent, lv_obj_t ** arc, lv_obj_t ** label,
                        const char * title, const char * unit, lv_color_t color,
                        int32_t min_val, int32_t max_val, int x, int y)
{
    // Container - возвращаем оригинальный размер 250x225 для лучших пропорций
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_width(cont, 250);
    lv_obj_set_height(cont, 225);
    lv_obj_set_x(cont, x);
    lv_obj_set_y(cont, y);
    lv_obj_set_align(cont, LV_ALIGN_TOP_LEFT);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x2a2a2a), 0);
    lv_obj_set_style_border_color(cont, color, 0);
    lv_obj_set_style_border_width(cont, 2, 0);
    lv_obj_set_style_radius(cont, 15, 0);
    lv_obj_set_style_pad_all(cont, 10, 0);

    // Title
    lv_obj_t * label_title = lv_label_create(cont);
    lv_label_set_text(label_title, title);
    lv_obj_set_style_text_color(label_title, lv_color_white(), 0);
    lv_obj_align(label_title, LV_ALIGN_BOTTOM_MID, 0, -15);

    // Arc - возвращаем оригинальный размер дуги
    *arc = lv_arc_create(cont);
    lv_obj_set_size(*arc, 160, 160);
    lv_arc_set_rotation(*arc, 135);
    lv_arc_set_bg_angles(*arc, 0, 270);
    lv_arc_set_range(*arc, min_val, max_val);
    lv_arc_set_value(*arc, min_val);
    lv_obj_set_style_arc_color(*arc, color, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(*arc, 15, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(*arc, lv_color_hex(0x4a4a4a), LV_PART_MAIN);
    lv_obj_set_style_arc_width(*arc, 15, LV_PART_MAIN);
    lv_obj_center(*arc);
    lv_obj_remove_style(*arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(*arc, LV_OBJ_FLAG_CLICKABLE);

    // Value label
    *label = lv_label_create(cont);
    lv_label_set_text(*label, "0");
    lv_obj_set_style_text_color(*label, lv_color_white(), 0);
    lv_obj_center(*label);
    lv_obj_align(*label, LV_ALIGN_CENTER, 0, -5);

    // Unit label
    lv_obj_t * label_unit = lv_label_create(cont);
    lv_label_set_text(label_unit, unit);
    lv_obj_set_style_text_color(label_unit, lv_color_hex(0xcccccc), 0);
    lv_obj_align_to(label_unit, *label, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
}

void ui_Screen1_screen_init(void)
{
    ui_Screen1 = lv_obj_create(NULL);
    
    // ЖЕСТКО ФИКСИРУЕМ РАЗМЕР ЭКРАНА - 800x480
    lv_obj_set_size(ui_Screen1, 800, 480);
    lv_obj_set_pos(ui_Screen1, 0, 0);
    
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(0x1a1a1a), 0);
    
    // Проверяем границы экрана - все элементы должны быть внутри 800x480
    ESP_LOGI("SCREEN1", "Screen1 initialized with HARD FIXED size: 800x480");
    ESP_LOGI("SCREEN1", "All gauges will be constrained within these boundaries");
    

    
    // Create gauges in 3x2 grid (5 датчиков + TCU)
    // ВСЕ ПОЗИЦИИ ПРОВЕРЕНЫ - НЕ ВЫХОДЯТ ЗА ГРАНИЦЫ 800x480

    // Первая строка (y=15) - исправленные координаты
    create_gauge(ui_Screen1, &ui_Arc_MAP, &ui_Label_MAP_Value,
                "MAP Pressure", "kPa", lv_color_hex(0x00D4FF), 100, 250, 15, 15);
                // Датчик 1: x=15 to 265, расстояние до края: 15px

    create_gauge(ui_Screen1, &ui_Arc_Wastegate, &ui_Label_Wastegate_Value,
                "Wastegate", "%", lv_color_hex(0x00D4FF), 0, 100, 285, 15);
                // Датчик 2: x=285 to 535, расстояние от датчика 1: 285-265=20px

    create_gauge(ui_Screen1, &ui_Arc_TPS, &ui_Label_TPS_Value,
                "TPS Position", "%", lv_color_hex(0x00D4FF), 0, 100, 545, 15);
                // Датчик 3: x=545 to 795, расстояние от датчика 2: 545-535=10px

    // Вторая строка (y=245) - исправленные координаты
    create_gauge(ui_Screen1, &ui_Arc_RPM, &ui_Label_RPM_Value,
                "Engine RPM", "RPM", lv_color_hex(0x00D4FF), 0, 8000, 15, 245);
                // Датчик 4: x=15 to 265, расстояние от верхнего ряда: 245-240=5px

    create_gauge(ui_Screen1, &ui_Arc_Boost, &ui_Label_Boost_Value,
                "Target Boost", "kPa", lv_color_hex(0x00D4FF), 100, 250, 285, 245);
                // Датчик 5: x=285 to 535, расстояние от датчика 4: 285-265=20px

    // Датчик 6: x=545 to 795, расстояние от датчика 5: 545-535=10px, 5px от границы

    // Intake Air Temp убран, возвращен TCU
    // TCU Status indicator
    lv_obj_t * tcu_cont = lv_obj_create(ui_Screen1);
    lv_obj_set_width(tcu_cont, 250);
    lv_obj_set_height(tcu_cont, 225);
    lv_obj_set_x(tcu_cont, 545);  // ИЗМЕНЕНО: 565 → 545 (5px от границы)
    lv_obj_set_y(tcu_cont, 245);
    lv_obj_set_align(tcu_cont, LV_ALIGN_TOP_LEFT);
    lv_obj_clear_flag(tcu_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(tcu_cont, lv_color_hex(0x2a2a2a), 0);
    lv_obj_set_style_border_color(tcu_cont, lv_color_hex(0x00D4FF), 0);
    lv_obj_set_style_border_width(tcu_cont, 2, 0);
    lv_obj_set_style_radius(tcu_cont, 15, 0);
    lv_obj_set_style_pad_all(tcu_cont, 10, 0);

    lv_obj_t * tcu_title = lv_label_create(tcu_cont);
    lv_label_set_text(tcu_title, "TCU Status");
    lv_obj_set_style_text_color(tcu_title, lv_color_white(), 0);
    lv_obj_align(tcu_title, LV_ALIGN_BOTTOM_MID, 0, -15);

    ui_LED_TCU = lv_led_create(tcu_cont);
    lv_obj_set_size(ui_LED_TCU, 80, 80);
    lv_obj_center(ui_LED_TCU);
    lv_led_set_color(ui_LED_TCU, lv_color_hex(0x00FF00));
    lv_led_on(ui_LED_TCU);

    ui_Label_TCU_Status = lv_label_create(tcu_cont);
    lv_label_set_text(ui_Label_TCU_Status, "OK");
    lv_obj_set_style_text_color(ui_Label_TCU_Status, lv_color_hex(0x00FF00), 0);
    lv_obj_align(ui_Label_TCU_Status, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    // Setup animations
    lv_anim_init(&anim_map);
    lv_anim_set_var(&anim_map, ui_Arc_MAP);
    lv_anim_set_values(&anim_map, 100, 250);
    lv_anim_set_time(&anim_map, 3000);
    lv_anim_set_playback_time(&anim_map, 3000);
    lv_anim_set_repeat_count(&anim_map, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&anim_map, anim_value_cb);

    // Start animation only if demo mode is enabled
    if (demo_mode_get_enabled()) {
        lv_anim_start(&anim_map);
    }
    
    lv_anim_init(&anim_wastegate);
    lv_anim_set_var(&anim_wastegate, ui_Arc_Wastegate);
    lv_anim_set_values(&anim_wastegate, 0, 100);
    lv_anim_set_time(&anim_wastegate, 2500);
    lv_anim_set_playback_time(&anim_wastegate, 2500);
    lv_anim_set_repeat_count(&anim_wastegate, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&anim_wastegate, anim_value_cb);

    // Start animation only if demo mode is enabled
    if (demo_mode_get_enabled()) {
        lv_anim_start(&anim_wastegate);
    }
    
    lv_anim_init(&anim_tps);
    lv_anim_set_var(&anim_tps, ui_Arc_TPS);
    lv_anim_set_values(&anim_tps, 0, 100);
    lv_anim_set_time(&anim_tps, 2000);
    lv_anim_set_playback_time(&anim_tps, 2000);
    lv_anim_set_repeat_count(&anim_tps, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&anim_tps, anim_value_cb);

    // Start animation only if demo mode is enabled
    if (demo_mode_get_enabled()) {
        lv_anim_start(&anim_tps);
    }
    
    lv_anim_init(&anim_rpm);
    lv_anim_set_var(&anim_rpm, ui_Arc_RPM);
    lv_anim_set_values(&anim_rpm, 0, 8000);
    lv_anim_set_time(&anim_rpm, 4000);
    lv_anim_set_playback_time(&anim_rpm, 4000);
    lv_anim_set_repeat_count(&anim_rpm, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&anim_rpm, anim_value_cb);

    // Start animation only if demo mode is enabled
    if (demo_mode_get_enabled()) {
        lv_anim_start(&anim_rpm);
    }
    
    lv_anim_init(&anim_boost);
    lv_anim_set_var(&anim_boost, ui_Arc_Boost);
    lv_anim_set_values(&anim_boost, 100, 250);
    lv_anim_set_time(&anim_boost, 3500);
    lv_anim_set_playback_time(&anim_boost, 3500);
    lv_anim_set_repeat_count(&anim_boost, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&anim_boost, anim_value_cb);

    // Start animation only if demo mode is enabled
    if (demo_mode_get_enabled()) {
        lv_anim_start(&anim_boost);
    }
    
    // Create touch cursor for Screen 1
    ui_Touch_Cursor_Screen1 = lv_obj_create(ui_Screen1);
    lv_obj_set_size(ui_Touch_Cursor_Screen1, 30, 30);
    lv_obj_set_style_bg_color(ui_Touch_Cursor_Screen1, lv_color_hex(0x00D4FF), 0);
    lv_obj_set_style_radius(ui_Touch_Cursor_Screen1, 15, 0);
    lv_obj_set_style_opa(ui_Touch_Cursor_Screen1, 0, 0); // Initially hidden
    lv_obj_add_flag(ui_Touch_Cursor_Screen1, LV_OBJ_FLAG_HIDDEN);
    
    // Touch gauges functionality removed - no longer needed
    
    // Add touch event handlers for basic touch functionality
    lv_obj_add_event_cb(ui_Screen1, general_touch_handler, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_Screen1, general_touch_handler, LV_EVENT_RELEASED, NULL);
    
    // Add swipe functionality for screen switching
    lv_obj_add_event_cb(ui_Screen1, swipe_handler_screen1, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_Screen1, swipe_handler_screen1, LV_EVENT_RELEASED, NULL);
    
    // Add navigation buttons for screen switching

    // Previous screen button (left arrow)
    lv_obj_t * prev_screen_btn = lv_btn_create(ui_Screen1);
    lv_obj_set_size(prev_screen_btn, 50, 50);
    lv_obj_set_x(prev_screen_btn, 10);
    lv_obj_set_y(prev_screen_btn, 400);
    lv_obj_set_style_bg_color(prev_screen_btn, lv_color_hex(0x00D4FF), 0);
    lv_obj_set_style_radius(prev_screen_btn, 25, 0);
    lv_obj_add_event_cb(prev_screen_btn, screen1_prev_screen_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * prev_icon = lv_label_create(prev_screen_btn);
    lv_label_set_text(prev_icon, "←");
    lv_obj_set_style_text_color(prev_icon, lv_color_white(), 0);
    lv_obj_set_style_text_font(prev_icon, &lv_font_montserrat_20, 0);
    lv_obj_center(prev_icon);

    // Previous screen label
    lv_obj_t * prev_label = lv_label_create(ui_Screen1);
    lv_label_set_text(prev_label, "Prev Screen");
    lv_obj_set_style_text_color(prev_label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(prev_label, &lv_font_montserrat_12, 0);
    lv_obj_align_to(prev_label, prev_screen_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    // Next screen button (right arrow)
    lv_obj_t * next_screen_btn = lv_btn_create(ui_Screen1);
    lv_obj_set_size(next_screen_btn, 50, 50);
    lv_obj_set_x(next_screen_btn, 750);
    lv_obj_set_y(next_screen_btn, 400);
    lv_obj_set_style_bg_color(next_screen_btn, lv_color_hex(0x00D4FF), 0);
    lv_obj_set_style_radius(next_screen_btn, 25, 0);
    lv_obj_add_event_cb(next_screen_btn, screen1_next_screen_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t * next_icon = lv_label_create(next_screen_btn);
    lv_label_set_text(next_icon, "→");
    lv_obj_set_style_text_color(next_icon, lv_color_white(), 0);
    lv_obj_set_style_text_font(next_icon, &lv_font_montserrat_20, 0);
    lv_obj_center(next_icon);
    
    // Next screen label
    lv_obj_t * next_label = lv_label_create(ui_Screen1);
    lv_label_set_text(next_label, "Next Screen");
    lv_obj_set_style_text_color(next_label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(next_label, &lv_font_montserrat_12, 0);
    lv_obj_align_to(next_label, next_screen_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    
    ESP_LOGI("SCREEN1", "Screen 1 initialized with basic touch functionality, swipe gestures, and navigation buttons");
}

// Previous screen button event callback
static void screen1_prev_screen_btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // Switch to previous enabled screen (backward direction)
        ui_switch_to_next_enabled_screen(false);
    }
}

// Next screen button event callback
static void screen1_next_screen_btn_event_cb(lv_event_t * e)
{
    // Switch to next enabled screen (forward direction)
    ui_switch_to_next_enabled_screen(true);
}

// Swipe handler for Screen1 - detects right swipe to go to Screen2
static void swipe_handler_screen1(lv_event_t * e)
{
    static lv_point_t swipe_start = {0, 0};
    static bool swipe_in_progress = false;
    
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_PRESSED) {
        lv_indev_get_point(lv_indev_get_act(), &swipe_start);
        swipe_in_progress = true;
        ESP_LOGI("SWIPE_SCREEN1", "Swipe started at x=%d, y=%d", swipe_start.x, swipe_start.y);
    } else if (code == LV_EVENT_RELEASED && swipe_in_progress) {
        lv_point_t swipe_end;
        lv_indev_get_point(lv_indev_get_act(), &swipe_end);
        
        int16_t delta_x = swipe_end.x - swipe_start.x;
        int16_t delta_y = swipe_end.y - swipe_start.y;
        
        ESP_LOGI("SWIPE_SCREEN1", "Swipe ended at x=%d, y=%d, delta_x=%d, delta_y=%d", 
                  swipe_end.x, swipe_end.y, delta_x, delta_y);
        
        // Check if it's a horizontal swipe
        if (abs(delta_x) > abs(delta_y) && abs(delta_x) > 100) {
            if (delta_x > 50) {
                // Right swipe - go to next enabled screen (forward direction)
                ESP_LOGI("SWIPE_SCREEN1", "Right swipe detected, switching to next enabled screen");
                ui_switch_to_next_enabled_screen(true);
            } else if (delta_x < -50) {
                // Left swipe - go to previous enabled screen (backward direction)
                ESP_LOGI("SWIPE_SCREEN1", "Left swipe detected, switching to previous enabled screen");
                ui_switch_to_next_enabled_screen(false);
            }
        }
        
        swipe_in_progress = false;
    }
}

// Touch cursor update function for Screen1
void ui_update_touch_cursor(lv_point_t * point) {
    printf("Touch cursor Screen1 update called: x=%d, y=%d\n", point->x, point->y);
    
    if (ui_Touch_Cursor_Screen1 && point) {
        // Move cursor to touch position
        lv_obj_set_pos(ui_Touch_Cursor_Screen1, point->x - 15, point->y - 15);
        
        // Make cursor visible
        lv_obj_clear_flag(ui_Touch_Cursor_Screen1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_opa(ui_Touch_Cursor_Screen1, 255, 0);
        
        printf("Cursor Screen1 made visible at x=%d, y=%d\n", point->x - 15, point->y - 15);
        
        // Start fade out animation
        lv_anim_t fade_anim;
        lv_anim_init(&fade_anim);
        lv_anim_set_var(&fade_anim, ui_Touch_Cursor_Screen1);
        lv_anim_set_values(&fade_anim, 255, 0);
        lv_anim_set_time(&fade_anim, 500);
        lv_anim_set_exec_cb(&fade_anim, fade_anim_cb_screen1);
        lv_anim_set_ready_cb(&fade_anim, fade_ready_cb_screen1);
        lv_anim_start(&fade_anim);
    } else {
        printf("Touch cursor Screen1 update failed: cursor=%p, point=%p\n", ui_Touch_Cursor_Screen1, point);
    }
}

// Splash screen animation callbacks - REMOVED UNUSED FUNCTIONS

// Animation callbacks for touch cursor
static void fade_anim_cb_screen1(void * var, int32_t v) {
    lv_obj_set_style_opa((lv_obj_t*)var, v, 0);
}

static void fade_ready_cb_screen1(lv_anim_t * a) {
    lv_obj_add_flag((lv_obj_t*)a->var, LV_OBJ_FLAG_HIDDEN);
}

// Function to control animations based on demo mode
void ui_Screen1_update_animations(bool demo_enabled)
{
    // Safety check - ensure UI objects are initialized
    if (!ui_Arc_MAP || !ui_Arc_Wastegate || !ui_Arc_TPS || !ui_Arc_RPM || !ui_Arc_Boost) {
        ESP_LOGW("SCREEN1", "UI objects not initialized yet, skipping animation update");
        return;
    }

    if (demo_enabled) {
        // Start all animations
        lv_anim_start(&anim_map);
        lv_anim_start(&anim_wastegate);
        lv_anim_start(&anim_tps);
        lv_anim_start(&anim_rpm);
        lv_anim_start(&anim_boost);
        ESP_LOGI("SCREEN1", "Started all animations (demo mode enabled)");
    } else {
        // Stop all animations and reset to minimum values
        lv_anim_del(ui_Arc_MAP, anim_value_cb);
        lv_anim_del(ui_Arc_Wastegate, anim_value_cb);
        lv_anim_del(ui_Arc_TPS, anim_value_cb);
        lv_anim_del(ui_Arc_RPM, anim_value_cb);
        lv_anim_del(ui_Arc_Boost, anim_value_cb);

        // Reset to minimum values
        lv_arc_set_value(ui_Arc_MAP, 100);
        lv_arc_set_value(ui_Arc_Wastegate, 0);
        lv_arc_set_value(ui_Arc_TPS, 0);
        lv_arc_set_value(ui_Arc_RPM, 0);
        lv_arc_set_value(ui_Arc_Boost, 100);

        // Update labels
        lv_label_set_text_fmt(ui_Label_MAP_Value, "%d", 100);
        lv_label_set_text_fmt(ui_Label_Wastegate_Value, "%d", 0);
        lv_label_set_text_fmt(ui_Label_TPS_Value, "%d", 0);
        lv_label_set_text_fmt(ui_Label_RPM_Value, "%d", 0);
        lv_label_set_text_fmt(ui_Label_Boost_Value, "%d", 100);

        ESP_LOGI("SCREEN1", "Stopped all animations and reset to minimum values (demo mode disabled)");
    }
}

// Function to control individual arc visibility
void ui_Screen1_update_arc_visibility(int arc_index, bool visible)
{
    lv_obj_t *arc_container = NULL;
    const char* arc_name = NULL;

    // Map arc index to container and name
    switch (arc_index) {
        case 0: // MAP
            arc_container = lv_obj_get_parent(ui_Arc_MAP);
            arc_name = "MAP Pressure";
            break;
        case 1: // Wastegate
            arc_container = lv_obj_get_parent(ui_Arc_Wastegate);
            arc_name = "Wastegate";
            break;
        case 2: // TPS
            arc_container = lv_obj_get_parent(ui_Arc_TPS);
            arc_name = "TPS Position";
            break;
        case 3: // RPM
            arc_container = lv_obj_get_parent(ui_Arc_RPM);
            arc_name = "Engine RPM";
            break;
        case 4: // Boost
            arc_container = lv_obj_get_parent(ui_Arc_Boost);
            arc_name = "Target Boost";
            break;
        // case 5 Intake Air Temp убран
        case 5: // TCU Status
            arc_container = lv_obj_get_parent(ui_LED_TCU);
            arc_name = "TCU Status";
            break;
        default:
            ESP_LOGW("SCREEN1", "Invalid arc index: %d", arc_index);
            return;
    }

    if (!arc_container) {
        ESP_LOGW("SCREEN1", "Arc container not found for index %d", arc_index);
        return;
    }

    if (visible) {
        lv_obj_set_style_opa(arc_container, LV_OPA_COVER, 0);
        ESP_LOGI("SCREEN1", "%s gauge is now VISIBLE", arc_name);
        
        // 🔍 ТРАБЛШУТИНГ: Логируем состояние видимости
        ESP_LOGI("TROUBLESHOOTING", "👁️ SCREEN1: %s стал ВИДИМЫМ", arc_name);
        ESP_LOGI("TROUBLESHOOTING", "   Прозрачность: LV_OPA_COVER (255)");
        ESP_LOGI("TROUBLESHOOTING", "   Контейнер: %p", arc_container);
    } else {
        lv_obj_set_style_opa(arc_container, LV_OPA_TRANSP, 0);
        ESP_LOGI("SCREEN1", "%s gauge is now HIDDEN", arc_name);
        
        // 🔍 ТРАБЛШУТИНГ: Логируем состояние видимости
        ESP_LOGI("TROUBLESHOOTING", "🙈 SCREEN1: %s стал НЕВИДИМЫМ", arc_name);
        ESP_LOGI("TROUBLESHOOTING", "   Прозрачность: LV_OPA_TRANSP (0)");
        ESP_LOGI("TROUBLESHOOTING", "   Контейнер: %p", arc_container);
    }
}

void ui_Screen1_screen_destroy(void)
{
    if(ui_Screen1) lv_obj_del(ui_Screen1);
    ui_Screen1 = NULL;
}
