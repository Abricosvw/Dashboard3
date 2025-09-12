// ECU Dashboard Screen 5 - MRE Data Gauges (Page 2)
#ifndef UI_SCREEN5_H
#define UI_SCREEN5_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// SCREEN: ui_Screen5
void ui_Screen5_screen_init(void);
extern lv_obj_t * ui_Screen5;

// MRE Gauge Objects
extern lv_obj_t * ui_Arc_Eng_TQ_Act;
extern lv_obj_t * ui_Arc_Limit_TQ;
extern lv_obj_t * ui_Arc_PID_Corr;

// MRE Label Objects
extern lv_obj_t * ui_Label_Eng_TQ_Act_Value;
extern lv_obj_t * ui_Label_Limit_TQ_Value;
extern lv_obj_t * ui_Label_PID_Corr_Value;


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
