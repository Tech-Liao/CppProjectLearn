#include <algorithm>
#include <iostream>

#include "vector.h"

void test_v1() {
    my::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    for (size_t i = 0; i < vec.get_size(); ++i) {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "V1 test success\n";
}
void test_v2() {
    my::vector<int> vec;
    for (size_t i = 0; i < 2; ++i) {
        vec.push_back(i + 1);
    }
    std::cout << "====vec====\n";
    for (size_t i = 0; i < vec.get_size(); ++i) {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;
    my::vector<int> vec1(vec);
    my::vector<int> vec2 = vec;
    // test 拷贝
    std::cout << "====vec1====\n";
    for (size_t i = 0; i < vec1.get_size(); ++i) {
        std::cout << vec1[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "====vec2====\n";
    for (size_t i = 0; i < vec2.get_size(); ++i) {
        std::cout << vec2[i] << " ";
    }
    std::cout << std::endl;
    // test resize
    std::cout << "====vec resize 10====\n";
    std::cout << "before resize:" << vec.get_capacity() << std::endl;
    vec.resize(10);
    std::cout << "after resize:" << vec.get_capacity() << std::endl;
    for (size_t i = 0; i < vec.get_size(); ++i) {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;
    // resize < capacity
    std::cout << "====vec resize 1====\n";
    std::cout << "before resize:" << vec.get_capacity() << std::endl;
    vec.resize(1);
    std::cout << "after resize:" << vec.get_capacity() << std::endl;
    for (size_t i = 0; i < vec.get_size(); ++i) {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;
    // test pop_back
    vec1.pop_back();
    std::cout << "====vec1 pop ====\n";
    for (size_t i = 0; i < vec1.get_size(); ++i) {
        std::cout << vec1[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "V2 test pop_back resize copy success\n";
}

void test_v3() {
    my::vector<int> vec;
    vec.push_back(10);
    vec.push_back(50);
    vec.push_back(40);
    vec.push_back(30);
    // std::cout << "Iterating over MyVector using iterator:" << std::endl;
    for (my::iterator<int> it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";  // 解引用迭代器来访问元素
    }
    std::cout << std::endl;

    // 使用 std::find 查找元素
    auto it = std::find(vec.begin(), vec.end(), 30);
    if (it != vec.end()) {
        std::cout << "Found: " << *it << std::endl;
    } else {
        std::cout << "Not found." << std::endl;
    }

    // 使用 std::distance 计算迭代器之间的距离
    std::cout << "Distance between begin and end: "
              << std::distance(vec.begin(), vec.end()) << std::endl;
    // 使用 std::sort 排序元素
    std::cout << "Before sorting:" << std::endl;
    std::for_each(vec.begin(), vec.end(),
                  [](const int& x) { std::cout << x << " "; });
    std::cout << std::endl;

    std::sort(vec.begin(), vec.end(), [](int a, int b) { return a < b; });
    std::cout << "After sorting:" << std::endl;
    std::for_each(vec.begin(), vec.end(),
                  [](const int& x) { std::cout << x << " "; });
    std::cout << std::endl;
}

int main() {
    // test_v1();
    // test_v2();
    test_v3();
    return 0;
}