add_subdirectory(wifi)
add_subdirectory(http)
add_subdirectory(url)

set(LIB_SRCS ${LIB_SRCS} ${_SRCS} PARENT_SCOPE)
