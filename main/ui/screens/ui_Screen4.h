// ECU Dashboard Screen 4 - Empty Screen
// Fourth screen - clean empty screen for future development

#ifndef UI_SCREEN4_H
#define UI_SCREEN4_H

#ifdef __cplusplus
extern "C" {
#endif

// SCREEN: ui_Screen4
extern void ui_Screen4_screen_init(void);
extern void ui_Screen4_screen_destroy(void);
extern lv_obj_t * ui_Screen4;

// Touch cursor object
extern lv_obj_t * ui_Touch_Cursor_Screen4;

// Touch cursor update function for Screen4
extern void ui_update_touch_cursor_screen4(lv_point_t * point);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
