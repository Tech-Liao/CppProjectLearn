#include "json.h"
#include <cctype>
#include <sstream>
#include <iomanip>
#include <fstream>
#include<functional>
// 构造JsonValue的函数.
std::unique_ptr<JsonValue> JsonValue::make_null()
{
    auto ptr = std::unique_ptr<JsonValue>(new JsonValue());
    ptr->m_type = JsonType::JSON_NULL;
    return ptr;
}

std::unique_ptr<JsonValue> JsonValue::make_bool(bool val)
{
    auto ptr = std::unique_ptr<JsonValue>(new JsonValue());
    ptr->m_type = JsonType::JSON_BOOL;
    ptr->m_bool = val;
    return ptr;
}
std::unique_ptr<JsonValue> JsonValue::make_number(double val)
{
    auto ptr = std::unique_ptr<JsonValue>(new JsonValue());
    ptr->m_type = JsonType::JSON_NUMBER;
    ptr->m_number = val;
    return ptr;
}
std::unique_ptr<JsonValue> JsonValue::make_string(std::string val)
{
    auto ptr = std::unique_ptr<JsonValue>(new JsonValue());
    ptr->m_type = JsonType::JSON_STRING;
    ptr->m_string = std::move(val);
    return ptr;
}
std::unique_ptr<JsonValue> JsonValue::make_array()
{
    auto ptr = std::unique_ptr<JsonValue>(new JsonValue());
    ptr->m_type = JsonType::JSON_ARRAY;
    return ptr;
}
std::unique_ptr<JsonValue> JsonValue::make_object()
{
    auto ptr = std::unique_ptr<JsonValue>(new JsonValue());
    ptr->m_type = JsonType::JSON_OBJECT;
    return ptr;
}

// 访问接口
JsonValue *JsonValue::array_get(size_t i) const
{
    if (m_type != JsonType::JSON_ARRAY)
        throw std::runtime_error("错误,此JsonValue不是Array类型");
    if (i >= m_array.size())
        throw std::out_of_range("错误,数组越界");
    return m_array[i].get();
}

JsonValue *JsonValue::object_get(const std::string key) const
{
    if (m_type != JsonType::JSON_OBJECT)
        throw std::runtime_error("错误,此JsonValue不是Object类型");
    auto it = m_object.find(key);
    if (it == m_object.end())
        throw std::out_of_range("key不存在");
    return it->second.get();
}

JsonValue *JsonValue::array_get(size_t i)
{
    if (m_type != JsonType::JSON_ARRAY)
        throw std::runtime_error("错误,此JsonValue不是Array类型");
    if (i >= m_array.size())
        throw std::out_of_range("错误,数组越界");
    return m_array[i].get();
}

JsonValue *JsonValue::object_get(const std::string key)
{
    if (m_type != JsonType::JSON_OBJECT)
        throw std::runtime_error("错误,此JsonValue不是Object类型");
    auto it = m_object.find(key);
    if (it == m_object.end())
        throw std::out_of_range("key不存在");
    return it->second.get();
}

// 修改接口
void JsonValue::array_push_back(std::unique_ptr<JsonValue> value)
{
    if (m_type != JsonType::JSON_ARRAY)
        throw std::runtime_error("错误,此JsonValue不是Array类型");
    m_array.push_back(std::move(value));
}
void JsonValue::object_set(std::string key, std::unique_ptr<JsonValue> value)
{
    if (m_type != JsonType::JSON_OBJECT)
        throw std::runtime_error("错误,此JsonValue不是Object类型");
    m_object[std::move(key)] = std::move(value);
}

// 序列化
std::string JsonValue::to_string() const
{
    switch (m_type)
    {
    case JsonType::JSON_NULL:
        return "null";
    case JsonType::JSON_BOOL:
        return m_bool ? "true" : "false";
    case JsonType::JSON_NUMBER:
    {
        std::ostringstream oss;
        if (static_cast<double>(static_cast<long long>(m_number)) == m_number)
            oss << static_cast<long long>(m_number);
        else
            oss << std::noshowpoint << std::setprecision(6) << m_number;
        return oss.str();
    }
    case JsonType::JSON_STRING:
    {
        std::string result = "\"" + m_string + "\"";
        return result;
    }
    case JsonType::JSON_ARRAY:
    {
        std::string result = "[";
        for (size_t i = 0; i < array_size(); ++i)
        {
            if (i > 0)
                result += ",";
            result += m_array[i]->to_string();
        }
        result += "]";
        return result;
    }
    case JsonType::JSON_OBJECT:
    {
        std::string result = "{";
        size_t count = 0;
        for (const auto &pair : m_object)
        {
            const std::string &key = pair.first;
            const std::unique_ptr<JsonValue> &value = pair.second;
            if (count > 0)
                result += ",";
            count++;
            result += "\"" + key + "\":";
            result += value->to_string();
        }

        result += "}";
        return result;
    }
    default:
        break;
    }
    return "";
}

