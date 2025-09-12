idf_component_register(SRCS "sd_card_manager.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "driver" "esp_vfs_fat" "sdmmc")
