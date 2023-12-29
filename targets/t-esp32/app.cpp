#include "app.hpp"

#include <cstring>

#include <functional>

#include <esp_log.h>
#include <esp_https_ota.h>
#include <esp_ota_ops.h>

namespace stuff {
namespace targets {
namespace esp32 {
namespace app {

static const char endpoint[] = "http://hub.home.twofei.com/v1/ota/firmware";

void OTA::execute(std::function<void()> fn) {
	auto desc = ::esp_ota_get_app_description();
	char url[0
		+ sizeof(endpoint) 
		+ sizeof('?') + sizeof("name=") + sizeof(desc->project_name)
		+ sizeof('&') + sizeof("arch=") + 32
		+ sizeof('&') + sizeof("version=") + sizeof(desc->version)
	];
	std::strcpy(url, endpoint);
	std::strcat(url, "?name=");
	std::strcat(url, desc->project_name);
	std::strcat(url, "&arch=esp8266");
	std::strcat(url, "&version=");
	std::strcat(url, desc->version);
	
	ESP_LOGI("OTA", "URL: %s", url);
	
	esp_http_client_config_t config;
	std::memset(&config, 0, sizeof(config));
	config.url = url;
	config.skip_cert_common_name_check = true;
	auto err = ::esp_https_ota(&config);
	ESP_LOGI("ota", "固件升级：%d", err);
	if (err == ESP_OK) {
		fn();
	}
}

}
}
}
}
