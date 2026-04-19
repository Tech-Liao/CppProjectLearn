import socket
import struct
import json
import time
import threading

def pack_msg(msg_type, content_dict):
    body = json.dumps(content_dict).encode('utf-8')
    header = struct.pack('!II', msg_type, len(body))
    return header + body

def receive_thread(client_socket):
    while True:
        try:
            header_data = client_socket.recv(8)
            if not header_data:
                break
            msg_type, body_len = struct.unpack('!II', header_data)
            body_data = client_socket.recv(body_len)
            print(f"  -> [收到服务端回应] Type: {msg_type}, Data: {body_data.decode('utf-8')}")
        except:
            print("  -> 💥 连接已被服务端强行断开！")
            break

def run():
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(('127.0.0.1', 8080))
    print("1. 成功连接到服务器！")

    # 登录
    client.sendall(pack_msg(1, {"cmd": "login", "username": "user1", "password": "123456"}))
    
    # 开后台线程收消息
    t = threading.Thread(target=receive_thread, args=(client,))
    t.daemon = True
    t.start()
    time.sleep(1)

    # 正常发送 3 次心跳
    print("\n2. 开始模拟客户端正常活跃 (每 5 秒发一次 Ping)")
    for i in range(3):
        print(f"  <- 发送 Ping {i+1}...")
        client.sendall(pack_msg(3, {"cmd": "ping"}))
        time.sleep(5)

    # 突然装死
    print("\n3. 🚧 模拟设备突然断网或进电梯 (不再发任何消息)！")
    print("   请盯死你的 C++ 服务器终端日志，大约 30 秒后，见证僵尸清理时刻...")
    
    # 线程休眠 100 秒。此时 Socket 没调用 close()，不会给服务器发挥手包。
    # 这就是一个完美的“僵尸连接”！
    time.sleep(100) 

if __name__ == '__main__':
    run()