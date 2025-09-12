// ECU Dashboard Screen 5 - ECU Data Gauges (Page 2)
#include "../ui.h"
#include "ui_Screen5.h"
#include "ui_screen_manager.h"
#include "ui_helpers.h"
#include <stdio.h>
#include <esp_log.h>

// Screen object
lv_obj_t * ui_Screen5;

// Gauge Objects
lv_obj_t * ui_Arc_Eng_TQ_Act;
lv_obj_t * ui_Label_Eng_TQ_Act_Value;
lv_obj_t * ui_Arc_Limit_TQ;
lv_obj_t * ui_Label_Limit_TQ_Value;

// Function prototypes
static void swipe_handler_screen5(lv_event_t * e);
static void screen5_prev_screen_btn_event_cb(lv_event_t * e);
static void screen5_next_screen_btn_event_cb(lv_event_t * e);

// Helper function to create a gauge
static void create_gauge(lv_obj_t * parent, lv_obj_t ** arc, lv_obj_t ** label,
                        const char * title, const char * unit, lv_color_t color,
                        int32_t min_val, int32_t max_val, int x, int y)
{
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

    lv_obj_t * label_title = lv_label_create(cont);
    lv_label_set_text(label_title, title);
    lv_obj_set_style_text_color(label_title, lv_color_white(), 0);
    lv_obj_align(label_title, LV_ALIGN_BOTTOM_MID, 0, -15);

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

    *label = lv_label_create(cont);
    lv_label_set_text(*label, "0");
    lv_obj_set_style_text_color(*label, lv_color_white(), 0);
    lv_obj_center(*label);
    lv_obj_align(*label, LV_ALIGN_CENTER, 0, -5);

    lv_obj_t * label_unit = lv_label_create(cont);
    lv_label_set_text(label_unit, unit);
    lv_obj_set_style_text_color(label_unit, lv_color_hex(0xcccccc), 0);
    lv_obj_align_to(label_unit, *label, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
}


// Main screen initialization
void ui_Screen5_screen_init(void) {
    ui_Screen5 = lv_obj_create(NULL);
    lv_obj_set_size(ui_Screen5, 800, 480);
    lv_obj_set_pos(ui_Screen5, 0, 0);
    lv_obj_clear_flag(ui_Screen5, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_Screen5, lv_color_hex(0x1a1a1a), 0);

    lv_obj_t * title_label = lv_label_create(ui_Screen5);
    lv_label_set_text(title_label, "ECU Data (Page 2)");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x00D4FF), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_24, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 10);

    create_gauge(ui_Screen5, &ui_Arc_Eng_TQ_Act, &ui_Label_Eng_TQ_Act_Value, "Eng Tq Act", "Nm", lv_color_hex(0x00D4FF), 0, 500, 15, 60);
    create_gauge(ui_Screen5, &ui_Arc_Limit_TQ, &ui_Label_Limit_TQ_Value, "Torque Limit", "Nm", lv_color_hex(0x00FF88), 0, 500, 285, 60);

    lv_obj_t * prev_screen_btn = lv_btn_create(ui_Screen5);
    lv_obj_set_size(prev_screen_btn, 50, 50);
    lv_obj_align(prev_screen_btn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_set_style_bg_color(prev_screen_btn, lv_color_hex(0x00D4FF), 0);
    lv_obj_set_style_radius(prev_screen_btn, 25, 0);
    lv_obj_add_event_cb(prev_screen_btn, screen5_prev_screen_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * prev_icon = lv_label_create(prev_screen_btn);
    lv_label_set_text(prev_icon, LV_SYMBOL_LEFT);
    lv_obj_center(prev_icon);

    lv_obj_t * next_screen_btn = lv_btn_create(ui_Screen5);
    lv_obj_set_size(next_screen_btn, 50, 50);
    lv_obj_align(next_screen_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_set_style_bg_color(next_screen_btn, lv_color_hex(0x00D4FF), 0);
    lv_obj_set_style_radius(next_screen_btn, 25, 0);
    lv_obj_add_event_cb(next_screen_btn, screen5_next_screen_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * next_icon = lv_label_create(next_screen_btn);
    lv_label_set_text(next_icon, LV_SYMBOL_RIGHT);
    lv_obj_center(next_icon);

    lv_obj_add_event_cb(ui_Screen5, swipe_handler_screen5, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_Screen5, swipe_handler_screen5, LV_EVENT_RELEASED, NULL);

    ESP_LOGI("SCREEN5", "Screen 5 initialized");
}

static void swipe_handler_screen5(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_RELEASED) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_LEFT) ui_switch_to_next_enabled_screen(true);
        if (dir == LV_DIR_RIGHT) ui_switch_to_next_enabled_screen(false);
    }
}

static void screen5_prev_screen_btn_event_cb(lv_event_t * e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) ui_switch_to_next_enabled_screen(false);
}

static void screen5_next_screen_btn_event_cb(lv_event_t * e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) ui_switch_to_next_enabled_screen(true);
}
