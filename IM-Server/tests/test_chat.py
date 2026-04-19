import socket
import struct
import json
import threading
import sys

def pack_msg(msg_type, content_dict):
    body = json.dumps(content_dict).encode('utf-8')
    header = struct.pack('!II', msg_type, len(body))
    return header + body

def receive_thread(client_socket):
    """后台死循环接收消息的线程"""
    while True:
        try:
            header_data = client_socket.recv(8)
            if not header_data:
                break
            msg_type, body_len = struct.unpack('!II', header_data)
            body_data = client_socket.recv(body_len)
            print(f"\n[收到服务器推送] Type: {msg_type}, Data: {body_data.decode('utf-8')}")
            print("请输入目标用户和消息 (格式: to_user|message) > ", end="")
        except:
            break

def run_client(username, password):
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(('127.0.0.1', 8080))
    print(f"[{username}] 成功连接到服务器，正在登录...")

    # 发送登录请求
    req = pack_msg(1, {"cmd": "login", "username": username, "password": password})
    client.sendall(req)

    # 启动后台收消息线程
    t = threading.Thread(target=receive_thread, args=(client,))
    t.daemon = True
    t.start()

    # 主线程负责发消息
    while True:
        try:
            user_input = input("请输入目标用户和消息 (格式: to_user|message) > ")
            if "|" in user_input:
                to_user, msg = user_input.split("|", 1)
                chat_req = pack_msg(2, {"cmd": "chat", "to": to_user, "msg": msg})
                client.sendall(chat_req)
            else:
                print("格式错误，请使用 | 分割")
        except KeyboardInterrupt:
            client.close()
            sys.exit(0)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("用法: python3 test_chat.py <username> <password>")
        sys.exit(1)
    run_client(sys.argv[1], sys.argv[2])