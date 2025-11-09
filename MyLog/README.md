# MyLog

MyLog 是一个基于 C++11 的高性能异步日志库，结合双缓冲队列、后台线程、可插拔格式化器以及文件轮转/重开的文件管理能力，适合在多线程环境下以较低开销持久化日志。

## 主要特性
- **异步写入**：写日志线程只负责格式化并把 `LogEntry` 推入 `AsyncQueue::push()`，耗时的 IO 由后台线程在 `BackendLoop()` 中批量完成。
- **双缓冲队列**：`AsyncQueue` 维护两个 `std::vector<EntryPtr>`，写线程始终操作写缓冲，消费者在 `waitSwapBatch()` 中原子交换读写缓冲并批量取走，关闭阶段自动清空。
- **可配置级别**：通过 `LOG_COMPILED_LEVEL` 在编译期剔除低级别日志，运行期再用 `Logger::setLevel()` 调整阈值；低于阈值的日志在入口即被丢弃。
- **Formatter 模块化**：默认格式为 `[wall-clock][thread id][level][file:line] message`，可实现 `Formatter` 子类并调用 `setFormatter()` 注入（例如 JSON/pattern）。
- **文件轮转与 reopen**：`LogRotateOptions` 支持目录、basename、扩展名、文件大小阈值等。后台线程在写批次前估算体积并决定是否 `rotateNow_()`；当外部 logrotate 移动文件时，可调用 `Logger::reopen()` 让后台线程只做 close→open，继续写入新文件。
- **幂等停止**：`Logger::stop()` 通过原子标记保证只执行一次；`AsyncQueue::close()` 唤醒等待线程并确保两缓冲数据写完后再退出。

## 快速上手
```cpp
#include "Log.h"

int main() {
    auto &logger = Logger::instance();

    LogRotateOptions opt;
    opt.dir = "logs";
    opt.base = "app";
    opt.ext = "log";
    opt.max_bytes = 5ull << 20; // 5 MB
    logger.setRotateOptions(opt);
    logger.setLevel(LogLevel::INFO);

    LOG_INFO(logger, "service start pid=%d", ::getpid());
    LOG_WARN(logger, "slow call cost=%lld us", 12345ll);

    // 如果外部脚本执行了 logrotate，可在收到信号后调用：
    logger.reopen();

    logger.stop(); // 可选；程序退出时析构也会触发
}
```

> 注意：所有宏都显式接收 logger 引用，便于在不同实例之间切换或在测试中注入假实现。

## 构建与测试
```bash
cmake -S MyLog -B build
cmake --build build --target SingleLog
./build/bin/SingleLog
```

ThreadSanitizer 版本：
```bash
cmake -S MyLog -B build-tsan -DENABLE_TSAN=ON
cmake --build build-tsan --target SingleLog
./build-tsan/bin/SingleLog 2> tsan-report.log
```

测试程序 `test/test_singlelog.cpp` 会触发高并发写入、长消息和格式多样性，生成的日志可用于人工检验。

## 关键 API
- `Logger::log(level, file, line, fmt, ...)`：由宏调用，一旦通过级别检查就封装 `LogEntry` 并入队。
- `Logger::setLevel(LogLevel)` / `Logger::level()`：运行时调节过滤阈值。
- `Logger::setFormatter(std::unique_ptr<Formatter>)`：设置自定义格式器，后台线程会在写入时获取共享快照。
- `Logger::setRotateOptions(const LogRotateOptions&)`：更新轮转策略，下一批写入前生效。
- `Logger::reopen()`：把 `m_need_reopen` 置位，后台线程在下一次循环中重新打开 live 文件。
- `Logger::stop()`：显式关闭并等待后台线程结束（适用于单元测试或提早收尾）。

## 扩展思路
- 实现 `PatternFormatter` 或 `JsonFormatter`，提供结构化输出。
- 在 `LogRotateOptions` 中补充 `daily`/`max_backups` 的实际逻辑，例如每天自动重置序号并删除旧文件。
- 添加统计接口（排队长度、丢弃条数、轮转次数）以便监控。

以上内容即为当前版本 MyLog 的运行机制与使用指南。欢迎根据业务需求继续扩展 formatter、sink 或轮转策略。 
