#include "esp_log.h"
#include "esp_pm.h"

static const char *TAG = "power_management";

void pm_init(void) {
#if CONFIG_PM_ENABLE
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 160,
        .min_freq_mhz = 80,
        .light_sleep_enable = false // Disable light sleep to improve WiFi stability
    };

    esp_err_t err = esp_pm_configure(&pm_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure power management: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Power management configured successfully");
    }
#else
    ESP_LOGW(TAG, "Power management is not enabled in sdkconfig");
#endif
}
