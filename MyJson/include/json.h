#include<iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
enum class JsonType
{
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
};

class JsonValue
{
public:
    JsonValue() = default;
    JsonValue(JsonValue &&) noexcept = default;
    JsonValue &operator=(JsonValue &&) noexcept = default;
    ~JsonValue() = default;

private:
    JsonValue(const JsonValue &) = delete;
    JsonValue &operator=(const JsonValue &) = delete;

public:
    // 构造JsonValue的函数.
    static std::unique_ptr<JsonValue> make_null();
    static std::unique_ptr<JsonValue> make_bool(bool);
    static std::unique_ptr<JsonValue> make_number(double);
    static std::unique_ptr<JsonValue> make_string(std::string);
    static std::unique_ptr<JsonValue> make_array();
    static std::unique_ptr<JsonValue> make_object();
    // 辅助函数
    bool is_null() const { return m_type == JsonType::JSON_NULL; }
    bool is_bool() const { return m_type == JsonType::JSON_BOOL; }
    bool is_number() const { return m_type == JsonType::JSON_NUMBER; }
    bool is_string() const { return m_type == JsonType::JSON_STRING; }
    bool is_array() const { return m_type == JsonType::JSON_ARRAY; }
    bool is_object() const { return m_type == JsonType::JSON_OBJECT; }
    bool get_bool() const;
    double get_number() const;
    size_t array_size() const;
    size_t object_size() const;
    const std::string &get_string() const &;
    std::string&& get_string() &&; 

    // 访问接口
    JsonValue *array_get(size_t i) const;
    JsonValue *array_get(size_t i);
    JsonValue *object_get(const std::string key);
    JsonValue *object_get(const std::string key) const;

    // 修改接口
    void array_push_back(std::unique_ptr<JsonValue> value);
    void object_set(std::string key, std::unique_ptr<JsonValue> value);

    // 序列化
    std::string to_string() const;
    std::string to_pretty_string(int indent = 2) const;  // 新增：漂亮打印

private:
    JsonType m_type;
    bool m_bool;
    double m_number;
    std::string m_string;
    std::vector<std::unique_ptr<JsonValue>> m_array;
    std::unordered_map<std::string, std::unique_ptr<JsonValue>> m_object;
};

class JsonParser
{

public:
    explicit JsonParser(const std::string &text):m_text(text){}
    std::unique_ptr<JsonValue> parse();

private:
    // 不同数据类型的解析函数
    std::unique_ptr<JsonValue> parse_value();
    std::unique_ptr<JsonValue> parse_null();
    std::unique_ptr<JsonValue> parse_bool();
    std::unique_ptr<JsonValue> parse_number();
    std::unique_ptr<JsonValue> parse_string();
    std::unique_ptr<JsonValue> parse_array();
    std::unique_ptr<JsonValue> parse_object();
    // 辅助函数
    char peek() const;
    char consume();
    void skip_whitespace();
    [[noreturn]] void error(const std::string &msg);

private:
    const std::string &m_text;
    size_t m_pos = 0;
    size_t m_line = 1;
    size_t m_column = 1;
};

// 自定义异常类
class JsonParseError : public std::runtime_error {
    size_t line_, column_;
public:
    JsonParseError(const std::string& msg, size_t line, size_t column)
        : std::runtime_error(msg), line_(line), column_(column) {}
    
    size_t line() const { return line_; }
    size_t column() const { return column_; }
};

// ==========================
// JSON 文件 I/O 辅助函数
// ==========================

// 从文件加载 JSON 内容并解析
std::unique_ptr<JsonValue> load_json(const std::string& path);

// 将 JsonValue 序列化并保存为文件
void save_json(const JsonValue& value, const std::string& path, bool pretty = false);