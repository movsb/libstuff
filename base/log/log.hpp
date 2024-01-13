#pragma once

#include <stuff/base/log/printf.hpp>

namespace stuff {
namespace base {
namespace log {

namespace __internal {

template<typename... Args>
int _log_printf(const alts::style::Style &styles, Args&&... args) {
	int n = 0;
	n += alts::printf("", styles);
	n += alts::printf(std::forward<Args&&>(args)...);
	n += alts::printf("", alts::style::reset);
	n += alts::printf("\n");
	return n;
}
	
} // namespace __internal

namespace log {
	using namespace alts::style::predefined;
} // namespace log

} // namespace log
} // namespace base
} // namespace stuff

#define logRed(...)    (stuff::base::log::__internal::_log_printf)(stuff::base::alts::style::red,       __VA_ARGS__)
#define logGreen(...)  (stuff::base::log::__internal::_log_printf)(stuff::base::alts::style::green,     __VA_ARGS__)
#define logYellow(...) (stuff::base::log::__internal::_log_printf)(stuff::base::alts::style::yellow,    __VA_ARGS__)
#define logBlue(...)   (stuff::base::log::__internal::_log_printf)(stuff::base::alts::style::blue,      __VA_ARGS__)
#define logPurple(...) (stuff::base::log::__internal::_log_printf)(stuff::base::alts::style::purple,    __VA_ARGS__)
#define logCyan(...)   (stuff::base::log::__internal::_log_printf)(stuff::base::alts::style::cyan,      __VA_ARGS__)

#define logDebug(...)  (stuff::base::log::__internal::_log_printf)(stuff::base::alts::style::Style{}, __VA_ARGS__)
#define logInfo(...)   (stuff::base::log::__internal::_log_printf)(stuff::base::alts::style::Style{}, __VA_ARGS__)
#define logWarn(...)   logYellow(__VA_ARGS__)
#define logErr(...)    logRed(__VA_ARGS__)
#define logFatal(...)  logRed(__VA_ARGS__)
