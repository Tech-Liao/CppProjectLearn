# test/CMakeLists.txt

# 功能测试（纯C++11，无外部依赖）
add_executable(test_json
    test_json.cpp
    ${CMAKE_SOURCE_DIR}/src/json.cpp
)
target_include_directories(test_json PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_features(test_json PRIVATE cxx_std_11)


# 深度嵌套测试（纯C++11，无外部依赖）
add_executable(deep_json
    test_deep_nesting.cpp
    ${CMAKE_SOURCE_DIR}/src/json.cpp
)
target_include_directories(deep_json PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_features(deep_json PRIVATE cxx_std_11)


# 深度嵌套测试（纯C++11，无外部依赖）
add_executable(test_io_json
    test_io_json.cpp
    ${CMAKE_SOURCE_DIR}/src/json.cpp
)
target_include_directories(test_io_json PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_features(test_io_json PRIVATE cxx_std_11)

# 性能基准测试（依赖Google Benchmark）
find_package(benchmark REQUIRED) # 确保已安装
add_executable(benchmark_json
    benchmark_json.cpp
    ${CMAKE_SOURCE_DIR}/src/json.cpp
)
target_link_libraries(benchmark_json PRIVATE benchmark::benchmark)
target_include_directories(benchmark_json PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_features(benchmark_json PRIVATE cxx_std_11)

# 输出目录统一
set_target_properties(test_json benchmark_json
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
