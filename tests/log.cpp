#include <stuff/base/log.hpp>

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <array>

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
	logErr("Error: %v end", log::Blink(log::Green(true)));
	
	std::string s("ABC");
	logWarn("Str: %s", log::Italic(s));
	
	std::vector arr{std::vector{1,2,3}, std::vector{4,5,6}};
	logFatal("Array: %v %v", arr, std::vector<int>{});
	
	std::map<int, const char*> m;
	m[0] = "X000";
	m[1] = "X111";
	logFatal("Map: %v", m);
	
	std::map<const char*, std::vector<int>> si;
	si["A"] = {1,2,3};
	si["B"] = {4,5,6};
	logFatal("Map+Vector: %v", si);
	
	std::pair<int, int> p = {1, 2};
	logFatal("Pair: %v", p);
	
	std::unordered_map<int, int> um;
	um[0] = 1;
	logCyan("unordered_map: %v", um);
	
	std::set<int> set1{1,2,3,4,5,6};
	logPurple("set: %v", set1);
	
	std::array<int,3> a1{1,2,3};
	logYellow("array: %v, %v", a1, &a1);
}
