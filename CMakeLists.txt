cmake_minimum_required(VERSION 3.15.0)
project(test VERSION 0.1.0 LANGUAGES CXX)

find_package(FLTK CONFIG REQUIRED)

add_executable(test main.cpp)
target_link_libraries(test PRIVATE fltk fltk_gl fltk_forms fltk_images)