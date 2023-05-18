#include <stdio.h>

#include <ebp/ebp.hpp>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

using namespace ebp;

static void t(void*)
{
	ESP_ERROR_CHECK( nvs_flash_init() );
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	auto sta = wifi::Station{};
	alt::must(sta.start());
	alt::must(sta.connect("__tao_iot", "[0xTaoIot]"));
	
	auto client = http::Client();
	auto [rsp] = alt::must(client.get("http://192.168.1.86:7899/relative-redirect/1"));
	printf("status code: %d\n", rsp.statusCode());
	
	for (auto &p : rsp.header()) {
		printf("%s: %s\n", p.first.c_str(), p.second.c_str());
	}
	
	io::copy(&os::StdOut, rsp.body());
}

extern "C" void app_main(void)
{
	os::createTask([]{t(nullptr);});
}
