#ifndef ESP32_COMPAT_H
#define ESP32_COMPAT_H

#include <Arduino.h>
#if __has_include(<sdkconfig.h>)
#include <sdkconfig.h>
#endif

// Include esp_cpu.h for all ESP32 variants
#if defined(ESP32) || defined(ESP32S2) || defined(ESP32S3) || defined(ESP32C3) || defined(ESP32P4) || defined(CONFIG_IDF_TARGET_ESP32P4)
#include <esp_cpu.h>
#endif

// Include esp_idf_version.h for version checking
#if __has_include(<esp_idf_version.h>)
#include <esp_idf_version.h>
#endif

// ESP-IDF v5.x renamed esp_cpu_get_ccount to esp_cpu_get_cycle_count.
// We use esp_cpu_get_cycle_count in the code.
// For legacy versions (IDF < 5.0), we map it back to esp_cpu_get_ccount.
#if defined(ESP_IDF_VERSION) && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#ifndef esp_cpu_get_cycle_count
#define esp_cpu_get_cycle_count esp_cpu_get_ccount
#endif
#endif

// ESP32-P4 compatibility layer
#ifdef CONFIG_IDF_TARGET_ESP32P4
// P4 Stub if needed (currently using native esp_cpu_get_cycle_count)
#endif

// ArduinoLog compatibility
#ifndef LOG_LEVEL_NONE
#define LOG_LEVEL_NONE LOG_LEVEL_SILENT
#endif

#endif // ESP32_COMPAT_H