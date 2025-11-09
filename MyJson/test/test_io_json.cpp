#include "json.h"
#include <iostream>

int main() {
    try {
        // 1️⃣ 从文件加载 JSON
        auto root = load_json("/root/code/C++/input.json");

        // 2️⃣ 修改字段
        root->object_set("author", JsonValue::make_string("wildrage"));
        root->object_set("active", JsonValue::make_bool(true));

        // 3️⃣ 保存为另一个文件（美化输出）
        save_json(*root, "output_pretty.json", true);

        // 4️⃣ 保存为紧凑格式
        save_json(*root, "output_compact.json", false);

        std::cout << "✅ JSON 修改并保存成功！" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "❌ 出错: " << e.what() << std::endl;
    }
}