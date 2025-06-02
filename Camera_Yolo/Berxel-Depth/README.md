# Berxel 距离测量工具

一个简单的深度相机距离测量工具，支持鼠标点击查看距离。

## 功能

- 实时显示彩色和深度视频流
- 鼠标左键点击查看任意点的距离
- 按 ESC 键或 Ctrl+C 退出

## 快速开始

```bash
# 直接运行（推荐）
./run_distance_meter.sh

# 或者手动编译运行
make all
make run
```

## 文件说明

- `distance_meter.cpp` - 主程序源代码
- `run_distance_meter.sh` - 启动脚本（自动处理环境变量）
- `Makefile` - 编译配置
- `libs/` - Berxel SDK 库文件
- `Include/` - SDK 头文件

## 使用方法

1. 连接 Berxel 深度相机
2. 运行 `./run_distance_meter.sh`
3. 在"彩色图像 (点击测距)"窗口中点击鼠标左键
4. 在终端中查看距离输出

## 系统要求

- Ubuntu 18.04+
- OpenCV 4.x
- Berxel 深度相机
- USB 3.0 接口

距离单位：毫米(mm)、厘米(cm)、米(m) 