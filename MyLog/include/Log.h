#include <string>
#include <thread>
#include <fstream>
#include <cstdarg>
#include <chrono>
#include <mutex>
#include <vector>
#include <memory>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <condition_variable>
#include <atomic>
enum class LogLevel : int
{
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4,
    OFF = 5
};
#ifndef LOG_COMPILED_LEVEL
#define LOG_LVL_DEBUG 0
#define LOG_LVL_INFO 1
#define LOG_LVL_WARN 2
#define LOG_LVL_ERROR 3
#define LOG_LVL_FATAL 4
#define LOG_LVL_OFF 5
#endif

struct LogEntry
{
    uint64_t m_timestamp_ns;
    std::chrono::system_clock::time_point m_wall_time; // 用于可读时间,实现时间单调性
    std::thread::id m_id;
    LogLevel m_level;
    const char *m_filename;
    int m_line;
    char m_msg[256];
};

class Formatter
{
public:
    virtual ~Formatter() = default;
    virtual std::string format(const LogEntry &entry) const = 0;
};

class DefaultFormatter : public Formatter
{
public:
    std::string format(const LogEntry &entry) const override;
};

using EntryPtr = std::unique_ptr<LogEntry>;

// 双缓冲队列
class AsyncQueue
{
public:
    // 返回 true 表示成功入队；在已关闭状态下返回 false（调用方应自行回收 entry）
    bool push(EntryPtr e);
    // 关闭队列:唤醒消费者,尽力清空阶段
    void close();
    // 等待直到有一批可读，或队列关闭且彻底清空
    // out 获取本次批次，返回值：true=仍可能还有数据（应继续循环），false=已经完全清空（可退出）
    bool waitSwapBatch(std::vector<EntryPtr> &out);

private:
    std::mutex m_mtx;
    std::condition_variable m_cv; // ✅ 新增：通知机制
    std::vector<EntryPtr> m_bufA, m_bufB;
    std::vector<EntryPtr> *m_write_buf{&m_bufA};
    std::vector<EntryPtr> *m_read_buf{&m_bufB};
    bool m_closed = false;                     // ✅ 新增：停止标志
    static const size_t kBatchThreshold = 256; // 触发切批/唤醒的阈值（可调）
};

class Logger
{
public:
    static Logger &instance()
    {
        static Logger inst;
        return inst;
    }
    void log(LogLevel level, const char *filename, int line, const char *fmt, ...);
    void setLevel(LogLevel lvl);
    LogLevel level() const;
    void setFormatter(std::unique_ptr<Formatter> formatter);
    void stop();

private:
    std::ofstream m_file;
    AsyncQueue m_queue;
    std::shared_ptr<Formatter> m_formatter;
    mutable std::mutex m_formatter_mtx;
    std::thread backend_thread;
    std::atomic<bool> m_stopped{false}; // 幂等标记
    std::atomic<int> m_runtime_level;   // 运行时阈值（与 LogLevel 的整数值对齐）
    Logger();
    ~Logger();
    // 禁止拷贝和移动（Cpp11 = delete）
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;
    void Write2File(const LogEntry &entry);
    // 获取当前时间（纳秒）
    uint64_t getCurrentTimeNs();
    void BackendLoop();
};


#if LOG_COMPILED_LEVEL <= LOG_LVL_DEBUG
#define LOG_DEBUG(logger, fmt, ...) (logger).log(LogLevel::DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(logger, fmt, ...) \
    do                              \
    {                               \
    } while (0)
#endif

#if LOG_COMPILED_LEVEL <= LOG_LVL_INFO
#define LOG_INFO(logger, fmt, ...) (logger).log(LogLevel::INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_INFO(logger, fmt, ...) \
    do                             \
    {                              \
    } while (0)
#endif

#if LOG_COMPILED_LEVEL <= LOG_LVL_WARN
#define LOG_WARN(logger, fmt, ...) (logger).log(LogLevel::WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_WARN(logger, fmt, ...) \
    do                             \
    {                              \
    } while (0)
#endif

#if LOG_COMPILED_LEVEL <= LOG_LVL_ERROR
#define LOG_ERROR(logger, fmt, ...) (logger).log(LogLevel::ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_ERROR(logger, fmt, ...) \
    do                              \
    {                               \
    } while (0)
#endif

// Fatal 通常始终保留
#define LOG_FATAL(logger, fmt, ...) (logger).log(LogLevel::FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)