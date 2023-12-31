#include <stuff/net/url.hpp>

int main() {
	auto [v, b] = stuff::net::url::parseQuery("a=b&&c=d%25X");
	for (auto kv : v) {
		std::printf("%s: %s\n", kv.first.c_str(), kv.second.c_str());
	}
	return 0;
}

