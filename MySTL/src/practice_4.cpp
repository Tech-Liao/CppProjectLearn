// ** 使用 `set` 和 `unordered_set` 来实现一个去重操作，并对比性能

#include <chrono>
#include <iostream>
#include <random>
#include <set>
#include <unordered_set>
#include <vector>
template <typename F>
long long benchmark(F&& fn) {
    auto start = std::chrono::steady_clock::now();
    fn();
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start)
        .count();
}

int main() {
    const size_t N = 1000000;
    std::vector<int> data;
    data.reserve(N);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 100);
    for (size_t i = 0; i < N; ++i) {
        data.push_back(dist(gen));
    }
    auto set_time = benchmark(
        [&]() { std::set<int> unique_set(data.begin(), data.end()); });
    std::cout << "set 去重时间:" << set_time << std::endl;
    auto unordered_set_time = benchmark([&]() {
        std::unordered_set<int> unique_unordered_set(data.begin(), data.end());
    });
    std::cout << "unordered set 去重时间:" << unordered_set_time << std::endl;
    return 0;
}