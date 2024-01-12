#pragma once

#include <stuff/base/log/printf.hpp>

namespace stuff {
namespace base {
namespace log {

// https://gist.github.com/JBlond/2fea43a3049b38287e5e9cefc87b2124
#define __CSI_BEG   "\033["
#define __CSI_END   "m"
#define __CSI_RST   (__CSI_BEG "0" __CSI_END)

#define __RESET     "0"
#define __BLACK     "30"
#define __RED       "31"
#define __GREEN     "32"
#define __YELLOW    "33"
#define __BLUE      "34"
#define __PURPLE    "35"
#define __CYAN      "36"
#define __WHITE     "37"

#define __REGULAR   "0"
#define __BOLD      "1"
#define __DELIM     ";"

#define __REGULAR_COLOR(c) (__CSI_BEG __REGULAR __DELIM c __CSI_END)
#define __BOLD_COLOR(c) (__CSI_BEG __BOLD __DELIM c __CSI_END)

using namespace stuff::base::alts;

template<typename... Args>
int _log_printf(const char *attr, Args&&... args) {
	int n = 0;
	n += _outputStr(attr);
	n += printf(std::forward<Args&&>(args)...);
	if (*attr) {
		n += _outputStr(__CSI_RST);
	}
	return n;
}

} // namespace log
} // namespace base
} // namespace stuff

#define logRed(...)    (stuff::base::log::_log_printf)(__REGULAR_COLOR(__RED),     __VA_ARGS__)
#define logGreen(...)  (stuff::base::log::_log_printf)(__REGULAR_COLOR(__GREEN),   __VA_ARGS__)
#define logYellow(...) (stuff::base::log::_log_printf)(__REGULAR_COLOR(__YELLOW),  __VA_ARGS__)
#define logBlue(...)   (stuff::base::log::_log_printf)(__REGULAR_COLOR(__BLUE),    __VA_ARGS__)
#define logPurple(...) (stuff::base::log::_log_printf)(__REGULAR_COLOR(__PURPLE),  __VA_ARGS__)
#define logCyan(...)   (stuff::base::log::_log_printf)(__REGULAR_COLOR(__CYAN),    __VA_ARGS__)

#define logDebug(...)  (stuff::base::log::_log_printf)("",                         __VA_ARGS__)
#define logInfo(...)   (stuff::base::log::_log_printf)("",                         __VA_ARGS__)
#define logWarn(...)   logYellow(__VA_ARGS__)
#define logErr(...)    logRed(__VA_ARGS__)
#define logFatal(...)  logRed(__VA_ARGS__)
