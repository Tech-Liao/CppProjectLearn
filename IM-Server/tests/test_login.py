import socket
import struct
import json

def pack_msg(msg_type, content_dict):
    body = json.dumps(content_dict).encode('utf-8')
    header = struct.pack('!II', msg_type, len(body))
    return header + body

def run_client():
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(('127.0.0.1', 8080))
    print("成功连接到服务器...")

    # 测试 1：发送正确的账号密码
    print("\n--- 发送正确的登录请求 ---")
    req1 = pack_msg(1, {"cmd": "login", "username": "user1", "password": "123456"})
    client.sendall(req1)
    
    # 接收回包
    header_data = client.recv(8)
    msg_type, body_len = struct.unpack('!II', header_data)
    body_data = client.recv(body_len)
    print(f"服务端响应 -> Type: {msg_type}, Data: {body_data.decode('utf-8')}")

    # 测试 2：发送错误的密码
    print("\n--- 发送错误的登录请求 ---")
    req2 = pack_msg(1, {"cmd": "login", "username": "user1", "password": "wrong_password"})
    client.sendall(req2)
    
    header_data = client.recv(8)
    msg_type, body_len = struct.unpack('!II', header_data)
    body_data = client.recv(body_len)
    print(f"服务端响应 -> Type: {msg_type}, Data: {body_data.decode('utf-8')}")

    client.close()

if __name__ == '__main__':
    run_client()