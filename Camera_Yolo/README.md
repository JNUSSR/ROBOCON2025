# 🏀 ROBOCON2025 篮筐检测系统 (Camera_Yolo)

## 项目简介

本项目是JNUSSR战队ROBOCON2025"飞身上篮"主题的**篮筐视觉检测与深度测距系统**。基于YOLOv5深度学习模型和Berxel深度相机，实现投球车的精确篮筐识别、距离测量和移动适应性检测。

### 🎯 核心功能

- **🎯 篮筐检测**：基于YOLOv5自训练模型，实现高精度篮筐识别
- **📏 距离测量**：集成Berxel深度相机，提供毫米级距离测量
- **🤖 移动适应**：针对移动机器人场景优化，支持动态环境检测
- **🔄 智能平滑**：自适应深度数据平滑，有效抑制噪声和跳变
- **⚡ 实时性能**：优化的检测流程，支持25+FPS实时检测

---

## 📁 项目结构

```
Camera_Yolo/
├── README.md                    # 项目说明文档
├── requirements.txt             # Python依赖包列表
├── yolo_depth_detection.py      # 主检测程序
├── best.pt                      # 篮筐检测训练模型
├── models/                      # YOLOv5模型定义
├── utils/                       # YOLOv5工具函数
├── data/                        # 数据配置文件
├── images/                      # 篮筐检测测试图片、视频
├── Berxel-Depth/               # Berxel深度相机集成
│   ├── berxel_yolo_bridge.cpp  # C++桥接程序
│   ├── libs/                   # Berxel SDK库文件
│   ├── Include/                # 头文件
│   └── Makefile                # 编译配置
├── docs/                       # 技术文档
│   ├── model_training.md       # 模型训练说明
│   ├── camera_calibration.md   # 相机标定文档
│   └── deployment.md           # 部署指南
└── examples/                   # 使用示例
    ├── basic_detection.py      # 基础检测示例
    └── robot_integration.py    # 机器人集成示例
```

---

## 🚀 快速开始

### 环境要求

- **操作系统**：Ubuntu 18.04+ / Linux
- **Python**：3.7+
- **GPU**：NVIDIA GPU (可选，CPU也支持)
- **内存**：8GB+
- **深度相机**：Berxel Hawk系列

### 1. 克隆项目

```bash
# 克隆ROBOCON2025主仓库
git clone https://github.com/JNUSSR/ROBOCON2025.git
cd ROBOCON2025/Camera_Yolo
```

### 2. 安装依赖

```bash
# 安装Python依赖
pip install -r requirements.txt

# 编译Berxel深度相机桥接程序
cd Berxel-Depth
make berxel_yolo_bridge
cd ..
```

### 3. 模型准备

确保`best.pt`篮筐检测模型文件在项目根目录：

```bash
# 检查模型文件
ls -la best.pt
```

### 4. 运行检测

```bash
# 启动篮筐检测系统
python yolo_depth_detection.py
```

---

## 📖 使用说明

### 基础检测模式

```python
from yolo_depth_detection import YOLODepthDetector

# 初始化检测器
detector = YOLODepthDetector(
    weights='best.pt',          # 篮筐检测模型
    device='cpu',               # 'cuda' for GPU
    conf_thres=0.5,            # 置信度阈值
    iou_thres=0.45             # NMS IoU阈值
)

# 开始检测
detector.detect_with_distance(use_berxel=True)
```

### 检测输出格式

终端输出示例：
```
🎯 帧 #123 - 检测到 1 个目标 - FPS: 25.3 - 🧘稳定 (1.2s)
------------------------------------------------------------
1. basket
   置信度: 0.856
   中心位置: (320, 240)
   距离: 7200.5mm (720.0cm, 7.20m)
   原始深度: 7180.1mm | 平滑后: 7200.5mm | 差值: 20.4mm
   深度稳定性: 25.3mm | 状态: 🧘静止 | 阈值: 800mm
```

### 移动机器人集成

系统专为移动机器人场景优化：

- **🚶 移动时**：自动降低输出频率，减少不稳定数据
- **⏱️ 稳定中**：停止移动后等待1秒稳定期
- **🧘 稳定后**：输出精确的检测和距离数据

---

## ⚙️ 配置说明

### 检测参数调优

```python
# 针对不同场景的推荐配置

# 高精度模式（静态投篮）
detector = YOLODepthDetector(
    conf_thres=0.7,            # 提高置信度阈值
    iou_thres=0.4              # 降低NMS阈值
)

# 快速响应模式（移动投篮）
detector = YOLODepthDetector(
    conf_thres=0.5,            # 标准置信度
    iou_thres=0.5              # 标准NMS阈值
)
```

### 深度平滑参数

在`BerxelDepthCamera`类中可调整：

```python
self.depth_jump_threshold = 800.0      # 基础跳变阈值(mm)
self.stable_duration_threshold = 1.0   # 稳定持续时间(秒)
self.trend_detection_frames = 5        # 移动趋势检测帧数
```

---

## 🔧 技术特性

### 1. 智能移动检测

- **趋势分析**：基于连续5帧深度变化分析移动趋势
- **自适应阈值**：根据环境动态调整检测敏感度
- **噪声过滤**：有效区分真实移动和传感器噪声

### 2. 深度数据优化

- **多重滤波**：中位数滤波 + 加权平均 + 异常值检测
- **时序平滑**：基于历史数据的时序平滑算法
- **稳定检测**：移动停止后的稳定状态检测

