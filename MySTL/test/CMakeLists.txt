# 生成测试可执行文件
add_executable(test_reverse test_reverse.cpp)
target_link_libraries(test_reverse PRIVATE ministl)

add_executable(test_type_utility test_type_utility.cpp)
target_link_libraries(test_type_utility PRIVATE ministl)
# 测试目标更严格的告警（可选）
if (MSVC)
  target_compile_options(test_reverse PRIVATE /W4 /permissive-)
  target_compile_options(test_revetest_type_utilityrse PRIVATE /W4 /permissive-)
else()
  target_compile_options(test_reverse PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
  target_compile_options(test_type_utility PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
endif()