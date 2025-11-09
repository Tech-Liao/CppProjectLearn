#include <iostream>
#include <sstream>
#include <chrono>
#include "../include/json.h"

// ================================
// 工具函数：生成深度嵌套对象
// ================================
static std::string generate_deep_object(int depth) {
    std::ostringstream oss;
    for (int i = 0; i < depth; ++i)
        oss << "{\"a\":";
    oss << "null";
    for (int i = 0; i < depth; ++i)
        oss << "}";
    return oss.str();
}

// ================================
// 工具函数：生成深度嵌套数组
// ================================
static std::string generate_deep_array(int depth) {
    std::ostringstream oss;
    for (int i = 0; i < depth; ++i)
        oss << "[";
    oss << "null";
    for (int i = 0; i < depth; ++i)
        oss << "]";
    return oss.str();
}

// ================================
// 测试函数
// ================================
static void test_deep_nesting(int depth, bool use_object = true) {
    std::string json = use_object ? generate_deep_object(depth)
                                  : generate_deep_array(depth);

    std::cout << "\n=== 测试深度: " << depth
              << " (" << (use_object ? "对象" : "数组") << ") ===\n";

    try {
        auto start = std::chrono::high_resolution_clock::now();

        JsonParser parser(json);
        auto value = parser.parse();

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        std::cout << "✅ 解析成功，耗时: "
                  << duration.count() << " 秒" << std::endl;
    } catch (const JsonParseError &e) {
        std::cerr << "❌ 解析失败: " << e.what()
                  << " (行 " << e.line() << ", 列 " << e.column() << ")\n";
    } catch (const std::exception &e) {
        std::cerr << "❌ 异常: " << e.what() << std::endl;
    }
}

// ================================
// 主函数入口
// ================================
int main() {
    std::cout << "=== JSON 深度嵌套解析测试 ===" << std::endl;

    // 测试对象类型
    for (int depth : {10, 50, 100, 300, 600, 1000}) {
        test_deep_nesting(depth, true);
    }

    // 测试数组类型
    for (int depth : {10, 50, 100, 300, 600, 1000}) {
        test_deep_nesting(depth, false);
    }

    return 0;
}