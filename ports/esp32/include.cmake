# 注意这里是 PUBLIC，由依赖组件自己实现的，否则会链接失败
target_sources(${COMPONENT_LIB} PUBLIC ${CMAKE_SOURCE_DIR}/_stuff/ports/esp32/stuff.c)
target_include_directories(${COMPONENT_LIB} PRIVATE ${CMAKE_SOURCE_DIR}/_stuff/base/include)
target_link_libraries(${COMPONENT_LIB} PRIVATE base)
