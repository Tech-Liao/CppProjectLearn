#include "json.h"
#include <iostream>
#include <cassert>
#include <sstream>

#define TEST(name)                                    \
    std::cout << "Testing " #name "... ";             \
    try                                               \
    {                                                 \
        name();                                       \
        std::cout << "✅ PASS\n";                     \
    }                                                 \
    catch (const std::exception &e)                   \
    {                                                 \
        std::cout << "❌ FAIL: " << e.what() << "\n"; \
        return 1;                                     \
    }

// 测试结果统计
int passed = 0, failed = 0;

// ==================== 测试用例 ====================

void test_null()
{
    JsonParser parser("null");
    auto val = parser.parse();
    assert(val->is_null());
    passed++;
}

void test_bool()
{
    assert(JsonParser("true").parse()->get_bool() == true);
    assert(JsonParser("false").parse()->get_bool() == false);
    passed += 2;
}

void test_number()
{
    assert(JsonParser("123.45").parse()->get_number() == 123.45);
    passed++;
}

void test_string()
{
    assert(JsonParser(R"("hello")").parse()->get_string() == "hello");
    passed++;
}

void test_array()
{
    auto arr = JsonParser("[1,2,3]").parse();
    assert(arr->array_size() == 3);
    assert(arr->array_get(1)->get_number() == 2);
    passed += 2;
}

void test_object()
{
    auto obj = JsonParser(R"({"name":"kimi","age":25})").parse();
    assert(obj->object_get("name")->get_string() == "kimi");
    assert(obj->object_get("age")->get_number() == 25);
    passed += 2;
}

void test_nested()
{
    auto obj = JsonParser(R"({"user":{"name":"test"},"items":[1,2,3]})").parse();
    assert(obj->object_get("user")->object_get("name")->get_string() == "test");
    assert(obj->object_get("items")->array_get(2)->get_number() == 3);
    passed += 2;
}

void test_serialize()
{
    assert(JsonValue::make_null()->to_string() == "null");
    assert(JsonValue::make_bool(true)->to_string() == "true");
    assert(JsonValue::make_number(42)->to_string() == "42");
    assert(JsonValue::make_string("hi")->to_string() == "\"hi\"");
    passed += 4;
}
static std::string generate_deep_object(int depth)
{
    std::ostringstream oss;
    for (int i = 0; i < depth; ++i)
        oss << "{\"a\":";
    oss << "{\"end\":\"ok\"}"; // ✅ 最内层带字符串字段
    for (int i = 0; i < depth; ++i)
        oss << "}";
    return oss.str();
}
// ==================== 主函数 ====================

int main()
{
    std::cout << "========== JSON 解析器功能测试 ==========\n\n";

    TEST(test_null);
    TEST(test_bool);
    TEST(test_number);
    TEST(test_string);
    TEST(test_array);
    TEST(test_object);
    TEST(test_nested);
    TEST(test_serialize);
    std::cout << "========== JSON 解析器功能测试 ==========\n\n";
    std::string json(100, '{');
    json += "\"end\":null";
    json.append(100, '}');
    

    std::cout << "\n✅ 测试完成: " << passed << " 断言通过\n";
    std::cout << "建议: valgrind --leak-check=full ./bin/test_json\n";

    return 0;
}