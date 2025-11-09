# CppProjectLearn

## MyLog 运行逻辑概览

1. **单例初始化**  
   首次调用 `LOG_INFO/LOG_ERROR` 等宏时，会懒加载 `Logger::instance()` 创建单例。构造函数在 `app.log` 上以追加模式打开文件，并启动后台线程执行 `Logger::BackendLoop()`。

2. **前端写入（push）**  
   `Logger::log()` 将格式化后的内容封装到 `LogEntry`，再交给 `AsyncQueue::push()`。`push` 拿互斥锁把日志写入“当前写缓冲”（双缓冲中的一个 `std::vector`），然后唤醒后台线程；如果队列已经 `stop()` 则直接丢弃。

3. **双缓冲交换**  
   后台线程在 `AsyncQueue::popBatch()` 中等待“写缓冲非空或停止信号”。一旦唤醒，会在同一把锁里翻转读写标志：旧写缓冲被交给后台线程消费，生产者切换到另一块缓冲继续写。这样交换操作 O(1) 且避免生产/消费互相阻塞。

4. **后台写盘**  
   `Logger::BackendLoop()` 拿到一批 `LogEntry` 后，在锁外逐条调用 `Write2File()`：  
   - 先将纳秒时间戳格式化成 `YYYY-MM-DD HH:MM:SS.uuuuuu`；  
   - 内容包括线程 ID、日志级别、源文件与行号以及消息体；  
   - 每批写完刷新文件缓冲，维持较低 IO 延迟。  
   如果 `popBatch()` 告诉它“停止且队列为空”，后台线程就安全退出循环。

5. **停止与析构**  
   程序结束时 `Logger::~Logger()` 调用 `AsyncQueue::stop()` 标记停止并唤醒后台线程，然后 `join` 等待其退出。此时所有缓冲都已写盘，只需 `flush()`/`close()` 文件即可，确保没有日志遗失。

6. **测试入口**  
   `MyLog/test/test_singlelog.cpp` 提供多线程压测、长消息、格式化、多线程 ID 等用例，可通过 `cmake -S MyLog -B build && cmake --build build` 构建，再运行 `build/bin/SingleLog` 验证行为；若开启 `-DENABLE_TSAN=ON`，还能用 ThreadSanitizer 检查并发问题。

该架构利用双缓冲 + 后台线程，把前端日志调用的成本压到锁 + 向量增尾，并在退出时确保顺序写盘和资源回收。
