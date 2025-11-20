#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <list>
#include <random>
#include <string>
#include <vector>

static volatile long long g_sink = 0;
using Clock = std::chrono::steady_clock;
struct Result {
    std::string name;
    size_t ops = 0;
    long long us = 0;
    long long checksum = 0;
};

template <typename F>
Result bench(const std::string &name, size_t ops, F fn) {
    auto t0 = Clock::now();
    long long sum = fn();
    auto t1 = Clock::now();
    auto us =
        std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    g_sink ^= sum;
    Result t;
    t.name = name;
    t.ops = ops;
    t.us = us;
    t.checksum = sum;
    return t;
}

static std::vector<int> make_data(size_t n, uint32_t seed = 123456u) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 999);
    std::vector<int> v;
    v.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        v.push_back(dist(rng));
    }
    return v;
}

static Result bench_vector_push_back_no_reserve(const std::vector<int> &data) {
    return bench("vector push_back(no reverse)", data.size(),
                 [&]() -> long long {
                     std::vector<int> v;
                     long long sum = 0;
                     for (size_t i = 0; i < data.size(); ++i) {
                         v.push_back(data[i]);
                     }
                     for (size_t i = 0; i < v.size(); ++i) {
                         sum += v[i];
                     }
                     return sum;
                 });
}
static Result bench_vector_push_back_reserve(const std::vector<int> &data) {
    return bench("vector push_back", data.size(), [&]() -> long long {
        std::vector<int> v;
        v.reserve(data.size());
        long long sum = 0;
        for (size_t i = 0; i < data.size(); ++i) {
            v.push_back(data[i]);
        }
        for (size_t i = 0; i < v.size(); ++i) {
            sum += v[i];
        }
        return sum;
    });
}

static Result bench_list_push_back(const std::vector<int> &data) {
    return bench("list push_back", data.size(), [&]() -> long long {
        std::list<int> lst;
        long long sum = 0;
        for (size_t i = 0; i < data.size(); ++i) {
            lst.push_back(data[i]);
        }
        for (auto it = lst.begin(); it != lst.end(); ++it) {
            sum += *it;
        }
        return sum;
    });
}

// ============ 场景 B：头部反复插入 ============
// vector 头插：每次 insert(begin) 需要搬移，接近 O(n^2)
static Result bench_vector_push_front_like(size_t n_ops) {
    return bench("vector insert at begin (simulate push_front)", n_ops,
                 [&]() -> long long {
                     std::vector<int> v;
                     v.reserve(n_ops);  // 即便预留，头插仍需搬移元素
                     long long sum = 0;
                     for (size_t i = 0; i < n_ops; ++i) {
                         v.insert(v.begin(), static_cast<int>(i));
                     }
                     for (size_t i = 0; i < v.size(); ++i) sum += v[i];
                     return sum;
                 });
}

// list 头插：O(1)
static Result bench_list_push_front(size_t n_ops) {
    return bench("list push_front", n_ops, [&]() -> long long {
        std::list<int> lst;
        long long sum = 0;
        for (size_t i = 0; i < n_ops; ++i) {
            lst.push_front(static_cast<int>(i));
        }
        for (std::list<int>::const_iterator it = lst.begin(); it != lst.end();
             ++it) {
            sum += *it;
        }
        return sum;
    });
}

// ============ 场景 C：随机位置插入（会放大 list 的遍历成本） ============
static Result bench_vector_random_insert(size_t n_ops, uint32_t seed = 123u) {
    return bench("vector insert at random position", n_ops, [&]() -> long long {
        std::mt19937 rng(seed);
        std::vector<int> v;
        v.reserve(n_ops);  // 尽量避免因扩容带来的干扰
        long long sum = 0;
        for (size_t i = 0; i < n_ops; ++i) {
            size_t pos = v.empty() ? 0 : (rng() % (v.size() + 1));
            v.insert(v.begin() + static_cast<std::ptrdiff_t>(pos),
                     static_cast<int>(pos));
        }
        for (size_t i = 0; i < v.size(); ++i) sum += v[i];
        return sum;
    });
}

static Result bench_list_random_insert(size_t n_ops, uint32_t seed = 123u) {
    return bench("list insert at random position", n_ops, [&]() -> long long {
        std::mt19937 rng(seed);
        std::list<int> lst;
        long long sum = 0;
        for (size_t i = 0; i < n_ops; ++i) {
            size_t pos = lst.empty() ? 0 : (rng() % (lst.size() + 1));
            std::list<int>::iterator it = lst.begin();
            std::advance(it,
                         static_cast<long long>(pos));  // 遍历到插入点，O(n)
            lst.insert(it, static_cast<int>(pos));      // 插入本身 O(1)
        }
        for (std::list<int>::const_iterator it = lst.begin(); it != lst.end();
             ++it) {
            sum += *it;
        }
        return sum;
    });
}

// ============ 场景 D：反复从头部删除 ============
static Result bench_vector_pop_front_like(size_t n_ops) {
    return bench("vector erase(begin) repeatedly", n_ops, [&]() -> long long {
        std::vector<int> v;
        v.reserve(n_ops);
        for (size_t i = 0; i < n_ops; ++i) v.push_back(static_cast<int>(i));
        long long removed_sum = 0;
        auto it = v.begin();
        for (size_t i = 0; i < n_ops; ++i) {
            removed_sum += *it;
            it = v.erase(v.begin());  // 每次erase都要整体搬移，O(n)
        }
        return removed_sum;
    });
}

