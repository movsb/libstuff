#include "homekit.hpp"

namespace stuff {
namespace protos {
namespace homekit {

void init(const char *setupCode, const char *setupID) {
	hap_cfg_t cfg;
	if(::hap_get_config(&cfg) != HAP_SUCCESS) {
		abort();
	}
	cfg.unique_param = UNIQUE_NAME;
	if (::hap_set_config(&cfg) != HAP_SUCCESS) {
		abort();
	}

	if (::hap_init(HAP_TRANSPORT_WIFI) != HAP_SUCCESS) {
		abort();
	}

	::hap_set_debug_level(HAP_DEBUG_LEVEL_INFO);
	::hap_set_setup_code(setupCode);
	::hap_set_setup_id(setupID);
}


}
}
}
