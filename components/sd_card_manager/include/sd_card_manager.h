#ifndef SD_CARD_MANAGER_H
#define SD_CARD_MANAGER_H

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the SD card and mounts the FAT filesystem.
 *
 * This function configures the SPI bus, initializes the SD card in SPI mode,
 * and mounts the filesystem at "/sdcard". This must be called once at startup.
 *
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t sd_card_init(void);

/**
 * @brief Deinitializes the SD card and unmounts the filesystem.
 *
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t sd_card_deinit(void);

/**
 * @brief Writes data to a file on the SD card, overwriting the file if it exists.
 *
 * @param path Full path to the file (e.g., "/sdcard/myfile.txt").
 * @param data The string data to write to the file.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t sd_card_write_file(const char* path, const char* data);

/**
 * @brief Appends data to a file on the SD card. Creates the file if it doesn't exist.
 *
 * @param path Full path to the file (e.g., "/sdcard/log.txt").
 * @param data The string data to append to the file.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t sd_card_append_file(const char* path, const char* data);

/**
 * @brief Enables or disables the logging of CAN bus traces to the SD card.
 *
 * @param enabled true to enable, false to disable.
 */
void sd_card_set_can_trace_enabled(bool enabled);

/**
 * @brief Checks if CAN bus trace logging is currently enabled.
 *
 * @return true if enabled, false otherwise.
 */
bool sd_card_is_can_trace_enabled(void);


#ifdef __cplusplus
}
#endif

#endif // SD_CARD_MANAGER_H
