//给定一个 `vector`，写一个函数找出其中的最大值，并打印其位置（使用迭代器）。
#include <iostream>
#include <random>
#include <vector>

void make_data(std::vector<int> &vec, size_t n) {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, n + 1);
    for (size_t i = 0; i < n; i++) {
        vec.push_back(dist(rd) / 2 + dist(rd) - 1);
    }
    for (auto it : vec) std::cout << it << " ";
    std::cout << std::endl;
}
int find_big(std::vector<int> &vec) {
    int big = 0;
    for (auto it : vec) {
        if (it > big) big = it;
    }
    return big;
}

int main() {
    std::vector<int> vec;
    make_data(vec, 10);
    int big = find_big(vec);
    std::cout << "big: " << big << std::endl;
    return 0;
}