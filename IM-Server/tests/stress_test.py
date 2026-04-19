import socket
import struct
import json
import time
import threading
from concurrent.futures import ThreadPoolExecutor

# 全局计数器和锁
success_count = 0
counter_lock = threading.Lock()
is_running = True

def pack_msg(msg_type, content_dict):
    body = json.dumps(content_dict).encode('utf-8')
    header = struct.pack('!II', msg_type, len(body))
    return header + body

def worker_task(thread_id, target_ip, target_port):
    global success_count, is_running
    
    try:
        # 1. 建立 TCP 连接
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.settimeout(5.0) # 超时时间 5 秒
        client.connect((target_ip, target_port))
        
        # 准备一个 Ping 包
        ping_packet = pack_msg(3, {"cmd": "ping"})
        
        while is_running:
            # 疯狂发送 Ping 包
            client.sendall(ping_packet)
            
            # 等待服务器回复 Pong包 (Type 4)
            header = client.recv(8)
            if not header:
                break
            msg_type, body_len = struct.unpack('!II', header)
            body = client.recv(body_len)
            
            # 收到回复，QPS 计数器 +1
            with counter_lock:
                success_count += 1
                
    except Exception as e:
        # 压测高并发下，个别连接断开或超时是正常的
        pass
    finally:
        client.close()

def run_stress_test(concurrent_users=500, duration=10):
    global success_count, is_running
    print(f"🚀 启动压测炮台！模拟并发连接数: {concurrent_users}")
    print(f"⏱️  持续压测时间: {duration} 秒")
    
    # 启动多线程并发
    with ThreadPoolExecutor(max_workers=concurrent_users) as executor:
        for i in range(concurrent_users):
            executor.submit(worker_task, i, '127.0.0.1', 8080)
            
        # 主线程负责每秒打印一次 QPS
        for _ in range(duration):
            time.sleep(1)
            with counter_lock:
                current_qps = success_count
                success_count = 0 # 清零，计算下一秒
            print(f"📊 [实时战报] 当前 QPS: {current_qps} 次/秒")
            
        # 压测时间到，停止所有炮火
        is_running = False
        print("🛑 压测结束！")

if __name__ == '__main__':
    # 你可以修改这里的并发数，先试 100，再试 500，最后冲刺 1000+！
    run_stress_test(concurrent_users=10000, duration=15)