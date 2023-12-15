# 注意这里是 PUBLIC，由依赖组件自己实现的，否则会链接失败
target_sources(${COMPONENT_LIB} PUBLIC $ENV{STUFF_PATH}/ports/esp32/ports/stuff.c)
target_include_directories(${COMPONENT_LIB} PRIVATE $ENV{STUFF_PATH}/base/include)
target_link_libraries(${COMPONENT_LIB} PRIVATE base)
