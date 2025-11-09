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
    std::thread::id m_id;
    LogLevel m_level;
    const char *m_filename;
    int m_line;
    char m_msg[256];
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

private:
    std::ofstream m_file;
    AsyncQueue m_queue;
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

    // 格式化时间戳为字符串
    void formatTime(uint64_t ns, char *buf);

    // 日志级别转字符串
    const char *toString(LogLevel lv);
};

#define LOG_INFO(fmt, ...) \
    Logger::instance().log(LogLevel::LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    Logger::instance().log(LogLevel::LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
