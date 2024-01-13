#include <stuff/base/log.hpp>

using namespace stuff::base::log;

int main() {
	logDebug("Debug");
	logInfo("INFO", log::Underline(log::Bold("ABC")));
	logInfo("INFO: %c", 'A', log::Red(true));
	logRed("Red: %s", "str");
	logWarn("Warn: %s", "str");
	logInfo("Info: %s, %d, %c, end.",
		log::Black("111"),
		log::Yellow(1),
		log::Black('A'),
		log::Bold(log::Underline(true))
	);
	logErr("Error: %v end", log::Green(true));
}
