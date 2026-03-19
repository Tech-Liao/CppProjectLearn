//使用deque实现双端队列功能
#include <deque>
#include <iostream>
#include <vector>

void mydeque_example(std::vector<int> &data) {
    std::deque<int> mydeque;
    // 从前端插入元素
    for (const auto &num : data) {
        mydeque.push_front(num);
    }
    std::cout << "双端队列内容（前端插入）: ";
    for (const auto &num : mydeque) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    mydeque.clear();
    // 从后端插入元素
    for (const auto &num : data) {
        mydeque.push_back(num);
    }
    std::cout << "双端队列内容（后端插入）: ";
    for (const auto &num : mydeque) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    // 从前端删除元素
    mydeque.pop_front();
    std::cout << "删除前端元素后内容: ";
    for (const auto &num : mydeque) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    // 从后端删除元素
    mydeque.pop_back();
    std::cout << "删除后端元素后内容: ";
    for (const auto &num : mydeque) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    // 访问前端和后端元素
    if (!mydeque.empty()) {
        std::cout << "前端元素: " << mydeque.front() << std::endl;
        std::cout << "后端元素: " << mydeque.back() << std::endl;
    }
}

int main() {
    std::vector<int> data{1, 2, 3, 5, 4, 7, 9, 8, 0};
    mydeque_example(data);
    return 0;
}