#include "Log.h"
#include <sstream>

namespace
{
    static std::string formatWallTime(std::chrono::system_clock::time_point tp)
    {
        using namespace std::chrono;
        const auto sec_tp = time_point_cast<seconds>(tp);
        const auto nsec = duration_cast<nanoseconds>(tp - sec_tp).count();
        const time_t sec = system_clock::to_time_t(sec_tp);

#if defined(_WIN32)
        tm t{};
        localtime_s(&t, &sec);
#else
        tm t{};
        localtime_r(&sec, &t);
#endif
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d:%09lld",
                      t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                      t.tm_hour, t.tm_min, t.tm_sec,
                      static_cast<long long>(nsec));
        return std::string(buf);
    }

    const char *levelToString(LogLevel lv)
    {
        switch (lv)
        {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::FATAL:
            return "FATAL";
        case LogLevel::OFF:
            return "OFF";
        }
        return "UNKNOWN";
    }
    static std::string joinPath(const std::string &dir, const std::string &name)
    {
#ifdef _WIN32
        const char sep = '\\';
#else
        const char sep = '/';
#endif
        if (dir.empty())
            return name;
        if (dir.back() == '/' || dir.back() == '\\')
            return dir + name;
        return dir + sep + name;
    }
    static std::string formatYMD(std::time_t t)
    {
        std::tm tmv;
#if defined(_WIN32)
        localtime_s(&tmv, &t);
#else
        localtime_r(&t, &tmv);
#endif
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%04d%02d%02d",
                      tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday);
        return std::string(buf);
    }
    static uint64_t fileSizeIfExists(const std::string &path)
    {
        std::ifstream f(path.c_str(), std::ios::binary | std::ios::ate);
        if (!f)
            return 0;
        return static_cast<uint64_t>(f.tellg());
    }
} // namespace

// ======= Logger ===========
// Logger private
Logger::Logger() : m_stopped(false)
{
    m_runtime_level.store(static_cast<int>(LogLevel::DEBUG), std::memory_order_relaxed);
    m_rot_opt = LogRotateOptions{};
    const std::string live = joinPath(m_rot_opt.dir, m_rot_opt.base + "." + m_rot_opt.ext);
    m_file.open(live.c_str(), std::ios::out | std::ios::app);
    m_writted_bytes = fileSizeIfExists(live);
    m_cur_data = formatYMD(std::time(nullptr)); // 先存今天（里程碑2用）
    m_formatter = std::make_shared<DefaultFormatter>();
    backend_thread = std::thread(&Logger::BackendLoop, this);
}
Logger::~Logger()
{
    stop();
    if (m_file.is_open())
        m_file.close();
}

void Logger::BackendLoop()
{
    std::vector<EntryPtr> batch;
    while (m_queue.waitSwapBatch(batch))
    {
        // === 新增：处理 reopen 标志（只 close→open，不 rename） ===
        if (m_need_reopen.exchange(false))
        {
            LogRotateOptions snap;
            { // 读 m_rot_opt 做快照，避免与 setRotateOptions 并发
                std::lock_guard<std::mutex> lk(m_rot_mtx);
                snap = m_rot_opt;
            }
            doReopen_(snap);
        }

        if (!batch.empty())
        {

            size_t est = 0;
            for (size_t i = 0; i < batch.size(); ++i)
            {
                est += 64;
                est += std::strlen(batch[i]->m_msg) + 1;
            }
            maybeRotateBeforeWrite_(est);
            for (size_t i = 0; i < batch.size(); ++i)
            {
                m_writted_bytes = Write2File(*batch[i]);
            }
            batch.clear();
            m_file.flush();
        }
    }
    m_file.flush();
}

size_t Logger::Write2File(const LogEntry &entry)
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
    const std::string line = formatter->format(entry);
    m_file << line << "\n";
    return line.size() + 1;
}

// 获取当前时间（纳秒）-> 2.2 修改为单调递增,避免回拨
uint64_t Logger::getCurrentTimeNs()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}
// ——按大小轮转：批量写入前的检查与实际轮转——
void Logger::maybeRotateBeforeWrite_(size_t estimated_batch_bytes)
{
    LogRotateOptions snap;
    {
        std::lock_guard<std::mutex> lock(m_rot_mtx);
        snap = m_rot_opt;
    }
    if (snap.max_bytes == 0)
        return;
    if (m_writted_bytes + estimated_batch_bytes <= snap.max_bytes)
        return;
    rotateNow_(snap);
}
void Logger::rotateNow_(const LogRotateOptions &opt)
{
    // 关闭旧文件
    m_file.flush();
    m_file.close();
    // 生成历史名
    const std::string today = formatYMD(std::time(nullptr));
    if (m_cur_data != today)
    {
        m_cur_data = today;
        m_seq = 0;
    }
    const std::string rotated = joinPath(opt.dir,
                                         opt.base + "_" + m_cur_data + "_" + std::to_string(m_seq++) + "." + opt.ext);
    const std::string live = joinPath(opt.dir, opt.base + "." + opt.ext);
    // 重命名为历史文件
    (void)std::rename(live.c_str(), rotated.c_str());
    m_file.open(live.c_str(), std::ios::binary | std::ios::app);
    m_writted_bytes = fileSizeIfExists(live);
}

