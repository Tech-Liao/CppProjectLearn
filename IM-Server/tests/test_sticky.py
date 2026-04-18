import socket
import struct
import json
import time

def pack_msg(msg_type, content_dict):
    """按照 C++ 端的 MsgHeader 格式打包数据"""
    body = json.dumps(content_dict).encode('utf-8')
    # ! 表示网络字节序(大端), I 表示 unsigned int (4字节)
    # 对应 C++ 的: msg_type 和 body_length
    header = struct.pack('!II', msg_type, len(body))
    return header + body

def run_client():
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(('127.0.0.1', 8080))
    print("成功连接到服务器")

    # 场景 1：制造极端的“粘包”
    print("\n--- 开始发送粘包测试 ---")
    packet1 = pack_msg(1, {"username": "user_a", "cmd": "login"})
    packet2 = pack_msg(3, {"from": 1, "to": 2, "content": "hello world"})
    packet3 = pack_msg(3, {"from": 1, "to": 2, "content": "are you there?"})
    
    # 丧心病狂地把三个包拼接成一段连续的字节流，一次性发过去！
    sticky_stream = packet1 + packet2 + packet3
    client.sendall(sticky_stream)

    # 接收服务端的回复 (假设服务端能正确拆出3个包并回传3次)
    for _ in range(3):
        # 先收 8 字节包头
        header_data = client.recv(8)
        if len(header_data) < 8:
            break
        msg_type, body_len = struct.unpack('!II', header_data)
        # 再收包体
        body_data = client.recv(body_len)
        print(f"收到回包 -> Type: {msg_type}, Data: {body_data.decode('utf-8')}")

    time.sleep(1)
    client.close()

if __name__ == '__main__':
    run_client()