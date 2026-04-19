import socket
import struct
import json
import threading
import sys
import time

def pack_msg(msg_type, content_dict):
    body = json.dumps(content_dict).encode('utf-8')
    header = struct.pack('!II', msg_type, len(body))
    return header + body

def receive_thread(client_socket):
    while True:
        try:
            header_data = client_socket.recv(8)
            if not header_data: break
            msg_type, body_len = struct.unpack('!II', header_data)
            body_data = client_socket.recv(body_len)
            print(f"\n📩 [收到服务器推送] Type: {msg_type}, Data: {body_data.decode('utf-8')}")
            print("请输入群号和消息 (格式: 1|兄弟们开黑吗) > ", end="", flush=True)
        except:
            print("\n❌ 连接断开")
            break

def run():
    if len(sys.argv) != 3:
        print("用法: python3 test_group.py <username> <password>")
        return

    username = sys.argv[1]
    password = sys.argv[2]

    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(('127.0.0.1', 8080))

    # 1. 登录
    client.sendall(pack_msg(1, {"cmd": "login", "username": username, "password": password}))
    
    # 开后台线程收消息
    t = threading.Thread(target=receive_thread, args=(client,))
    t.daemon = True
    t.start()
    
    time.sleep(1) # 等待登录回执打印完毕

    # 2. 交互式发送群聊
    while True:
        try:
            user_input = input("请输入群号和消息 (格式: 1|兄弟们开黑吗) > ")
            if "|" not in user_input: continue
            
            group_id_str, msg_content = user_input.split("|", 1)
            group_id = int(group_id_str)

            # 打包我们 C++ 里写的群聊协议
            req_json = {
                "cmd": "group_chat",
                "group_id": group_id,
                "msg": msg_content
            }
            client.sendall(pack_msg(2, req_json))
        except KeyboardInterrupt:
            break

if __name__ == '__main__':
    run()