#include "ui_updates.h"
#include "ui.h"
#include "ecu_data.h"
#include <stdio.h>

// This function is called periodically by the LVGL task.
// It reads the latest data from the global ECU data struct
// and updates all the gauge widgets on all screens.
void update_all_gauges(void) {
    ecu_data_t data_copy;

    // Get a thread-safe copy of the latest ECU data
    ecu_data_get_copy(&data_copy);

    char buffer[50];

    // --- Update Screen 1 Widgets ---
    if (lv_obj_is_valid(ui_Arc_RPM)) {
        lv_arc_set_value(ui_Arc_RPM, (int16_t)data_copy.engine_rpm);
        lv_label_set_text_fmt(ui_Label_RPM_Value, "%d", (int)data_copy.engine_rpm);
    }
    if (lv_obj_is_valid(ui_Arc_TPS)) {
        lv_arc_set_value(ui_Arc_TPS, (int16_t)data_copy.tps_position);
        lv_label_set_text_fmt(ui_Label_TPS_Value, "%.1f", data_copy.tps_position);
    }
    if (lv_obj_is_valid(ui_Arc_MAP)) {
        lv_arc_set_value(ui_Arc_MAP, (int16_t)data_copy.map_kpa);
        lv_label_set_text_fmt(ui_Label_MAP_Value, "%.0f", data_copy.map_kpa);
    }
    // NOTE: Target Boost gauge is not updated from CAN spec, it was a simulated value.
    // I will leave it alone for now. The user spec has no "Target MAP".
    // Actually, the old parser had TargetMAP. The new one does not seem to have it.
    // Let me re-check the spec.
    // The user's spec does not mention a target MAP or target boost.
    // The old `ecu_data_t` had `target_boost`. The new one does not.
    // So I will not update the boost gauge.

    // --- Update Screen 4 Widgets ---
    if (lv_obj_is_valid(ui_Arc_Abs_Pedal)) {
        lv_arc_set_value(ui_Arc_Abs_Pedal, (int16_t)data_copy.abs_pedal_pos);
        lv_label_set_text_fmt(ui_Label_Abs_Pedal_Value, "%.1f", data_copy.abs_pedal_pos);
    }
    if (lv_obj_is_valid(ui_Arc_WG_Pos)) {
        lv_arc_set_value(ui_Arc_WG_Pos, (int16_t)data_copy.wg_pos_percent);
        lv_label_set_text_fmt(ui_Label_WG_Pos_Value, "%.1f", data_copy.wg_pos_percent);
    }
    if (lv_obj_is_valid(ui_Arc_BOV)) {
        lv_arc_set_value(ui_Arc_BOV, (int16_t)data_copy.bov_percent);
        lv_label_set_text_fmt(ui_Label_BOV_Value, "%.1f", data_copy.bov_percent);
    }
    if (lv_obj_is_valid(ui_Arc_TCU_TQ_Req)) {
        lv_arc_set_value(ui_Arc_TCU_TQ_Req, (int16_t)data_copy.tcu_tq_req_nm);
        lv_label_set_text_fmt(ui_Label_TCU_TQ_Req_Value, "%.0f", data_copy.tcu_tq_req_nm);
    }
    if (lv_obj_is_valid(ui_Arc_TCU_TQ_Act)) {
        lv_arc_set_value(ui_Arc_TCU_TQ_Act, (int16_t)data_copy.tcu_tq_act_nm);
        lv_label_set_text_fmt(ui_Label_TCU_TQ_Act_Value, "%.0f", data_copy.tcu_tq_act_nm);
    }
    if (lv_obj_is_valid(ui_Arc_Eng_TQ_Req)) {
        lv_arc_set_value(ui_Arc_Eng_TQ_Req, (int16_t)data_copy.eng_trg_nm);
        lv_label_set_text_fmt(ui_Label_Eng_TQ_Req_Value, "%.0f", data_copy.eng_trg_nm);
    }

    // --- Update Screen 5 Widgets ---
    if (lv_obj_is_valid(ui_Arc_Eng_TQ_Act)) {
        lv_arc_set_value(ui_Arc_Eng_TQ_Act, (int16_t)data_copy.eng_act_nm);
        lv_label_set_text_fmt(ui_Label_Eng_TQ_Act_Value, "%.0f", data_copy.eng_act_nm);
    }
    if (lv_obj_is_valid(ui_Arc_Limit_TQ)) {
        lv_arc_set_value(ui_Arc_Limit_TQ, (int16_t)data_copy.limit_tq_nm);
        lv_label_set_text_fmt(ui_Label_Limit_TQ_Value, "%.0f", data_copy.limit_tq_nm);
    }
}
