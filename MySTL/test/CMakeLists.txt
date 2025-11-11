# test 目录下的 CMakeLists.txt

# 创建测试可执行文件 test_reverse
add_executable(test_reverse test_reverse.cpp)
target_link_libraries(test_reverse PRIVATE ministl)

# 创建测试可执行文件 test_type_utility
add_executable(test_type_utility test_type_utility.cpp)
target_link_libraries(test_type_utility PRIVATE ministl)

# 创建测试可执行文件 test_vector，并链接 MyVectorLib 和 ministl
add_executable(test_vector test_vector.cpp)
target_link_libraries(test_vector PRIVATE ministl)

# 添加测试
add_test(NAME test_reverse COMMAND test_reverse)
add_test(NAME test_type_utility COMMAND test_type_utility)
add_test(NAME test_vector COMMAND test_vector)

# 设置更严格的编译警告
if (MSVC)
  target_compile_options(test_reverse PRIVATE /W4 /permissive-)
  target_compile_options(test_type_utility PRIVATE /W4 /permissive-)
  target_compile_options(test_vector PRIVATE /W4 /permissive-)
else()
  target_compile_options(test_reverse PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
  target_compile_options(test_type_utility PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
  target_compile_options(test_vector PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
endif()