# 使用官方 20.04 镜像
FROM ubuntu:20.04

# 避免交互式安装时的时区选择弹窗
ENV DEBIAN_FRONTEND=noninteractive

# 1. 一次性更换 APT 源为阿里云 (mirrors.aliyun.com)
# 2. 更新并安装常用工具
RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    sed -i 's/security.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    vim \
    git \
    curl \
    ca-certificates \
    build-essential \
    python3 \
    python3-pip && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /home/code

# 默认启动 bash
CMD ["/bin/bash"]