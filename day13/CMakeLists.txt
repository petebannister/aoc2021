cmake_minimum_required(VERSION 3.10)
project(aoc)
set(CMAKE_CXX_STANDARD 17)
add_executable(aoc 
	"${CMAKE_CURRENT_LIST_DIR}/main.cpp" 
	"${CMAKE_CURRENT_LIST_DIR}/utils.h"
	)
target_compile_definitions(aoc PUBLIC -D_SECURE_SCL=0)
target_include_directories(aoc PUBLIC ${CMAKE_CURRENT_LIST_DIR})

configure_file("${CMAKE_CURRENT_LIST_DIR}/example.txt" "${CMAKE_BINARY_DIR}/"  COPYONLY)
configure_file("${CMAKE_CURRENT_LIST_DIR}/input.txt" "${CMAKE_BINARY_DIR}/"  COPYONLY)