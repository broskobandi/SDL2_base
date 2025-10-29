cmake_minimum_required(VERSION 3.20)
project(SDL2_base)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(test test/test.cpp)
target_link_libraries(test PRIVATE SDL2 ctest)
target_compile_options(
	test PRIVATE -Wall -Wextra -Werror -Wunused-result -Wconversion
)
target_include_directories(test PRIVATE include)

install(FILES include/SDL2_base.hpp DESTINATION include)