static Result bench_list_pop_front(size_t n_ops) {
    return bench("list pop_front repeatedly", n_ops, [&]() -> long long {
        std::list<int> lst;
        for (size_t i = 0; i < n_ops; ++i) lst.push_back(static_cast<int>(i));
        long long removed_sum = 0;
        for (size_t i = 0; i < n_ops; ++i) {
            removed_sum += lst.front();
            lst.pop_front();  // O(1)
        }
        return removed_sum;
    });
}

// ============ 场景 E：按值删除（大量重复值） ============
// vector：erase-remove 惯用法；list：list::remove
static Result bench_vector_remove_value(size_t n, int key) {
    std::vector<int> data = make_data(n, 2025);
    return bench("vector erase-remove(value)", n, [&]() -> long long {
        std::vector<int> v = data;  // 拷贝到待删容器
        auto t0 = Clock::now();
        auto it = std::remove(v.begin(), v.end(), key);  // 线性稳定重排
        v.erase(it, v.end());                            // 一次性删尾
        auto t1 = Clock::now();
        long long sum = 0;
        for (size_t i = 0; i < v.size(); ++i) sum += v[i];
        return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0)
            .count();
    });
}

static Result bench_list_remove_value(size_t n, int key) {
    std::vector<int> data = make_data(n, 2025);
    return bench("list remove(value)", n, [&]() -> long long {
        std::list<int> lst(data.begin(), data.end());
        auto t0 = Clock::now();
        lst.remove(key);  // 逐节点检查/删除，O(n)
        auto t1 = Clock::now();
        long long sum = 0;
        for (std::list<int>::const_iterator it = lst.begin(); it != lst.end();
             ++it) {
            sum += *it;
        }
        return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0)
            .count();
    });
}

static void print_result(const Result &r) {
    double ms = r.us / 1000.0;
    double ops_per_s =
        (r.us > 0) ? (r.ops * 1e6 / static_cast<double>(r.us)) : 0.0;
    std::cout << std::left << std::setw(36) << r.name
              << " | ops=" << std::setw(9) << r.ops << " time=" << std::setw(10)
              << std::fixed << std::setprecision(2) << ms << " ms"
              << "  (" << std::setprecision(1) << ops_per_s << " ops/s)"
              << "  checksum=" << r.checksum << "\n";
}

int main(int argc, char **argv) {
    // ---------- 参数 ----------
    // N_push: 大量尾插的元素数量；N_front: 头插/头删次数；N_mid:
    // 随机中插/删次数；N_val: 按值删除的数据量
    size_t N_push = 500000;  // 可改大到 2000000 观察差异
    size_t N_front = 80000;  // 头插/头删较慢，默认给个相对小的值，避免测试过久
    size_t N_mid = 60000;   // 随机中插/删也较慢，适度
    size_t N_val = 800000;  // 生成用于按值删除的数据总量

    if (argc >= 2) N_push = static_cast<size_t>(std::stoul(argv[1]));
    if (argc >= 3) N_front = static_cast<size_t>(std::stoul(argv[2]));
    if (argc >= 4) N_mid = static_cast<size_t>(std::stoul(argv[3]));
    if (argc >= 5) N_val = static_cast<size_t>(std::stoul(argv[4]));

    std::cout << "N_push=" << N_push << ", N_front=" << N_front
              << ", N_mid=" << N_mid << ", N_val=" << N_val << "\n\n";

    // 预生成一份大随机数据（值域小，便于remove测试）
    std::vector<int> data = make_data(N_push, 123456u);

    // --------- A. 尾插 ----------
    auto r1 = bench_vector_push_back_no_reserve(data);
    auto r2 = bench_vector_push_back_reserve(data);
    auto r3 = bench_list_push_back(data);

    // --------- B. 头插 ----------
    auto r4 = bench_vector_push_front_like(N_front);
    auto r5 = bench_list_push_front(N_front);

    // --------- C. 随机中间插入 ----------
    auto r6 = bench_vector_random_insert(N_mid, 42u);
    auto r7 = bench_list_random_insert(N_mid, 42u);

    // --------- D. 反复从头部删除 ----------
    auto r8 = bench_vector_pop_front_like(N_front);
    auto r9 = bench_list_pop_front(N_front);

    // --------- E. 按值删除（多重复值） ----------
    int key = 123;  // 要删除的目标值
    auto r10 = bench_vector_remove_value(N_val, key);
    auto r11 = bench_list_remove_value(N_val, key);

    // --------- 汇总输出 ----------
    std::cout << std::left << std::setw(36) << "Case"
              << " | " << std::setw(9) << "Ops"
              << " | " << std::setw(10) << "Time(ms)"
              << " | "
              << "Throughput(ops/s)"
              << "\n";
    std::cout << std::string(80, '-') << "\n";

    print_result(r1);
    print_result(r2);
    print_result(r3);
    print_result(r4);
    print_result(r5);
    print_result(r6);
    print_result(r7);
    print_result(r8);
    print_result(r9);
    print_result(r10);
    print_result(r11);

    // 防优化的最终输出（不影响耗时统计，因为我们分别在每个 bench 内已测量）
    std::cout << "\nignore: " << g_sink << "\n";
    return 0;
}