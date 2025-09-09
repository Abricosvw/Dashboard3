// ECU Dashboard Screen 2 - Additional Gauges
// Second screen accessible by swiping left/right

#ifndef UI_SCREEN2_H
#define UI_SCREEN2_H

#ifdef __cplusplus
extern "C" {
#endif

// SCREEN: ui_Screen2
extern void ui_Screen2_screen_init(void);
void ui_Screen2_update_animations(bool demo_enabled);
extern lv_obj_t * ui_Screen2;

// Additional ECU Gauge Objects
extern lv_obj_t * ui_Arc_Oil_Pressure;
extern lv_obj_t * ui_Arc_Oil_Temp;
extern lv_obj_t * ui_Arc_Water_Temp;
extern lv_obj_t * ui_Arc_Fuel_Pressure;
extern lv_obj_t * ui_Arc_Battery_Voltage;

// Additional Label Objects
extern lv_obj_t * ui_Label_Oil_Pressure_Value;
extern lv_obj_t * ui_Label_Oil_Temp_Value;
extern lv_obj_t * ui_Label_Water_Temp_Value;
extern lv_obj_t * ui_Label_Fuel_Pressure_Value;
extern lv_obj_t * ui_Label_Battery_Voltage_Value;

// Arc visibility control
void ui_Screen2_update_arc_visibility(int arc_index, bool visible);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
