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

struct LogRotateOptions
{
    // 日志文件应该为这样:app_YYYYMMDD_0.log
    std::string dir = ".";            // 日志目录
    std::string base = "app";         // 日志基础名
    std::string ext = "log";          // 扩展名
    uint64_t max_bytes = 10ull << 20; // 10MB
    bool daily = false;               // 标志符号1 先不启动天
    size_t max_backups = 0;           // 标志符号1:是否清理log---0=先不清理
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

    // 日志文件存储设置
    void setRotateOptions(const LogRotateOptions &opt);
    // 重新打开
    void reopen();

private:
    std::ofstream m_file;
    AsyncQueue m_queue;
    std::shared_ptr<Formatter> m_formatter;
    mutable std::mutex m_formatter_mtx;
    std::thread backend_thread;
    std::atomic<bool> m_stopped{false}; // 幂等标记
    std::atomic<int> m_runtime_level;   // 运行时阈值（与 LogLevel 的整数值对齐）

    LogRotateOptions m_rot_opt;
    uint64_t m_writted_bytes = 0; // 当前已写入字节
    std::string m_cur_data;       // 当前日期
    int m_seq = 0;                // 当天序号
    std::atomic<bool> m_need_reopen{false};
    std::mutex m_rot_mtx;

private:
    Logger();
    ~Logger();
    // 禁止拷贝和移动（Cpp11 = delete）
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;
    size_t Write2File(const LogEntry &entry);
    // 获取当前时间（纳秒）
    uint64_t getCurrentTimeNs();
    void BackendLoop();

    // ——按大小轮转：批量写入前的检查与实际轮转——
    void maybeRotateBeforeWrite_(size_t estimated_batch_bytes);
    void rotateNow_(const LogRotateOptions &opt);
    void doReopen_(const LogRotateOptions& opt); // 新增：后台真正执行的重开
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