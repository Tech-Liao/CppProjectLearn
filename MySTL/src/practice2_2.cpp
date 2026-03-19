//题目2：使用map存储一个词频统计系统
#include <iostream>
#include <map>
#include <sstream>
#include <string>
int main() {
    std::string text = "this is a test this is only a test this test is a test";
    std::map<std::string, int> word_count;
    std::istringstream iss(text);  // 将字符串作为输入流处理
    std::string word;
    while (iss >> word) {
        ++word_count[word];
    }
    std::cout << "词频统计结果：" << std::endl;
    for (const auto& pair : word_count) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }

    return 0;
}