# Add sources from ui directory (excluding background_task.c to avoid duplication)
file(GLOB_RECURSE SRC_UI "ui/*.c")
# Remove background_task.c from SRC_UI if it was included by glob
list(REMOVE_ITEM SRC_UI "${CMAKE_CURRENT_SOURCE_DIR}/background_task.c")

idf_component_register(
    SRCS "background_task.c" "canbus.c" "web_server.c" "main.c" "ecu_data.c" "can_websocket.c" "serial_reader.c" "mre_parser.c" ${SRC_UI}
    INCLUDE_DIRS "." "ui" "include"
    REQUIRES esp_lcd lvgl driver esp_lcd_touch_gt911 esp_http_server nvs_flash esp_wifi
)