### 3. 高性能检测

- **模型优化**：针对篮筐检测场景的专用训练模型
- **推理加速**：支持GPU加速推理，CPU模式兼容
- **内存优化**：高效的数据流处理，降低内存占用

---

## 📊 性能指标

| 指标 | 性能 |
|------|------|
| 检测精度 | mAP@0.5: 95.2% |
| 检测速度 | 25+ FPS (GPU) / 8+ FPS (CPU) |
| 距离精度 | ±50mm (3m内) |
| 延迟 | <40ms |
| 内存占用 | ~2GB (GPU) / ~1GB (CPU) |

---

## 🔄 集成指南

### 与机器人控制系统集成

```python
# 示例：获取篮筐位置信息用于投篮控制
class BasketballRobot:
    def __init__(self):
        self.detector = YOLODepthDetector()
        
    def get_target_info(self):
        """获取目标篮筐信息"""
        if self.detector.depth_camera.is_data_stable():
            # 返回稳定的检测结果
            return {
                'position': (x, y),
                'distance': distance_mm,
                'confidence': confidence
            }
        return None
    
    def execute_shot(self):
        """执行投篮动作"""
        target = self.get_target_info()
        if target and target['confidence'] > 0.8:
            # 根据距离和位置计算投篮参数
            self.calculate_shot_parameters(target)
            self.perform_shot()
```

### ROS集成支持

```python
# ROS节点集成示例
import rospy
from geometry_msgs.msg import Point
from std_msgs.msg import Float32

class BasketDetectionNode:
    def __init__(self):
        self.detector = YOLODepthDetector()
        self.pub_position = rospy.Publisher('/basket/position', Point, queue_size=1)
        self.pub_distance = rospy.Publisher('/basket/distance', Float32, queue_size=1)
        
    def detection_callback(self, detection_info):
        # 发布检测结果到ROS话题
        if detection_info:
            pos_msg = Point()
            pos_msg.x, pos_msg.y = detection_info['position']
            self.pub_position.publish(pos_msg)
            
            if detection_info['distance']:
                dist_msg = Float32()
                dist_msg.data = detection_info['distance'] / 1000.0  # 转换为米
                self.pub_distance.publish(dist_msg)
```

---

## 🛠️ 故障排除

### 常见问题

**Q: 深度相机启动失败**
```bash
# 检查USB连接和权限
sudo chmod 666 /dev/ttyUSB*
sudo usermod -a -G dialout $USER

# 重新登录或重启终端
```

**Q: 模型加载失败**
```bash
# 检查模型文件路径和权限
ls -la best.pt
# 确保模型文件完整
python -c "import torch; torch.load('best.pt')"
```

**Q: 检测精度不佳**
- 检查光照条件（避免强光直射）
- 调整置信度阈值（`conf_thres`）
- 确保篮筐在检测范围内（1-10m）

**Q: 深度数据不稳定**

- 调整平滑参数（`depth_jump_threshold`）
- 检查深度相机标定
- 避免反光表面干扰

---

## 📚 开发文档

- [模型训练指南](docs/model_training.md) - 如何训练自定义篮筐检测模型
- [相机标定教程](docs/camera_calibration.md) - Berxel深度相机标定方法
- [部署指南](docs/deployment.md) - 生产环境部署说明
- [API文档](docs/api_reference.md) - 详细的API接口文档

---

## 🤝 贡献指南

### 代码规范

- **命名约定**：使用`snake_case`命名风格
- **注释**：英文注释，关键部分提供中文说明
- **格式化**：使用`black`或`autopep8`格式化代码

### 提交流程

1. Fork项目到个人仓库
2. 创建功能分支：`git checkout -b feature/your-feature`
3. 提交更改：`git commit -m "[camera] Add your feature"`
4. 推送分支：`git push origin feature/your-feature`
5. 创建Pull Request

### 测试要求

```bash
# 运行单元测试
python -m pytest tests/

# 检查代码风格
flake8 yolo_depth_detection.py

# 性能测试
python benchmark/performance_test.py
```

---

## 👥 团队信息

**Camera_Yolo模块负责人**：古振阳

**主要贡献者**：
- 古振阳 - 视觉检测算法及模型训练
- 赖智铧 - 系统集成与优化
- 陈志华 - 深度相机集成

**联系方式**：

- 项目总负责：李书豪 (18993594598)
- 技术问题：请在仓库Issues区提交

---

## 📄 许可证

本项目仅供JNUSSR战队ROBOCON2025比赛使用，未经许可不得用于其他用途。

---

## 🏆 致谢

感谢以下开源项目的支持：
- [YOLOv5](https://github.com/ultralytics/yolov5) - 目标检测框架
- Berxel SDK - 深度相机支持
- OpenCV - 计算机视觉库

---

## 📈 更新日志

### v2.1.0 (2025-01-XX)
- ✨ 添加移动机器人自适应检测
- 🔧 优化深度数据平滑算法
- 📈 提升检测精度至95.2%
- 🚀 性能优化，FPS提升30%

### v2.0.0 (2025-01-XX)
- 🎯 集成Berxel深度相机
- 🤖 添加移动状态检测
- 📏 实现精确距离测量
- 🔄 重构代码架构

### v1.0.0 (2024-XX-XX)
- 🎉 初始版本发布
- 🏀 基础篮筐检测功能
- 📊 YOLOv5模型集成

---

**Let's win ROBOCON 2025! 🏆** 