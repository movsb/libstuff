# 一个强类型、易用、带样式的 printf / 日志库实现

替换 printf 的一种强类型实现。接收任意个数的参数并输出。

别看重载函数非常多，其实只对外暴露了一个函数：

```cpp
template<typename... Args>
int printf(const char* fmt, Args&&... args);
```

以及一些预定义的样式：`Black`, `Red`, `Green`...

所有以下划线开头的函数（比如：_printf）外部均不可使用。

强类型的好处之一是：打印 `int`、`int8`、`int16`、`int32`、`int64` 不用再区分 `%d`、`%ld`、`%u` `%llu` 之类的了，可以统一成 `%d`。
另外一个好处是：可以打印对象（比如包含 `toString` 的对象）。

支持格式控制：样式、颜色。

## 示例

```cpp
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
```

## 类型支持

- [X] `bool`
- [X] `char`, `unsigned char`, `char32_t`
- [X] `int`, `unsigned int`
- [X] `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
- [X] `const char*`
- [X] 任何指针（输出地址）
- [X] 带 `const char* toString()` 方法的结构体或类

## 格式支持

- 常规：

  - [X] %% 字面值 %
  - [X] %v 值的默认格式

- 布尔（%v = %t）：

  - [X] %t 布尔值：true / false

- 字符（%v = %c）：

  - [X] %c 对应的 Unicode 符号。

- 整数（%v = %d）：

  - [X] %b 整数的二进制
  - [o] %o 整数的八进制
  - [X] %d 整数的十进制
  - [X] %x 十六进制（小写）
  - [X] %X 十六进制（大写）
  - [X] %c Unicode 码点值
  
- 浮点数：

  - [ ] %f

- 字符串（%v = %s）：

  - [X] %s 字符串
  - [ ] %X
  - [ ] %x

- 指针（%v = %p）：

  - [X] %p 0x 表示的十六进制，固定显示为指针长度，大写字母。

## 样式支持

- 颜色：黑色、白色、红色、绿色、黄色、蓝色、紫色、青色。
- 字体：加粗、下划线。

## TODO

- 整理单元测试。

## Bugs

可能是 Bugs，也可能是设计如此。

- 不支持类似 %[0]s 的表示。因为实现方式使用了 C++ 的模板展开，无法提前和延迟获取到顺序不一致的参数。
  无解，但是很少被人知道/使用。
- 多次 printf 调用之间的样式无法嵌套。否则需要递归加状态，实现意义是乎不是很大。
