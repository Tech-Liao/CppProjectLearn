cmake_minimum_required(VERSION 3.16)
project(IM_Server_V1)
# 强制使用11标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 设置头文件搜索路径
include_directories(${CMAKE_SOURCE_DIR}/include)
# 收集src目录下的源文件
set(SRC_FILES
    src/main.cpp
    src/network/TcpServer.cpp
    src/network/EventLoop.cpp
    src/network/Buffer.cpp
    src/network/Connection.cpp
)
# 生成可执行文件
add_executable(im_server ${SRC_FILES})
# 链接多线程库
target_link_libraries(im_server pthread)
