# CppProjectLearn

## MyLog 运行逻辑概览

### 架构
- `Logger` 是惰性构造的全局单例，内部包含 `AsyncQueue`、文件句柄、后台线程和可热插拔的 `Formatter`。任何线程调用 `log()` 都只负责格式化字符串并入队，实际 IO 由后台线程批处理完成。
- `AsyncQueue` 采用双缓冲向量：生产者永远往写缓冲追加，满足“从空变非空”或达到阈值（默认 256 条）时唤醒消费者；消费者在同一把锁里交换读写缓冲并一次性取走一批日志，关闭时还能自然清空。
- 后台线程在 `BackendLoop()` 中执行：先处理 `reopen` 标志（close→open 当前文件），再批量写入、按需要进行日志轮转，最后刷新磁盘。析构或显式 `stop()` 时队列进入关闭阶段，线程在完全清空后退出。

### 关键特性
- **编译期 + 运行时双重级别**：`LOG_DEBUG/INFO/WARN/ERROR/FATAL` 宏按照 `LOG_COMPILED_LEVEL` 做编译期裁剪，`Logger::setLevel()` 再提供运行时阈值，低于阈值的日志会在 `log()` 入口直接返回。
- **Formatter 模块化**：`DefaultFormatter` 输出 `[wall-clock][thread][level][file:line] msg`，`setFormatter()` 可注入自定义实现（JSON、pattern 等）；后台线程读取共享指针快照，保证线程安全。
- **日志轮转与 reopen**：`LogRotateOptions` 定义输出目录/基名/扩展名/大小阈值等。写入前根据估算字节触发 `rotateNow_()`，生成 `app_YYYYMMDD_seq.log` 并重开“live 文件”。外部 logrotate 可以调用 `Logger::reopen()`，后台线程收到标志后只 close→open 当前文件，不额外 rename。
- **时间戳**：`LogEntry` 同时记录单调纳秒（用于排序）和墙钟时间（可读性），`DefaultFormatter` 保证回拨时仍能呈现正确顺序。
- **幂等停止**：`stop()` 使用 `atomic<bool>` 保证只关闭一次，`AsyncQueue::close()` 唤醒等待线程并在两缓冲耗尽后返回，析构阶段安全写完残余日志。

### 使用方式
```cpp
#include "Log.h"

int main() {
    auto &logger = Logger::instance();
    logger.setRotateOptions({.dir="logs", .base="app", .ext="log", .max_bytes = 5ull << 20});
    logger.setLevel(LogLevel::INFO);
    LOG_INFO(logger, "hello %d", 42);
    logger.reopen(); // 当外部 logrotate trunc 文件后调用
    logger.stop();   // 可选：显式停止（也会在程序退出时析构触发）
}
```
- 宏需要显式传入 logger 引用，以便在不同实例之间切换或做测试。
- 若要替换格式：实现 `Formatter` 子类并调用 `logger.setFormatter(std::unique_ptr<MyFormatter>{...});`

### 构建与测试
1. 正常构建：`cmake -S MyLog -B build && cmake --build build --target SingleLog`  
2. 运行测试程序：`./build/bin/SingleLog`（会在当前目录生成日志）。  
3. ThreadSanitizer：`cmake -S MyLog -B build-tsan -DENABLE_TSAN=ON && cmake --build build-tsan && ./build-tsan/bin/SingleLog 2> tsan-report.log`

测试代码位于 `MyLog/test/test_singlelog.cpp`，覆盖高并发、长消息、时间戳、格式多样性等场景。若需要集成外部 logrotate，可在轮转脚本中发送信号再调用 `Logger::instance().reopen()`，以便在不终止进程的情况下切换日志文件。
