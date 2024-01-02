#!/bin/bash
#
# This IS the most complicated AWK script I've ever written.

set -euo pipefail


STATUS_GO_RELATIVE=/src/net/http/status.go
STATUS_GO="$(go env GOROOT)${STATUS_GO_RELATIVE}"
DO_NOT_EDIT="// Generated from $STATUS_GO_RELATIVE by using ./$(basename $0). DO NOT EDIT."

function gen_hpp() {
	cat<<CPP
${DO_NOT_EDIT}

#pragma once

namespace stuff {
namespace net {
namespace http {
namespace status {

/**
 * @brief 所有的状态码列表。
 *
 * @note 306 未添加。
 */
enum Code {
CPP

	awk -F'[=/]' '
		/[a-zA-Z]+[ ]*=[ ]*[0-9]+/ {
			gsub(/^\t+| +$/,"",$1);
			gsub(/^\t+| +$/,"",$2);
			printf("\t%s@= %s, // %s\n", substr($1,7), $2, $4);
		}
		' "${STATUS_GO}" | column -ts@
	
	cat<<CPP
};

/**
 * @brief 把状态码转换成带状态码的状态文本。
 *
 * 比如：把 200 转换成 200 OK。
 */
const char* Status(Code code);

/**
 * @brief 把状态码转换成状态文本。
 *
 * 比如：把 200 转换成 OK。
 */
const char* Text(Code code);

} // namespace status
} // namespace http
} // namespace net
} // namespace stuff
CPP
}

function gen_cpp() {
	cat<<CPP
${DO_NOT_EDIT}

#include "status.hpp"

namespace stuff {
namespace net {
namespace http {
namespace status {

const char* Status(Code code) {
	switch (code) {
CPP

	awk -F'[=/]' '
		/[a-zA-Z]+[ ]*=[ ]*[0-9]+/ {
			gsub(/^\t+| +$/,"",$1);
			gsub(/ +/,"",$2);
			cut=substr($1,7);
			map[cut] = $2;
			# printf("map[%s] -> [%s]\n", cut, $2);
		}
		/case Status/ {
			gsub(/[\t]+case Status|:/, "", $0);
			printf("\t\tcase %s:", $0);
			name = $0;
			# printf("save name: %s\n", name);
		}
		/return "[^"]/ {
			gsub(/\t+/, "", $0);
			gsub(/n "/, "n \""map[name]" ", $0);
			printf("@%s;\n", $0);
		}
		END {
			printf("\t\t%s\n", "default:@return \"\";");
			# for (key in map) { print key " => " map[key] }
		}
		' "${STATUS_GO}" | column -ts@
	
	cat<<CPP
	}
}

const char* Text(Code code) {
	auto t = Status(code);
	return *t ? t + 4 : t;
}

} // namespace status
} // namespace http
} // namespace net
} // namespace stuff
CPP
}

gen_hpp > status.hpp
gen_cpp > status.cpp