std::string JsonValue::to_pretty_string(int indent) const
{
    // 负数或 0 统一当作紧凑输出
    if (indent <= 0) return to_string();

    std::ostringstream oss;

    std::function<void(const JsonValue&, int)> dump = [&](const JsonValue& v, int level) {
        switch (v.m_type) {
        case JsonType::JSON_NULL:
            oss << "null"; break;
        case JsonType::JSON_BOOL:
            oss << (v.m_bool ? "true" : "false"); break;
        case JsonType::JSON_NUMBER: {
            std::ostringstream num;
            if (static_cast<double>(static_cast<long long>(v.m_number)) == v.m_number)
                num << static_cast<long long>(v.m_number);
            else
                num << std::noshowpoint << std::setprecision(6) << v.m_number;
            oss << num.str();
            break;
        }
        case JsonType::JSON_STRING:
            oss << "\"" << v.m_string << "\""; break;

        case JsonType::JSON_ARRAY: {
            oss << "[";
            if (!v.m_array.empty()) {
                oss << "\n";
                for (size_t i = 0; i < v.m_array.size(); ++i) {
                    oss << std::string(level + indent, ' ');
                    dump(*v.m_array[i], level + indent);
                    if (i + 1 < v.m_array.size()) oss << ",";
                    oss << "\n";
                }
                oss << std::string(level, ' ');
            }
            oss << "]";
            break;
        }

        case JsonType::JSON_OBJECT: {
            oss << "{";
            if (!v.m_object.empty()) {
                oss << "\n";
                size_t cnt = 0;
                for (const auto& kv : v.m_object) {
                    oss << std::string(level + indent, ' ');
                    oss << "\"" << kv.first << "\": ";
                    dump(*kv.second, level + indent);
                    if (++cnt < v.m_object.size()) oss << ",";
                    oss << "\n";
                }
                oss << std::string(level, ' ');
            }
            oss << "}";
            break;
        }
        }
    };

    dump(*this, 0);
    return oss.str();
}



bool JsonValue::get_bool() const
{
    if (m_type != JsonType::JSON_BOOL)
        throw std::runtime_error("错误：此JsonValue不是Bool类型");
    return m_bool;
}

double JsonValue::get_number() const
{
    if (m_type != JsonType::JSON_NUMBER)
        throw std::runtime_error("错误：此JsonValue不是Number类型");
    return m_number;
}
size_t JsonValue::array_size() const
{
    if (!is_array())
        throw std::runtime_error("错误：此JsonValue不是Array类型");
    return m_array.size();
}
// 对象大小
size_t JsonValue::object_size() const
{
    if (!is_object())
        throw std::runtime_error("错误：此JsonValue不是Object类型");
    return m_object.size();
}

// 字符串访问（需要实现）
const std::string &JsonValue::get_string() const &
{
    if (!is_string())
        throw std::runtime_error("错误：此JsonValue不是String类型");
    return m_string;
}

std::string &&JsonValue::get_string() &&
{
    if (!is_string())
        throw std::runtime_error("...");
    return std::move(m_string); // 允许从临时对象中移动
}
// ==========================
// =========JsonParser=======
// ==========================

std::unique_ptr<JsonValue> JsonParser::parse()
{
    skip_whitespace();
    return parse_value();
}

// 不同数据类型的解析函数
std::unique_ptr<JsonValue> JsonParser::parse_value()
{
    skip_whitespace();
    char c = peek();
    if (c == 'n')
    {
        return parse_null();
    }
    else if (c == 't' || c == 'f')
    {
        return parse_bool();
    }
    else if (c == '-' || std::isdigit(c))
    {
        return parse_number();
    }
    else if (c == '\"')
    {
        return parse_string();
    }
    else if (c == '[')
    {
        return parse_array();
    }
    else if (c == '{')
    {
        return parse_object();
    }
    else
        error("未知数据类型");
}
std::unique_ptr<JsonValue> JsonParser::parse_null()
{
    if (m_text.compare(m_pos, 4, "null") != 0)
        error("期待null");
    for (int i = 0; i < 4; ++i)
        consume();
    return JsonValue::make_null();
}

