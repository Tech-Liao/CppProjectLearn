cmake_minimum_required(VERSION 3.14)
project(MiniSTL LANGUAGES CXX)

# C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 头文件库（header-only）
add_library(ministl INTERFACE)
target_include_directories(ministl
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(ministl INTERFACE cxx_std_11)

# 警告级别（可按需调整）
if (MSVC)
  target_compile_options(ministl INTERFACE /W4)
else()
  target_compile_options(ministl INTERFACE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_subdirectory(test)