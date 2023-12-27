# 注意这里是 PUBLIC，由依赖组件自己实现的，否则会链接失败
file(GLOB SRCS ${CMAKE_CURRENT_LIST_DIR}/*.c ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
set(STUFF_PATH ${CMAKE_CURRENT_LIST_DIR}/../..)
target_sources(${COMPONENT_LIB} PUBLIC ${STUFF_PATH}/ports/esp32/stuff.c)
target_include_directories(${COMPONENT_LIB} PRIVATE ${STUFF_PATH}/base/include)
target_link_libraries(${COMPONENT_LIB} PRIVATE base)
