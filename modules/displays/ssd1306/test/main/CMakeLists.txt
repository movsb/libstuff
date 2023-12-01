idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS ".")



include_directories(../../..)
target_link_libraries(${COMPONENT_LIB} ssd1306)
