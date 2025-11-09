#include "json.h"
#include <benchmark/benchmark.h>
#include <sstream>

// ====================================================
// 辅助函数1：生成大数组 [{"a":0},{"a":1},...]
// ====================================================
static std::string generate_large_array(int size) {
    std::ostringstream oss;
    oss << "[";
    for (int i = 0; i < size; ++i) {
        if (i > 0) oss << ",";
        oss << "{\"a\":" << i << "}";
    }
    oss << "]";
    return oss.str();
}

// ====================================================
// 辅助函数2：生成深度嵌套对象 {"a":{"a":...{"end":null}...}}
// ====================================================
static std::string generate_deep_object(int depth) {
    std::ostringstream oss;
    for (int i = 0; i < depth; ++i)
        oss << "{\"a\":";
    oss << "{\"end\":null}";
    for (int i = 0; i < depth; ++i)
        oss << "}";
    return oss.str();
}

// ====================================================
// 场景1：小对象解析（常见API响应）
// ====================================================
static void BM_Parse_SmallObject(benchmark::State& state) {
    std::string json = R"({
        "status":200,
        "data":{
            "user_id":123,
            "name":"kimi",
            "active":true
        }
    })";

    for (auto _ : state) {
        JsonParser parser(json);
        auto val = parser.parse();
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_Parse_SmallObject);

// ====================================================
// 场景2：大数组解析（性能关键路径）
// ====================================================
static void BM_Parse_LargeArray(benchmark::State& state) {
    std::string json = generate_large_array(state.range(0)); // 可控长度
    for (auto _ : state) {
        JsonParser parser(json);
        auto val = parser.parse();
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_Parse_LargeArray)->Arg(100)->Arg(1000)->Arg(5000);

// ====================================================
// 场景3：深度嵌套对象解析（递归栈开销）
// ====================================================
static void BM_Parse_DeepNested(benchmark::State& state) {
    int depth = static_cast<int>(state.range(0));
    std::string json = generate_deep_object(depth);  // ✅ 合法结构
    for (auto _ : state) {
        JsonParser parser(json);
        auto val = parser.parse();
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_Parse_DeepNested)->Arg(10)->Arg(50)->Arg(100)->Arg(300)->Arg(600);

// ====================================================
// 场景4：长字符串解析（内存与字符扫描性能）
// ====================================================
static void BM_Parse_LongString(benchmark::State& state) {
    std::string large_str(10000, 'a'); // 1万个字符
    std::string json = "{\"content\":\"" + large_str + "\"}";  // ✅ 合法 JSON
    for (auto _ : state) {
        JsonParser parser(json);
        auto val = parser.parse();
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_Parse_LongString);

// ====================================================
// 场景5：序列化性能（to_string测试）
// ====================================================
static void BM_Serialize_Complex(benchmark::State& state) {
    // 构建一个中等复杂的JSON对象
    auto obj = JsonValue::make_object();
    obj->object_set("id", JsonValue::make_number(123));
    obj->object_set("name", JsonValue::make_string("test"));

    // 构建数组字段
    auto arr = JsonValue::make_array();
    for (int i = 0; i < 500; ++i) {
        auto item = JsonValue::make_object();
        item->object_set("index", JsonValue::make_number(i));
        item->object_set("value", JsonValue::make_string("hello"));
        arr->array_push_back(std::move(item));
    }
    obj->object_set("list", std::move(arr));

    for (auto _ : state) {
        auto str = obj->to_string();
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_Serialize_Complex);

// ====================================================
// 场景6：访问性能（对象+数组访问）
// ====================================================
static void BM_Access_Mixed(benchmark::State& state) {
    std::string json = R"({"a":1,"b":[1,2,3,4,5],"user":{"name":"kimi","age":22}})";
    JsonParser parser(json);
    auto root = parser.parse();

    for (auto _ : state) {
        double a = root->object_get("a")->get_number();
        double b = root->object_get("b")->array_get(3)->get_number();
        std::string name = root->object_get("user")->object_get("name")->get_string();
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        benchmark::DoNotOptimize(name);
    }
}
BENCHMARK(BM_Access_Mixed);

// ====================================================
// 主函数
// ====================================================
BENCHMARK_MAIN();