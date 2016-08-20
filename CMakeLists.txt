project(evdev-test)
cmake_minimum_required(VERSION 2.8)

#add_definitions("-std=c++17")
add_compile_options("-std=c++17")

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} evdev)
