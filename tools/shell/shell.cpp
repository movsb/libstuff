#include "shell.hpp"
#include <cstring>

#include <esp_log.h>

namespace stuff {
namespace targets {
namespace esp32 {
namespace tools {
namespace shell {
	
bool str2int(const char *s, int64_t *value) {
	if (s[0] == ' ' || s[0] == '\t') {
		return false;
	}
	char c;
	int n = std::sscanf(s, "%lld%c", value, &c);
	return n == 1;
}

void Shell::help(Output output, const Args &args) {
	auto n = args.size();

	if (n == 0) {
		char buf[512];
		output("<b>help - 帮助</b>\n\n");
		output("输入具体的命令可以查看对应的帮助文档。\n\n");
		for (auto p = _known; p->name; p++) {
			std::snprintf(buf, sizeof(buf),
				"\t<b>%s</b>\t\t%s\n",
				p->name, p->description
			);
			output(buf);
		}
		output("\n");
		return;
	}
	else if (n > 1) {
		output("<red>只能同时查看一个命令的帮助。</red>\n");
	}
}

void Shell::eval(char *script, Output output) {
	SimpleCommand cmd;
	if (parse(script, &cmd)) {
		if (cmd.name == "help") {
			return help(output, cmd.args);
		}
		for (auto p = _known; p->name; p++) {
			if (cmd.name == p->name) {
				Writer w;
				w._output = output;
				p->execute(cmd.args, w);
				return;
			}
		}
		output("<red>找不到这个命令。</red>\n");
	}
}

bool Shell::parse(char *script, SimpleCommand *out) {
	_p = script;
	_expectWS = false;

	auto &cmd = *out;
	_next();
	if (_tk == Id) {
		cmd.name = {_tks};
		auto tke = _tke;
		_next();
		*tke = 0;
		while(_tk == Id) {
			cmd.args.push_back({_tks});
			auto tke = _tke;
			_next();
			*tke = 0;
		}
		if (_tk ==None) {
			return true;
		}
	}
	if (_tk == None) {
		return false;
	} else if (_tk == Eof) {
		_error("unexpected end of command");
		return false;
	} else if (_tk == Err) {
		_error("error parsing command");
		return false;
	} else {
		_error("unhandled error occurred");
		return false;
	}
}

void Shell::_next() {
	char *pp = _p;
	while(*_p == ' ') {
		++_p;
	}
	if (_expectWS && pp == _p && *_p) {
		_tk = Err;
		return;
	}
	_expectWS = false;
	return _next2();
}

void Shell::_next2() {
	auto isAlnum = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'); };
	auto isAllowed = [isAlnum](char c) { return isAlnum(c) || c == '-' || c == '.' || c == '_' || c == '*' || c > 127; };

	if (isAllowed(*_p)) {
		_tks = _p;
		while(isAllowed(*_p)) {
			++_p;
		}
		_expectWS = true;
		_tke = _p;
		_tk = Id;
		return;
	}
	if (char c = *_p; c == '\'' || c == '"') {
		_tks = ++_p;
		while(*_p != c && *_p != 0) {
			++_p;
		}
		if (*_p == 0) {
			_tk = Eof;
			return;
		}
		_tke = _p++;
		_expectWS = true;
		_tk = Id;
		return;
	} else if (!*_p) {
		_tk = None;
		return;
	}
	
	_tks = _p;
	_tk = Err;
}

} // namespace shell
} // namespace tools
} // namespace esp32
} // namespace targets
} // namespace stuff
