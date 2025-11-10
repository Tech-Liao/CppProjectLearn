# 生成测试可执行文件
add_executable(test_reverse test_reverse.cpp)
target_link_libraries(test_reverse PRIVATE ministl)

# 测试目标更严格的告警（可选）
if (MSVC)
  target_compile_options(test_reverse PRIVATE /W4 /permissive-)
else()
  target_compile_options(test_reverse PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
endif()

# 集成到 CTest
add_test(NAME reverse_iterator COMMAND test_reverse)