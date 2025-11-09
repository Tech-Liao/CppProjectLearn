cmake_minimum_required(VERSION 3.10)

project(Json)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(SOURCES
    src/json.cpp
)


# 4. 创建可执行文件（主程序）
#    如果你现在只想编译测试程序，就用这个
add_executable(json_main ${SOURCES} test/test_json.cpp)

enable_testing()
add_subdirectory(test)  # 这行必须加
# 5. 如果你之后想做成库（推荐）
#    add_library(myjson STATIC ${SOURCES})
#    target_include_directories(myjson PUBLIC include)

# 6. 如果你用Google Test，可以在这里添加
#    find_package(GTest REQUIRED)
#    target_link_libraries(json_test GTest::GTest GTest::Main)