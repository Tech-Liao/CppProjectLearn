cmake_minimum_required(VERSION 3.16)
project(IM_Server_V1)
# 强制使用11标准
set(CMAKE_CXX_STANDARD 17)
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
    src/network/Codec.cpp
    src/common/Config.cpp
    src/storage/MySQLManager.cpp
    src/business/UserManager.cpp
    src/business/GroupManager.cpp
)
# 生成可执行文件
# 开启 AddressSanitizer 标志
# -fsanitize=address: 开启 ASan
# -g: 包含调试信息（这样报错时能显示具体行号）
# -fno-omit-frame-pointer: 确保堆栈回溯准确
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -fno-omit-frame-pointer")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
add_executable(im_server ${SRC_FILES})
# 链接多线程库与spdlog
target_link_libraries(im_server pthread spdlog mysqlclient redis++ hiredis)
