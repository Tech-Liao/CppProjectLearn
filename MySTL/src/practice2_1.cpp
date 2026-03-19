// 题目1:实现堆排序，使用priority—queue完成
#include <iostream>
#include <queue>
#include <vector>

int main() {
    std::vector<int> data = {5, 3, 8, 1, 2, 7, 4, 6};
    std::priority_queue<int> max_heap;
    for (const auto &num : data) {
        max_heap.push(num);
    }
    std::cout << "堆排序的结果：" << std::endl;
    while (!max_heap.empty()) {
        std::cout << max_heap.top() << " ";
        max_heap.pop();
    }
    std::cout << std::endl;
    return 0;
}
