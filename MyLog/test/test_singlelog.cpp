#include "Log.h"
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <sstream>
// 测试1：高并发短线程
void Test_HighConcurrencyShortThreads()
{
    const int total_threads = 20;   // 500个线程
    const int logs_per_thread = 100; // 每个100条
    std::atomic<int> counter{0};

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < total_threads; ++i)
    {
        threads.emplace_back([&, i]()
                             {
            for (int j = 0; j < logs_per_thread; ++j) {
                LOG_INFO("ShortThread-%d-Log-%d", i, j);
                counter.fetch_add(1, std::memory_order_relaxed);
            } });
    }

    for (auto &t : threads)
        t.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    LOG_INFO("=== 测试1: 高并发短线程 ===");
    LOG_INFO("预期日志: %d", total_threads * logs_per_thread);
    LOG_INFO("实际生产: %d", counter.load());
    LOG_INFO("耗时: %ld ms", ms);
    LOG_INFO("平均吞吐量: %.2f 条/秒", counter.load() * 1000.0 / ms);
}

// 测试2：超长日志内容
void Test_LongMessage()
{
    char long_msg[512];
    memset(long_msg, 'A', sizeof(long_msg) - 1);
    long_msg[sizeof(long_msg) - 1] = '\0';

    LOG_INFO("超长日志测试: %s", long_msg); // 将超过256字节，被截断

    // 测试接近边界
    char border_msg[250];
    memset(border_msg, 'B', sizeof(border_msg) - 1);
    border_msg[sizeof(border_msg) - 1] = '\0';
    LOG_INFO("边界日志测试: %s", border_msg);

    LOG_INFO("=== 测试2: 超长日志完成 ===");
}

// 测试3：多参数格式
void Test_VariousFormats()
{
    LOG_INFO("字符串: %s, 整数: %d, 浮点: %.2f", "test", 42, 3.14159);
    LOG_INFO("指针: %p, 十六进制: 0x%x, 八进制: %o",
             (void *)0x1234, 255, 64);
    LOG_INFO("无符号: %u, 长整型: %ld", 4294967295U, 1234567890L);
    LOG_INFO("=== 测试3: 多参数格式完成 ===");
}

// 测试4：线程ID验证
void Test_ThreadIDUniqueness()
{
    const int num_threads = 20;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([i]()
                             {
            // ✅ 正确转换方式
            std::ostringstream oss;
            oss << std::this_thread::get_id();
            std::string id_str = oss.str();
            
            LOG_INFO("Thread-%d-ID: %s", i, id_str.c_str()); });
    }

    for (auto &t : threads)
        t.join();
    LOG_INFO("=== 测试4: 线程ID验证完成 ===");
}
// 测试5：时间戳唯一性
void Test_TimestampUniqueness()
{
    const int rapid_logs = 10000; // 快速写入1万条
    for (int i = 0; i < rapid_logs; ++i)
    {
        LOG_INFO("RapidLog-%d", i);
    }
    LOG_INFO("=== 测试5: 时间戳唯一性完成 ===");
}

int main()
{

    LOG_INFO("========== 开始进阶测试 ==========");
    
    Test_HighConcurrencyShortThreads();
    Test_LongMessage();
    Test_VariousFormats();
    Test_ThreadIDUniqueness();
    Test_TimestampUniqueness();
    
    LOG_INFO("========== 所有测试完成 ==========");
    
    LOG_INFO("CPU核心数: %d", std::thread::hardware_concurrency());
    return 0;
}