std::unique_ptr<JsonValue> JsonParser::parse_bool()
{
    if (m_text.compare(m_pos, 4, "true") == 0)
    {
        for (int i = 0; i < 4; ++i)
            consume();
        return JsonValue::make_bool(true);
    }
    if (m_text.compare(m_pos, 5, "false") == 0)
    {
        for (int i = 0; i < 5; ++i)
            consume();
        return JsonValue::make_bool(false);
    }
    error("期待true/false");
}
std::unique_ptr<JsonValue> JsonParser::parse_number()
{
    skip_whitespace(); // 确保从有效字符开始
    size_t start = m_pos;
    // 在skip_whitespace()之后
    char c = peek();
    // 检查负号（可选）
    if (c == '-')
    {
        consume(); // 吃掉'-'
        c = peek();
    }
    // 整数部分（必须）
    if (!std::isdigit(c))
        error("期待数字");
    while (std::isdigit(peek()))
    {
        consume(); // 吃掉所有数字
    }
    // 小数部分（可选）
    if (peek() == '.')
    {
        consume(); // 吃掉'.'
        if (!std::isdigit(peek()))
            error("小数点后必须有数字");
        while (std::isdigit(peek()))
        {
            consume();
        }
    }
    // 指数部分（可选）
    if (peek() == 'e' || peek() == 'E')
    {
        consume(); // 吃掉'e'或'E'

        // 指数符号（可选）
        if (peek() == '+' || peek() == '-')
        {
            consume();
        }

        // 指数数字（必须）
        if (!std::isdigit(peek()))
            error("指数部分必须有数字");
        while (std::isdigit(peek()))
        {
            consume();
        }
    }
    // 计算数字字符串的长度
    size_t length = m_pos - start;

    // 从m_text中提取数字字符串
    std::string num_str = m_text.substr(start, length);

    // 用标准库转为double（它会处理所有格式）
    try
    {
        double value = std::stod(num_str);
        return JsonValue::make_number(value);
    }
    catch (const std::exception &)
    {
        error("非法的数字格式");
    }
}
std::unique_ptr<JsonValue> JsonParser::parse_string()
{
    skip_whitespace(); // 确保从有效字符开始
    if (peek() == '\"')
        consume();
    else
        error("字符串起始引号");
    std::string result;
    while (peek() != '\"')
    {
        if (peek() == '\0')
            error("字符串未闭合");
        skip_whitespace();
        result += consume();
    }
    consume();
    return JsonValue::make_string(std::move(result));
}
std::unique_ptr<JsonValue> JsonParser::parse_array()
{
    skip_whitespace();
    if (peek() != '[')
        error("期待数组类型起始[");
    consume();
    auto array = JsonValue::make_array();
    if (peek() == ']')
    {
        consume();
        return array;
    }
    while (true)
    {
        if (peek() == '\0')
            error("数组未闭合");
        skip_whitespace();
        auto element = parse_value();
        array->array_push_back(std::move(element));
        skip_whitespace();
        char c = peek();
        if (c == ']')
            break;
        else if (c == ',')
            consume();
        else
            error("数组元素后面期待‘,’、或者‘]’");
    }
    consume();
    return array;
}
std::unique_ptr<JsonValue> JsonParser::parse_object()
{
    skip_whitespace();
    if (peek() != '{')
        error("期待对象数据类型起始'{'");
    consume();
    auto object = JsonValue::make_object();
    if (peek() == '}')
    {
        consume();
        return object;
    }
    while (true)
    {
        if (peek() == '\0')
            error("对象未闭合");
        skip_whitespace();
        auto key_val = parse_string();
        std::string key = key_val->get_string();
        skip_whitespace();
        if (peek() != ':')
            error("期待冒号分隔符");
        consume();
        auto value = parse_value();
        object->object_set(std::move(key), std::move(value));
        skip_whitespace();
        if (peek() == '}')
        {
            consume();
            break;
        }
        if (peek() != ',')
            error("对象元素后面期待‘,‘或者‘}'");
        consume();
    }
    return object;
}
// 辅助函数

char JsonParser::peek() const
{
    if (m_pos >= m_text.size())
        return '\0';
    return m_text[m_pos];
}
char JsonParser::consume()
{
    if (m_pos >= m_text.size())
        return '\0';
    char c = m_text[m_pos++];
    if (c == '\n')
    {
        m_line++;
        m_column = 1;
    }
    else
    {
        m_column++;
    }
    return c;
}
void JsonParser::skip_whitespace()
{
    while (std::isspace(peek()))
        consume();
}
[[noreturn]] void JsonParser::error(const std::string &msg)
{
    throw JsonParseError(msg, m_line, m_column);
}

// =====================================================
// 读取 JSON 文件并解析成 JsonValue
// =====================================================
std::unique_ptr<JsonValue> load_json(const std::string &path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open())
    {
        throw std::runtime_error("无法打开 JSON 文件: " + path);
    }

    std::ostringstream ss;
    ss << ifs.rdbuf(); // 读取整个文件内容
    std::string text = ss.str();

    JsonParser parser(text);
    return parser.parse(); // 返回解析后的对象
}

// =====================================================
// 将 JsonValue 转为字符串并写入文件
// =====================================================
void save_json(const JsonValue& value, const std::string& path, bool pretty) {
    std::ofstream ofs(path);
    if (!ofs.is_open())
        throw std::runtime_error("无法写入 JSON 文件: " + path);

    if (pretty)
        ofs << value.to_pretty_string(2);   // ✅ 调用成员函数，不越权
    else
        ofs << value.to_string();
}