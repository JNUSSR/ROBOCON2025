#!/bin/bash

# Berxel 距离测量工具启动脚本

echo "=== Berxel 距离测量工具 ==="
echo

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# 设置环境变量
echo "设置环境变量..."
export LD_LIBRARY_PATH="${SCRIPT_DIR}/libs:${LD_LIBRARY_PATH}"

# 检查库文件
if [ ! -f "libs/libBerxelHawk.so" ]; then
    echo "错误：找不到 Berxel 库文件！"
    echo "请确保 libs/ 目录存在且包含必要的 .so 文件"
    exit 1
fi

# 检查可执行文件是否存在，不存在则编译
if [ ! -f "distance_meter" ]; then
    echo "编译距离测量工具..."
    make clean
    make all
    
    if [ $? -ne 0 ]; then
        echo "编译失败！请检查依赖是否安装完整。"
        echo "运行 'make check' 检查依赖。"
        exit 1
    fi
fi

echo "启动距离测量工具..."
echo "使用方法："
echo "- 在彩色图像窗口中点击鼠标左键查看距离"
echo "- 按 ESC 键或 Ctrl+C 退出程序"
echo

# 运行程序
./distance_meter

echo "程序已退出" 