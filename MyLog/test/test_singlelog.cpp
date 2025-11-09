#include "Log.h"
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <sstream>
#include <cstring>

// 全局 Logger（也可改成单例获取）

// 便捷宏：将新宏（需要 logger 参数）适配为无 logger 的调用方式
#define L_DEBUG(fmt, ...) LOG_DEBUG(Logger::instance(), fmt, ##__VA_ARGS__)
#define L_INFO(fmt, ...)  LOG_INFO (Logger::instance(), fmt, ##__VA_ARGS__)
#define L_WARN(fmt, ...)  LOG_WARN (Logger::instance(), fmt, ##__VA_ARGS__)
#define L_ERROR(fmt, ...) LOG_ERROR(Logger::instance(), fmt, ##__VA_ARGS__)
#define L_FATAL(fmt, ...) LOG_FATAL(Logger::instance(), fmt, ##__VA_ARGS__)

// ============ 测试1：高并发短线程 ============
void Test_HighConcurrencyShortThreads()
{
    const int total_threads   = 20;
    const int logs_per_thread = 100;
    std::atomic<int> counter{0};

    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(total_threads);
    for (int i = 0; i < total_threads; ++i)
    {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < logs_per_thread; ++j) {
                L_INFO("ShortThread-%d-Log-%d", i, j);
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    for (auto &t : threads) t.join();

    auto end = std::chrono::steady_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    L_INFO("=== 测试1: 高并发短线程 ===");
    L_INFO("预期日志: %d", total_threads * logs_per_thread);
    L_INFO("实际生产: %d", counter.load());
    L_INFO("耗时: %lld ms", static_cast<long long>(ms));
    if (ms > 0) {
        L_INFO("平均吞吐量: %.2f 条/秒", counter.load() * 1000.0 / ms);
    }
}

// ============ 测试2：超长日志内容（会被静默截断，已决定不告警） ============
void Test_LongMessage()
{
    char long_msg[512];
    std::memset(long_msg, 'A', sizeof(long_msg) - 1);
    long_msg[sizeof(long_msg) - 1] = '\0';

    L_INFO("超长日志测试: %s", long_msg); // 超过 256 字节会被静默截断

    // 接近边界
    char border_msg[250];
    std::memset(border_msg, 'B', sizeof(border_msg) - 1);
    border_msg[sizeof(border_msg) - 1] = '\0';
    L_INFO("边界日志测试: %s", border_msg);

    L_INFO("=== 测试2: 超长日志完成 ===");
}

// ============ 测试3：多参数格式 ============
void Test_VariousFormats()
{
    L_INFO("字符串: %s, 整数: %d, 浮点: %.2f", "test", 42, 3.14159);
    L_INFO("指针: %p, 十六进制: 0x%x, 八进制: %o", (void*)0x1234, 255, 064);
    L_INFO("无符号: %u, 长整型: %ld", 4294967295U, 1234567890L);
    L_INFO("=== 测试3: 多参数格式完成 ===");
}

// ============ 测试4：线程ID验证 ============
void Test_ThreadIDUniqueness()
{
    const int num_threads = 20;
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i]() {
            std::ostringstream oss;
            oss << std::this_thread::get_id();
            std::string id_str = oss.str();
            L_INFO("Thread-%d-ID: %s", i, id_str.c_str());
        });
    }
    for (auto &t : threads) t.join();

    L_INFO("=== 测试4: 线程ID验证完成 ===");
}

// ============ 测试5：快速写入，验证时间戳单调与排队 ============
void Test_TimestampUniqueness()
{
    const int rapid_logs = 10000;
    for (int i = 0; i < rapid_logs; ++i) {
        L_INFO("RapidLog-%d", i);
    }
    L_INFO("=== 测试5: 时间戳唯一性完成 ===");
}

// ============ 测试6：级别过滤验证（运行时可热切换） ============
void Test_LevelFiltering()
{
    L_INFO("=== 测试6: 级别过滤开始 ===");

    Logger::instance().setLevel(LogLevel::INFO);
    L_DEBUG("（应被过滤）这条 DEBUG 不应该出现");
    L_INFO ("（应出现）当前运行时阈值 = INFO");
    L_WARN ("（应出现）WARN 级别");
    L_ERROR("（应出现）ERROR 级别");

    Logger::instance().setLevel(LogLevel::WARN);
    L_INFO ("（应被过滤）这条 INFO 不应该出现");
    L_WARN ("（应出现）阈值=Warn，Warn 可见");
    L_ERROR("（应出现）阈值=Warn，Error 可见");

    Logger::instance().setLevel(LogLevel::DEBUG); // 复位为最开放
    L_INFO("=== 测试6: 级别过滤完成 ===");
}

int main()
{
    // 运行时阈值：默认 Debug；也可在运行中随时调整
    Logger::instance().setLevel(LogLevel::DEBUG);

    L_INFO("========== 开始进阶测试 ==========");

    Test_HighConcurrencyShortThreads();
    Test_LongMessage();
    Test_VariousFormats();
    Test_ThreadIDUniqueness();
    Test_TimestampUniqueness();
    Test_LevelFiltering();

    L_INFO("========== 所有测试完成 ==========");
    L_INFO("CPU核心数: %u", std::thread::hardware_concurrency());

    // 显式优雅停机（确保“最后一批”写盘后退出）
    Logger::instance().stop();
    return 0;
}