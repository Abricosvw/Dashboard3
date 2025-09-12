#include "ui.h"
#include "ui_helpers.h"
#include <stdio.h> // For snprintf

// This function updates all UI elements with the data received from the MRE parser.
void update_mre_data_on_screen(const mre_data_t* data) {
    if (!data) {
        return;
    }

    char buffer[50];

    // --- Update Screen 1 Widgets ---
    // RPM, TPS, MAP, Target Boost (TargetMAP), Wastegate (WG_SET)
    if (lv_obj_is_valid(ui_Arc_RPM) && lv_obj_is_valid(ui_Label_RPM_Value)) {
        lv_arc_set_value(ui_Arc_RPM, data->rpm);
        snprintf(buffer, sizeof(buffer), "%d", data->rpm);
        lv_label_set_text(ui_Label_RPM_Value, buffer);
    }
    if (lv_obj_is_valid(ui_Arc_TPS) && lv_obj_is_valid(ui_Label_TPS_Value)) {
        lv_arc_set_value(ui_Arc_TPS, (int)data->tps);
        snprintf(buffer, sizeof(buffer), "%.1f", data->tps);
        lv_label_set_text(ui_Label_TPS_Value, buffer);
    }
    if (lv_obj_is_valid(ui_Arc_MAP) && lv_obj_is_valid(ui_Label_MAP_Value)) {
        lv_arc_set_value(ui_Arc_MAP, (int)data->map_kpa);
        snprintf(buffer, sizeof(buffer), "%.0f", data->map_kpa);
        lv_label_set_text(ui_Label_MAP_Value, buffer);
    }
    if (lv_obj_is_valid(ui_Arc_Boost) && lv_obj_is_valid(ui_Label_Boost_Value)) {
        lv_arc_set_value(ui_Arc_Boost, (int)data->target_map_kpa);
        snprintf(buffer, sizeof(buffer), "%.0f", data->target_map_kpa);
        lv_label_set_text(ui_Label_Boost_Value, buffer);
    }
    if (lv_obj_is_valid(ui_Arc_Wastegate) && lv_obj_is_valid(ui_Label_Wastegate_Value)) {
        lv_arc_set_value(ui_Arc_Wastegate, (int)data->wg_set_percent);
        snprintf(buffer, sizeof(buffer), "%.0f", data->wg_set_percent);
        lv_label_set_text(ui_Label_Wastegate_Value, buffer);
    }

    // --- Update Screen 4 Widgets ---
    // abs_tps, wg_pos_percent, bov_percent, tcu_tq_nm, tcu_act_nm, eng_trg_nm
    if (lv_obj_is_valid(ui_Arc_Abs_TPS) && lv_obj_is_valid(ui_Label_Abs_TPS_Value)) {
        lv_arc_set_value(ui_Arc_Abs_TPS, (int)data->abs_tps);
        snprintf(buffer, sizeof(buffer), "%.1f", data->abs_tps);
        lv_label_set_text(ui_Label_Abs_TPS_Value, buffer);
    }
    if (lv_obj_is_valid(ui_Arc_WG_Pos) && lv_obj_is_valid(ui_Label_WG_Pos_Value)) {
        lv_arc_set_value(ui_Arc_WG_Pos, (int)data->wg_pos_percent);
        snprintf(buffer, sizeof(buffer), "%.1f", data->wg_pos_percent);
        lv_label_set_text(ui_Label_WG_Pos_Value, buffer);
    }
    if (lv_obj_is_valid(ui_Arc_BOV) && lv_obj_is_valid(ui_Label_BOV_Value)) {
        lv_arc_set_value(ui_Arc_BOV, (int)data->bov_percent);
        snprintf(buffer, sizeof(buffer), "%.1f", data->bov_percent);
        lv_label_set_text(ui_Label_BOV_Value, buffer);
    }
    if (lv_obj_is_valid(ui_Arc_TCU_TQ_Req) && lv_obj_is_valid(ui_Label_TCU_TQ_Req_Value)) {
        lv_arc_set_value(ui_Arc_TCU_TQ_Req, (int)data->tcu_tq_nm);
        snprintf(buffer, sizeof(buffer), "%.0f", data->tcu_tq_nm);
        lv_label_set_text(ui_Label_TCU_TQ_Req_Value, buffer);
    }
    if (lv_obj_is_valid(ui_Arc_TCU_TQ_Act) && lv_obj_is_valid(ui_Label_TCU_TQ_Act_Value)) {
        lv_arc_set_value(ui_Arc_TCU_TQ_Act, (int)data->tcu_act_nm);
        snprintf(buffer, sizeof(buffer), "%.0f", data->tcu_act_nm);
        lv_label_set_text(ui_Label_TCU_TQ_Act_Value, buffer);
    }
    if (lv_obj_is_valid(ui_Arc_Eng_TQ_Req) && lv_obj_is_valid(ui_Label_Eng_TQ_Req_Value)) {
        lv_arc_set_value(ui_Arc_Eng_TQ_Req, (int)data->eng_trg_nm);
        snprintf(buffer, sizeof(buffer), "%.0f", data->eng_trg_nm);
        lv_label_set_text(ui_Label_Eng_TQ_Req_Value, buffer);
    }

    // --- Update Screen 5 Widgets ---
    // eng_act_nm, limit_tq_nm, pid_correction
    if (lv_obj_is_valid(ui_Arc_Eng_TQ_Act) && lv_obj_is_valid(ui_Label_Eng_TQ_Act_Value)) {
        lv_arc_set_value(ui_Arc_Eng_TQ_Act, (int)data->eng_act_nm);
        snprintf(buffer, sizeof(buffer), "%.0f", data->eng_act_nm);
        lv_label_set_text(ui_Label_Eng_TQ_Act_Value, buffer);
    }
    if (lv_obj_is_valid(ui_Arc_Limit_TQ) && lv_obj_is_valid(ui_Label_Limit_TQ_Value)) {
        lv_arc_set_value(ui_Arc_Limit_TQ, (int)data->limit_tq_nm);
        snprintf(buffer, sizeof(buffer), "%.0f", data->limit_tq_nm);
        lv_label_set_text(ui_Label_Limit_TQ_Value, buffer);
    }
    if (lv_obj_is_valid(ui_Arc_PID_Corr) && lv_obj_is_valid(ui_Label_PID_Corr_Value)) {
        lv_arc_set_value(ui_Arc_PID_Corr, (int)data->pid_correction);
        snprintf(buffer, sizeof(buffer), "%.1f", data->pid_correction);
        lv_label_set_text(ui_Label_PID_Corr_Value, buffer);
    }
}
