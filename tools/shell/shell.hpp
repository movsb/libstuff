#pragma once

#include <stdint.h>
#include <cstdio>

#include <cstring>
#include <string>
#include <functional>
#include <vector>

/**
 * @brief 一门极简的 Shell-like 命令行脚本语言及其解释器实现。
 * 
 * 你可以把它当作一个类似 Bash/busybox 的东西。主要用于微控制器命令解释及执行。
 * 
 * 语法简介：
 *   - COMMAND [OPTIONS... | ARGS...]
 *   
 *   COMMAND    := ls | echo | date
 *   OPTIONS    := -a | -b | --long-arg
 *   ARGS       := a.txt | "hello world"
 */


namespace stuff {
namespace targets {
namespace esp32 {
namespace tools {
namespace shell {

bool str2int(const char *s, int64_t *value);

inline const char * index() {
	extern const char _index_html_start[] asm("_binary_shell_html_start");
	return _index_html_start;
}

struct CharPtrRef {
	const char *p;
	bool operator==(const char *other) const {
		return std::strcmp(p, other) == 0;
	}
};

typedef std::vector<CharPtrRef> Args;

struct SimpleCommand {
	CharPtrRef      name;
	Args            args;
};

typedef std::function<void(const char *output)> Output;

struct Writer {
	void write(const char *s) {
		_output(s);
	}
	void write(const std::string& s) {
		_output(s.c_str());
	}
private:
	friend class Shell;
	Output _output;
};

struct KnownCommand {
	const char *name;
	const char *description;
	const char *help;
	void (*execute)(const Args& args, Writer &writer);
};

class Shell {
public:
	Shell(KnownCommand *knownCommands, std::function<void(const char *)> error)
		: _error(error) , _known(knownCommands)
		{}
public:
	/**
	 * @brief 解析并执行一段命令脚本。
	 * 
	 * @note  为了省内存，内部会原地修改 script 指向的内存，需是可写的。
	*/
	void eval(char *script, Output output);
private:
	bool parse(char *script, SimpleCommand *cmd);
	void help(Output output, const Args &args);
private:
	void _next();
	void _next2();
private:
	std::function<void(const char *)> _error;
	enum TK {
		Id, Eof, Err, None,
	};
	char *_p;
	bool _expectWS;
	TK _tk;
	char *_tks, *_tke;
	KnownCommand *_known;
};

} // namespace shell
} // namespace tools
} // namespace esp32
} // namespace targets
} // namespace stuff
