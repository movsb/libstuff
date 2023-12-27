#include <ebp/io.hpp>

namespace ebp {
namespace io {
	
static constexpr int copyBufferSize = 64;

_EofReader _eofReader;
Reader* EofReader = &_eofReader;

std::tuple<int, esp_err_t> copy(Writer *dst, Reader& src)
{
	// TODO: WriteTo & ReadFrom

	uint8_t buf[copyBufferSize];
	
	int64_t written = 0;
	esp_err_t err = ESP_OK;
	
	for (;;) {
		auto [nr, er] = src.read(&buf[0], std::size(buf));
		if (nr > 0) {
			auto [nw, ew] = dst->write(buf, nr);
			if (nw < 0 || nr < nw) {
				nw = 0;
				if (ew == ESP_OK) {
					ew = ESP_ERR_INVALID_STATE;
				}
			}
			written += nw;
			if (ew != ESP_OK) {
				ESP_LOGE("io", "error writing: %d", ew);
				err = ew;
				break;
			}
			if (nr != nw) {
				ESP_LOGE("io", "short write: nr=%d, nw=%d", nr, nw);
				err = ESP_ERR_NOT_FINISHED;
				break;
			}
		}
		if (er != ESP_OK) {
			if (er != EOF) {
				ESP_LOGE("io", "error reading: %d", er);
				err = er;
			}
			break;
		}
	}
	
	return {written ,err};
}

}
}