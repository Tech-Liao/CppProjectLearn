# test/CMakeLists.txt
# ========== 关键3：显式链接线程库 ==========

# ========== 新增：ThreadSanitizer 选项 ==========
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

if(ENABLE_TSAN)
    # 显示当前编译器信息
    message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
    message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")
    
    # 简化检测（不使用check_cxx_compiler_flag）
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message(STATUS "✅ ThreadSanitizer enabled")
        add_compile_options(-fsanitize=thread -g -fno-omit-frame-pointer)
        add_link_options(-fsanitize=thread)
    else()
        message(FATAL_ERROR "❌ ThreadSanitizer requires Clang or GCC")
    endif()
endif()

find_package(Threads REQUIRED)
# 功能测试（纯C++11，无外部依赖）
add_executable(SingleLog
    test_singlelog.cpp
    ${CMAKE_SOURCE_DIR}/src/Log.cpp
)
# ========== 关键1：编译时加 -pthread ==========
add_compile_options(-pthread)

# ========== 关键2：链接时加 -pthread ==========
target_link_libraries(SingleLog PRIVATE Threads::Threads)
target_include_directories(SingleLog PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_features(SingleLog PRIVATE cxx_std_11)



