# ports

如何把基础库移植到 ESP32 项目。

在根目录（定义 project 的 cmake 目录）的 CMakeLists.txt 文件中添加内容：

```cmake
add_subdirectory($ENV{STUFF_PATH}/base stuff/base)
```

然后在各种 targets（比如：main） 自身目录的 CMakeLists.txt 中添加如下内容：

```cmake
include($ENV{STUFF_PATH}/ports/esp32/ports/include.cmake)
```
