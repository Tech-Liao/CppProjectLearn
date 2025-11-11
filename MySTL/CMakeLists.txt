cmake_minimum_required(VERSION 3.14)
project(MiniSTL LANGUAGES CXX)

# 设置 C++11 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 头文件库（header-only）
add_library(ministl INTERFACE)
message(STATUS "CMAKE_CURRENT_SOURCE_DIR is: ${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(ministl
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(ministl INTERFACE cxx_std_11)

# 警告级别（可以根据需要调整）
if (MSVC)
  target_compile_options(ministl INTERFACE /W4)
else()
  target_compile_options(ministl INTERFACE -Wall -Wextra -Wpedantic)
endif()

# 启用测试
enable_testing()

# 添加子目录
add_subdirectory(src)
add_subdirectory(test)