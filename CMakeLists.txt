# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "1.3")  
add_definitions(-DPROJECT_VER="${PROJECT_VER}")
set(PROJECT_NAME "EG915_LIB")  
add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(${PROJECT_NAME})