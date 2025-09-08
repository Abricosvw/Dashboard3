// ECU Dashboard Screen 4 - Empty Screen
// Fourth screen - clean empty screen for future development

#include "../ui.h"
#include "ui_Screen4.h"
#include "ui_screen_manager.h"
#include "ui_helpers.h"
#include <stdio.h>
#include <esp_log.h>

// Screen object
lv_obj_t * ui_Screen4;

// Touch cursor object
lv_obj_t * ui_Touch_Cursor_Screen4;

// Function prototypes
static void screen4_touch_handler(lv_event_t * e);
static void swipe_handler_screen4(lv_event_t * e);
static void screen4_prev_screen_btn_event_cb(lv_event_t * e);
static void screen4_next_screen_btn_event_cb(lv_event_t * e);

// Touch handler for general touch events
static void screen4_touch_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED) {
        // Show touch cursor at touch point
        lv_point_t point;
        lv_indev_t * indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &point);
            ui_update_touch_cursor_screen4(&point);
        }
    } else if (code == LV_EVENT_RELEASED) {
        // Hide touch cursor
        lv_obj_add_flag((lv_obj_t*)ui_Touch_Cursor_Screen4, LV_OBJ_FLAG_HIDDEN);
    }
}

// Swipe handler for screen switching
static void swipe_handler_screen4(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    static lv_point_t start_point;
    static int is_swiping = 0;

    if (code == LV_EVENT_PRESSED) {
        lv_indev_t * indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &start_point);
            is_swiping = 1;
        }
    } else if (code == LV_EVENT_RELEASED && is_swiping != 0) {
        lv_point_t end_point;
        lv_indev_t * indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &end_point);
            int delta_x = end_point.x - start_point.x;

            // Swipe right to go to previous enabled screen (backward direction)
            if (delta_x > 50) {
                ui_switch_to_next_enabled_screen(false);
            }
            // Swipe left to go to next enabled screen (forward direction)
            else if (delta_x < -50) {
                ui_switch_to_next_enabled_screen(true);
            }
        }
        is_swiping = 0;
    }
}

// Previous screen button event callback
static void screen4_prev_screen_btn_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // Switch to previous enabled screen (backward direction)
        ui_switch_to_next_enabled_screen(false);
    }
}

// Next screen button event callback
static void screen4_next_screen_btn_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // Switch to next enabled screen (forward direction)
        ui_switch_to_next_enabled_screen(true);
    }
}

// Touch cursor update function for Screen4
void ui_update_touch_cursor_screen4(lv_point_t * point) {
    if (ui_Touch_Cursor_Screen4 && point) {
        lv_obj_set_pos(ui_Touch_Cursor_Screen4, point->x - 10, point->y - 10);
        lv_obj_clear_flag(ui_Touch_Cursor_Screen4, LV_OBJ_FLAG_HIDDEN);
    }
}

// Main screen initialization
void ui_Screen4_screen_init(void) {
    ui_Screen4 = lv_obj_create(NULL);

    // ЖЕСТКО ФИКСИРУЕМ РАЗМЕР ЭКРАНА - 800x480
    lv_obj_set_size(ui_Screen4, 800, 480);
    lv_obj_set_pos(ui_Screen4, 0, 0);

    lv_obj_clear_flag(ui_Screen4, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_Screen4, lv_color_hex(0x1a1a1a), 0);

    // Title
    lv_obj_t * title_label = lv_label_create(ui_Screen4);
    lv_label_set_text(title_label, "SCREEN 4 - EMPTY");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x00D4FF), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_24, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 20);

    // Subtitle
    lv_obj_t * subtitle_label = lv_label_create((lv_obj_t*)ui_Screen4);
    lv_label_set_text(subtitle_label, "Clean screen for future development");
    lv_obj_set_style_text_color(subtitle_label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(subtitle_label, &lv_font_montserrat_16, 0);
    lv_obj_align(subtitle_label, LV_ALIGN_TOP_MID, 0, 60);

    // Navigation buttons
    // Previous screen button
    lv_obj_t * prev_screen_btn = lv_btn_create(ui_Screen4);
    lv_obj_set_size(prev_screen_btn, 50, 50);
    lv_obj_set_x(prev_screen_btn, 10);
    lv_obj_set_y(prev_screen_btn, 400);
    lv_obj_set_style_bg_color(prev_screen_btn, lv_color_hex(0x00D4FF), 0);
    lv_obj_set_style_radius(prev_screen_btn, 25, 0);
    lv_obj_add_event_cb(prev_screen_btn, screen4_prev_screen_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * prev_icon = lv_label_create(prev_screen_btn);
    lv_label_set_text(prev_icon, "←");
    lv_obj_set_style_text_color(prev_icon, lv_color_white(), 0);
    lv_obj_set_style_text_font(prev_icon, &lv_font_montserrat_20, 0);
    lv_obj_center(prev_icon);

    // Next screen button
    lv_obj_t * next_screen_btn = lv_btn_create(ui_Screen4);
    lv_obj_set_size(next_screen_btn, 50, 50);
    lv_obj_set_x(next_screen_btn, 750);
    lv_obj_set_y(next_screen_btn, 400);
    lv_obj_set_style_bg_color(next_screen_btn, lv_color_hex(0x00D4FF), 0);
    lv_obj_set_style_radius(next_screen_btn, 25, 0);
    lv_obj_add_event_cb(next_screen_btn, screen4_next_screen_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * next_icon = lv_label_create(next_screen_btn);
    lv_label_set_text(next_icon, "→");
    lv_obj_set_style_text_color(next_icon, lv_color_white(), 0);
    lv_obj_set_style_text_font(next_icon, &lv_font_montserrat_20, 0);
    lv_obj_center(next_icon);

    // Previous screen label
    lv_obj_t * prev_label = lv_label_create(ui_Screen4);
    lv_label_set_text(prev_label, "Prev Screen");
    lv_obj_set_style_text_color(prev_label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(prev_label, &lv_font_montserrat_12, 0);
    lv_obj_align_to(prev_label, prev_screen_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    // Next screen label
    lv_obj_t * next_label = lv_label_create(ui_Screen4);
    lv_label_set_text(next_label, "Next Screen");
    lv_obj_set_style_text_color(next_label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(next_label, &lv_font_montserrat_12, 0);
    lv_obj_align_to(next_label, next_screen_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    // Touch cursor
    ui_Touch_Cursor_Screen4 = lv_obj_create(ui_Screen4);
    lv_obj_set_size(ui_Touch_Cursor_Screen4, 20, 20);
    lv_obj_set_style_bg_color(ui_Touch_Cursor_Screen4, lv_color_hex(0x00D4FF), 0);
    lv_obj_set_style_radius(ui_Touch_Cursor_Screen4, 10, 0);
    lv_obj_add_flag(ui_Touch_Cursor_Screen4, LV_OBJ_FLAG_HIDDEN);

    // Add event handlers
    lv_obj_add_event_cb(ui_Screen4, screen4_touch_handler, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_Screen4, screen4_touch_handler, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(ui_Screen4, swipe_handler_screen4, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_Screen4, swipe_handler_screen4, LV_EVENT_RELEASED, NULL);

    ESP_LOGI("SCREEN4", "Screen 4 initialized as clean empty screen");
}

// Screen destroy function
void ui_Screen4_screen_destroy(void) {
    if (ui_Screen4) {
        lv_obj_del(ui_Screen4);
        ui_Screen4 = NULL;
    }
    ESP_LOGI("SCREEN4", "Screen 4 destroyed");
}
