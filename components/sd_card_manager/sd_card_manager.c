#include "include/sd_card_manager.h"
#include "esp_log.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "SD_CARD";

static bool g_can_trace_enabled = false;
static SemaphoreHandle_t sd_card_mutex = NULL;

// Pinout from user's example
#define PIN_NUM_MISO 13
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK  12
#define PIN_NUM_CS   4

#define MOUNT_POINT "/sdcard"

static sdmmc_card_t *s_card;
static sdmmc_host_t s_host = SDSPI_HOST_DEFAULT();

esp_err_t sd_card_init(void) {
    esp_err_t ret;

    ESP_LOGI(TAG, "Initializing SD card");

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    ESP_LOGI(TAG, "Initializing SPI bus...");
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(s_host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ret;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = s_host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &s_host, &slot_config, &mount_config, &s_card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors.", esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, s_card);

    // Create the mutex for thread-safe file access
    sd_card_mutex = xSemaphoreCreateMutex();
    if (sd_card_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create SD card mutex");
        // Cleanup already initialized resources
        esp_vfs_fat_sdcard_unmount(MOUNT_POINT, s_card);
        spi_bus_free(s_host.slot);
        return ESP_ERR_NO_MEM;
    }
    ESP_LOGI(TAG, "SD card mutex created");

    return ESP_OK;
}

esp_err_t sd_card_deinit(void) {
    if (s_card) {
        esp_vfs_fat_sdcard_unmount(MOUNT_POINT, s_card);
        ESP_LOGI(TAG, "SD card unmounted");
    }
    if (sd_card_mutex) {
        vSemaphoreDelete(sd_card_mutex);
        sd_card_mutex = NULL;
    }
    spi_bus_free(s_host.slot);
    ESP_LOGI(TAG, "SPI bus freed");
    return ESP_OK;
}

// Helper function to ensure the directory for a given path exists.
// Note: This is a simplified implementation. For production code, a more
// robust recursive directory creation function would be better.
static esp_err_t ensure_dir_exists(const char *path) {
    // Make a copy of the path to safely manipulate it
    char *dir_path = strdup(path);
    if (dir_path == NULL) {
        ESP_LOGE(TAG, "strdup failed");
        return ESP_ERR_NO_MEM;
    }

    // Find the last '/' to get the directory part of the path
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash != NULL && last_slash != dir_path) { // Check it's not the root slash
        // Terminate the string at the slash to get just the directory path
        *last_slash = '\0';

        // Check if the directory already exists
        struct stat st;
        if (stat(dir_path, &st) != 0) {
            // Directory does not exist, so try to create it
            ESP_LOGI(TAG, "Directory %s does not exist. Creating...", dir_path);
            if (mkdir(dir_path, 0755) != 0) {
                ESP_LOGE(TAG, "Failed to create directory %s", dir_path);
                free(dir_path);
                return ESP_FAIL;
            }
            ESP_LOGI(TAG, "Created directory %s", dir_path);
        }
    }

    // Free the duplicated string
    free(dir_path);
    return ESP_OK;
}

esp_err_t sd_card_write_file(const char* path, const char* data) {
    if (xSemaphoreTake(sd_card_mutex, portMAX_DELAY) == pdTRUE) {
        // Ensure the directory exists before trying to write the file
        if (ensure_dir_exists(path) != ESP_OK) {
            xSemaphoreGive(sd_card_mutex);
            return ESP_FAIL;
        }

        ESP_LOGI(TAG, "Writing file: %s", path);
        FILE *f = fopen(path, "w");
        if (f == NULL) {
            ESP_LOGE(TAG, "Failed to open file for writing");
            xSemaphoreGive(sd_card_mutex);
            return ESP_FAIL;
        }
        fprintf(f, "%s", data);
        fclose(f);
        ESP_LOGI(TAG, "File written");
        xSemaphoreGive(sd_card_mutex);
        return ESP_OK;
    }
    return ESP_ERR_TIMEOUT;
}

esp_err_t sd_card_append_file(const char* path, const char* data) {
    if (xSemaphoreTake(sd_card_mutex, portMAX_DELAY) == pdTRUE) {
        // Ensure the directory exists before trying to append to the file
        if (ensure_dir_exists(path) != ESP_OK) {
            xSemaphoreGive(sd_card_mutex);
            return ESP_FAIL;
        }

        ESP_LOGD(TAG, "Appending to file: %s", path);
        FILE *f = fopen(path, "a");
        if (f == NULL) {
            ESP_LOGE(TAG, "Failed to open file for appending");
            xSemaphoreGive(sd_card_mutex);
            return ESP_FAIL;
        }
        fprintf(f, "%s", data);
        fclose(f);
        xSemaphoreGive(sd_card_mutex);
        return ESP_OK;
    }
    return ESP_ERR_TIMEOUT;
}

void sd_card_set_can_trace_enabled(bool enabled) {
    g_can_trace_enabled = enabled;
    ESP_LOGI(TAG, "CAN trace logging %s", enabled ? "enabled" : "disabled");
}

bool sd_card_is_can_trace_enabled(void) {
    return g_can_trace_enabled;
}
