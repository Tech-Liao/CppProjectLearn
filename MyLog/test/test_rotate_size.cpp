#include "Log.h"
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#define MKDIR(p) _mkdir(p)
#define ACCESS(p) _access(p, 0)
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MKDIR(p) mkdir(p, 0755)
#define ACCESS(p) access(p, F_OK)
#endif

// ---------- 简单工具：拼路径、文件大小、枚举文件 ----------
static std::string joinPath(const std::string &dir, const std::string &name)
{
#if defined(_WIN32)
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

static uint64_t fileSizeIfExists(const std::string &path)
{
#if defined(_WIN32)
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(path.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE)
        return 0;
    FindClose(h);
    LARGE_INTEGER li;
    li.HighPart = fd.nFileSizeHigh;
    li.LowPart = fd.nFileSizeLow;
    return static_cast<uint64_t>(li.QuadPart);
#else
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
        return 0;
    return static_cast<uint64_t>(st.st_size);
#endif
}

struct FileInfo
{
    FileInfo(std::string n, uint64_t b) : name(n), bytes(b) {}
    std::string name;
    uint64_t bytes = 0;
};

static std::vector<FileInfo> listLogs(const std::string &dir,
                                      const std::string &base,
                                      const std::string &ext)
{
    std::vector<FileInfo> out;
#if defined(_WIN32)
    // 修改搜索模式，别只限定点号
    std::string pattern = joinPath(dir, base + "*." + ext);

    // 过滤时同样用 prefix_ok / suffix_ok
    std::string name = ffd.cFileName;
    if (name == base + "." + ext)
        continue;

    bool prefix_ok = (name.find(base + ".") == 0) || (name.find(base + "_") == 0);
    bool suffix_ok = name.size() > ext.size() + 1 &&
                     name.rfind("." + ext) == name.size() - (ext.size() + 1);
    if (!(prefix_ok && suffix_ok))
        continue;

    std::string path = joinPath(dir, name);
    LARGE_INTEGER li;
    li.HighPart = ffd.nFileSizeHigh;
    li.LowPart = ffd.nFileSizeLow;
    out.push_back(FileInfo{name, static_cast<uint64_t>(li.QuadPart)});
#else
    DIR *dp = opendir(dir.c_str());
    if (!dp)
        return out;
    struct dirent *ent;
    while ((ent = readdir(dp)) != nullptr)
    {
        std::string name = ent->d_name;
        // 替换 listLogs() 里 POSIX 分支的过滤
        if (name == "." || name == "..")
            continue;

        // 活跃文件：app.log（排除）
        if (name == base + "." + ext)
            continue;

        // 历史文件：既支持 app.YYYYmmdd.N.log，也支持 app_YYYYmmdd_N.log
        bool prefix_ok = (name.find(base + ".") == 0) || (name.find(base + "_") == 0);
        bool suffix_ok = name.size() > ext.size() + 1 &&
                         name.rfind("." + ext) == name.size() - (ext.size() + 1);

        if (!(prefix_ok && suffix_ok))
            continue;

        std::string path = joinPath(dir, name);
        out.push_back(FileInfo{name, fileSizeIfExists(path)});
    }
    closedir(dp);
#endif
return out;
}

// ---------- 测试主体 ----------
int main()
{
    // 1) 准备日志目录 & 轮转配置
    const std::string dir = "./logs";
    const std::string base = "app";
    const std::string ext = "log";

    if (ACCESS(dir.c_str()) != 0)
    {
        MKDIR(dir.c_str());
    }

    LogRotateOptions opt;
    opt.dir = dir;
    opt.base = base;
    opt.ext = ext;
    opt.max_bytes = 32 * 1024; //便于触发轮转
    Logger::instance().setRotateOptions(opt);

    LOG_INFO(Logger::instance(), "=== Rotate-By-Size Test Begin ===");
    LOG_INFO(Logger::instance(), "dir=%s base=%s ext=%s max_bytes=%llu",
             dir.c_str(), base.c_str(), ext.c_str(),
             static_cast<unsigned long long>(opt.max_bytes));

    // 2) 并发写日志；同时另起线程动态调整 max_bytes（验证并发安全）
    const int total_threads = 8;
    const int logs_per_thread = 5000;
    std::atomic<int> produced{0};

    // 构造一个中等长度的消息（~120 字节）
    char payload[100];
    std::memset(payload, 'X', sizeof(payload) - 1);
    payload[sizeof(payload) - 1] = '\0';

    std::vector<std::thread> workers;
    workers.reserve(total_threads);

    auto t0 = std::chrono::steady_clock::now();

    for (int i = 0; i < total_threads; ++i)
    {
        workers.emplace_back([&, i]()
                             {
            for (int j = 0; j < logs_per_thread; ++j) {
                LOG_INFO(Logger::instance(),"T%02d-%05d %s", i, j, payload);
                produced.fetch_add(1, std::memory_order_relaxed);
            } });
    }

    // 动态修改阈值（在 150KB 与 300KB 之间切换），模拟运行期修改配置
    std::atomic<bool> toggler_stop{false};
    std::thread toggler([&]()
                        {
        LogRotateOptions cur = opt;
        for (int k = 0; k < 20 && !toggler_stop.load(); ++k) {
            cur.max_bytes = (k % 2 == 0) ? (150 * 1024) : (300 * 1024);
            Logger::instance().setRotateOptions(cur);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } });

    for (auto &t : workers)
        t.join();
    toggler_stop.store(true);
    if (toggler.joinable())
        toggler.join();

    auto t1 = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    LOG_INFO(Logger::instance(), "Produced logs: %d, elapsed=%lld ms",
             produced.load(), static_cast<long long>(ms));

    // 3) 优雅停机，确保“最后一批”写盘
    Logger::instance().stop();

    // 4) 统计结果（直接用 stdout 打印）
    const std::string live = joinPath(dir, base + "." + ext);
    auto rotated = listLogs(dir, base, ext);
    uint64_t live_sz = fileSizeIfExists(live);

    uint64_t hist_total = 0;
    for (auto &f : rotated)
        hist_total += f.bytes;

    std::cout << "=== Rotate-By-Size Summary ===\n";
    std::cout << "Active file: " << live << "  size=" << live_sz << " bytes\n";
    std::cout << "History count: " << rotated.size()
              << "  total_size=" << hist_total << " bytes\n";
    for (size_t i = 0; i < rotated.size(); ++i)
    {
        std::cout << "  [" << i << "] " << rotated[i].name
                  << "  size=" << rotated[i].bytes << "\n";
    }
    std::cout << "==============================\n";

    // 简单判定：至少发生过一次轮转（存在历史文件）
    if (rotated.empty())
    {
        std::cerr << "[TEST] ❌ FAILED: no rotated files found. "
                  << "Try lowering max_bytes or increasing traffic.\n";
        return 2;
    }
    else
    {
        std::cout << "[TEST] ✅ PASSED: rotation occurred (" << rotated.size() << " files).\n";
        return 0;
    }
}
