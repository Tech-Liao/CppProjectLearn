#include "Log.h"

Logger::Logger()
{
    m_file.open("app.log", std::ios::app); // 追加模式
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
void Logger::Write2File(const LogEntry &entry)
{
    // 1. 把时间戳（纳秒数）格式化成 "2025-11-08 14:30:25.123456"
    char time_buf[32];
    formatTime(entry.m_timestamp_ns, time_buf); // 你之前实现的函数

    // 2. 拼接成一行日志
    m_file << "[" << time_buf << "] "
           << "[" << entry.m_id << "] "              // thread::id 可以直接输出
           << "[" << toString(entry.m_level) << "] " // 级别转字符串
           << "[" << entry.m_filename << ":" << entry.m_line << "] "
           << entry.m_msg << '\n'; // 用'\n'避免频繁flush
}

// 获取当前时间（纳秒）
uint64_t Logger::getCurrentTimeNs()
{
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               now.time_since_epoch())
        .count();
}

// 格式化时间戳为字符串
void Logger::formatTime(uint64_t ns, char *buf)
{
    // 第一步：纳秒 → 秒 + 微秒
    // 1秒 = 1,000,000,000 纳秒
    time_t seconds = ns / 1000000000ULL;                      // 秒部分
    unsigned long microseconds = (ns % 1000000000ULL) / 1000; // 微秒部分（只保留6位）

    // 第二步：秒 → tm结构体（年月日时分秒）
    struct tm tm_info;
    localtime_r(&seconds, &tm_info); // localtime_r 是线程安全的

    // 第三步：格式化日期时间部分
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_info);

    // 第四步：拼接毫秒部分
    snprintf(buf, 32, "%s.%06lu", time_str, microseconds);
}

// 日志级别转字符串
const char *Logger::toString(LogLevel lv)
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
