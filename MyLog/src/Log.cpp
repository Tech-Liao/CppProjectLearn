#include "Log.h"
#include <sstream>

namespace
{
    void formatTimestamp(uint64_t ns, char *buf)
    {
        time_t seconds = ns / 1000000000ULL;
        unsigned long microseconds = (ns % 1000000000ULL) / 1000;

        struct tm tm_info;
        localtime_r(&seconds, &tm_info);

        char time_str[32];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_info);

        snprintf(buf, 32, "%s.%06lu", time_str, microseconds);
    }

    const char *levelToString(LogLevel lv)
    {
        switch (lv)
        {
        case LogLevel::LOG_INFO:
            return "INFO";
        case LogLevel::LOG_DEBUG:
            return "DEBUG";
        case LogLevel::LOG_WARN:
            return "WARN";
        case LogLevel::LOG_ERROR:
            return "ERROR";
        case LogLevel::LOG_FATAL:
            return "FATAL";
        }
        return "UNKNOWN";
    }
} // namespace

Logger::Logger()
{
    m_file.open("app.log", std::ios::app); // 追加模式
    m_formatter = std::make_shared<DefaultFormatter>();
    backend_thread = std::thread(&Logger::BackendLoop, this);
}
Logger::~Logger()
{
    m_queue.stop(); // 通知后端线程退出

    if (backend_thread.joinable())
        backend_thread.join(); // 等待后端线程完全退出

    m_file.flush();
    m_file.close();
}
void Logger::BackendLoop()
{
    std::vector<std::unique_ptr<LogEntry>> buffer;
    while (true)
    {
        bool keep_running = m_queue.popBatch(buffer);

        // 处理数据（锁外执行，避免阻塞生产者）
        for (auto &entry : buffer)
        {
            Write2File(*entry);
        }
        buffer.clear();
        m_file.flush();

        if (!keep_running)
            break;
    }
}

void Logger::log(LogLevel level, const char *filename, int line, const char *fmt, ...)
{
    auto entry = std::unique_ptr<LogEntry>(new LogEntry());
    entry->m_timestamp_ns = getCurrentTimeNs();
    entry->m_id = std::this_thread::get_id();
    entry->m_level = level;
    entry->m_filename = filename;
    entry->m_line = line;

    va_list args;
    va_start(args, fmt);
    vsnprintf(entry->m_msg, sizeof(entry->m_msg), fmt, args);
    va_end(args);

    // ✅ 正确：push到队列，立即返回
    m_queue.push(std::move(entry));
}

void Logger::setFormatter(std::unique_ptr<Formatter> formatter)
{
    if (!formatter)
        return;
    std::lock_guard<std::mutex> lock(m_formatter_mtx);
    m_formatter = std::shared_ptr<Formatter>(std::move(formatter));
}

void Logger::Write2File(const LogEntry &entry)
{
    std::shared_ptr<Formatter> formatter;
    {
        std::lock_guard<std::mutex> lock(m_formatter_mtx);
        formatter = m_formatter;
    }
    if (!formatter)
    {
        formatter = std::make_shared<DefaultFormatter>();
    }

    m_file << formatter->format(entry) << '\n';
}

// 获取当前时间（纳秒）
uint64_t Logger::getCurrentTimeNs()
{
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               now.time_since_epoch())
        .count();
}

void AsyncQueue::push(std::unique_ptr<LogEntry> entry)
{ // TODO: 加锁，把 entry 塞进 buffers_[cur_write_idx_]

    std::lock_guard<std::mutex> lock(m_mtx);
    if (m_stop)
        return;
    m_buffers[m_cur_write_idx].emplace_back(std::move(entry));
    m_cv.notify_one();
}

bool AsyncQueue::popBatch(std::vector<std::unique_ptr<LogEntry>> &buffer)
{
    std::unique_lock<std::mutex> lock(m_mtx);

    m_cv.wait(lock, [this]
              { return m_stop || !m_buffers[m_cur_write_idx].empty(); });

    size_t read_idx = m_cur_write_idx;
    m_cur_write_idx = 1 - m_cur_write_idx;

    buffer.swap(m_buffers[read_idx]);

    if (!buffer.empty())
        return true;

    // 如果 stop 且两个缓冲区都为空，允许后端线程退出
    return !(m_stop && m_buffers[m_cur_write_idx].empty());
}

// ✅ 新增：停止方法
void AsyncQueue::stop()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_stop = true;
    m_cv.notify_all(); // 唤醒所有等待线程
}

std::string DefaultFormatter::format(const LogEntry &entry) const
{
    char time_buf[32];
    formatTimestamp(entry.m_timestamp_ns, time_buf);

    std::ostringstream oss;
    oss << "[" << time_buf << "] "
        << "[" << entry.m_id << "] "
        << "[" << levelToString(entry.m_level) << "] "
        << "[" << entry.m_filename << ":" << entry.m_line << "] "
        << entry.m_msg;
    return oss.str();
}