void Logger::doReopen_(const LogRotateOptions &opt)
{
    // 与 rotateNow_ 的前半段相同，但不生成历史名、不 rename
    m_file.flush();
    m_file.close();

    const std::string live = joinPath(opt.dir, opt.base + "." + opt.ext);

    // 重新打开活跃文件（配合外部 logrotate 的 move+create）
    m_file.open(live.c_str(), std::ios::out | std::ios::app);

    // 重新读取当前大小（外部可能新建了空文件）
    m_writted_bytes = fileSizeIfExists(live);

    // 可选：打开失败时降级到 stderr，或把 need_reopen 置回以便下次重试
    // if (!m_file) { std::perror("reopen"); }
}

// Logger Public
void Logger::log(LogLevel level, const char *filename, int line, const char *fmt, ...)
{
    if (static_cast<int>(level) < m_runtime_level.load(std::memory_order_relaxed))
    {
        return; // 低于运行时阈值：直接丢弃，避免格式化与入队
    }
    auto entry = std::unique_ptr<LogEntry>(new LogEntry());
    entry->m_timestamp_ns = getCurrentTimeNs();
    entry->m_wall_time = std::chrono::system_clock::now(); // 墙钟时间展示
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

void Logger::setLevel(LogLevel lvl)
{
    m_runtime_level.store(static_cast<int>(lvl), std::memory_order_relaxed);
}
LogLevel Logger::level() const
{
    return static_cast<LogLevel>(m_runtime_level.load(std::memory_order_relaxed));
}

void Logger::stop()
{
    bool expected = false;
    if (!m_stopped.compare_exchange_strong(expected, true))
    {
        return; // 已停止：幂等返回
    }
    m_queue.close(); // 进入关闭+清空阶段
    if (backend_thread.joinable())
        backend_thread.join(); // 等后台线程清空并退出
}

void Logger::setRotateOptions(const LogRotateOptions &opt)
{
    std::lock_guard<std::mutex> lock(m_rot_mtx);
    m_rot_opt = opt; // 简单拷贝；若在运行中调用，生效点是下个批次写入前
}

void Logger::reopen()
{
    m_need_reopen.store(true, std::memory_order_relaxed);
}

// ======= AsyncQueue===========

// 返回 true 表示成功入队；在已关闭状态下返回 false（调用方应自行回收 entry）
bool AsyncQueue::push(EntryPtr e)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if (m_closed)
        return false;
    const bool was_empty = m_write_buf->empty(); // 关键：入队前记录
    m_write_buf->push_back(std::move(e));
    // 触发条件：从空到非空，或恰好达到阈值
    if (was_empty || m_write_buf->size() == kBatchThreshold)
    {
        m_cv.notify_one();
    }
    return true;
}
// 关闭队列:唤醒消费者,尽力清空阶段
void AsyncQueue::close()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if (m_closed)
        return;
    m_closed = true;
    m_cv.notify_all();
}
// 等待直到有一批可读，或队列关闭且彻底清空
// out 获取本次批次，返回值：true=仍可能还有数据（应继续循环），false=已经完全清空（可退出）
bool AsyncQueue::waitSwapBatch(std::vector<EntryPtr> &out)
{
    out.clear();
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cv.wait(lock, [&]
              { return m_closed || !m_write_buf->empty(); });
    // 若关闭且写缓冲为空，检查读缓冲是否还有遗留
    if (m_write_buf->empty() && m_closed)
    {
        if (!m_read_buf->empty())
        {
            out.swap(*m_read_buf);
            return true; // 仍有数据需要处理
        }
        return false; // 两缓冲均空，彻底清空
    }
    // 正常切换一批：把写缓冲切换为读缓冲并导出
    std::swap(m_write_buf, m_read_buf);
    out.swap(*m_read_buf);
    return true;
}

std::string DefaultFormatter::format(const LogEntry &entry) const
{
    const std::string ts = formatWallTime(entry.m_wall_time);

    std::ostringstream oss;
    oss << "[" << ts << "] "
        << "[" << entry.m_id << "] "
        << "[" << levelToString(entry.m_level) << "] "
        << "[" << entry.m_filename << ":" << entry.m_line << "] "
        << entry.m_msg;
    return oss.str();
}
