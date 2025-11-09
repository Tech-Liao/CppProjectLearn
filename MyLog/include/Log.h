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
enum class LogLevel
{
    LOG_INFO = 0,
    LOG_DEBUG,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
};

struct LogEntry
{
    uint64_t m_timestamp_ns;
    std::chrono::system_clock::time_point m_wall_time; //用于可读时间,实现时间单调性
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
// 双缓冲队列
class AsyncQueue
{
public:
    void push(std::unique_ptr<LogEntry> entry);
    bool popBatch(std::vector<std::unique_ptr<LogEntry>> &buffer);
    void stop();

private:
    std::mutex m_mtx;
    std::condition_variable m_cv; // ✅ 新增：通知机制
    bool m_stop = false;          // ✅ 新增：停止标志
    std::vector<std::unique_ptr<LogEntry>> m_buffers[2];
    size_t m_cur_write_idx = 0;
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
    void setFormatter(std::unique_ptr<Formatter> formatter);

private:
    std::ofstream m_file;
    AsyncQueue m_queue;
    std::shared_ptr<Formatter> m_formatter;
    mutable std::mutex m_formatter_mtx;
    void BackendLoop();
    std::thread backend_thread;
    Logger();
    ~Logger();
    // 禁止拷贝和移动（C++11 = delete）
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;
    void Write2File(const LogEntry &entry);
    // 获取当前时间（纳秒）
    uint64_t getCurrentTimeNs();
};

#define LOG_INFO(fmt, ...) \
    Logger::instance().log(LogLevel::LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    Logger::instance().log(LogLevel::LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
