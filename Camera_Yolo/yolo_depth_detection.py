#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
YOLOv5 + Berxel深度相机集成检测系统
实现目标检测和距离测量的结合
"""

import sys
import os
import cv2
import torch
import numpy as np
import subprocess
import time
import threading
import struct
from pathlib import Path

# 添加YOLOv5路径
sys.path.append(str(Path(__file__).parent))

# YOLOv5导入
from models.common import DetectMultiBackend
from utils.general import (LOGGER, check_img_size, check_requirements, check_suffix, 
                          colorstr, non_max_suppression, scale_boxes, xyxy2xywh)
from utils.plots import Annotator, colors
from utils.torch_utils import select_device
from utils.augmentations import letterbox

class BerxelDepthCamera:
    """Berxel深度相机接口类"""
    
    def __init__(self):
        self.berxel_process = None
        self.color_image = None
        self.depth_image = None
        self.running = False
        self.berxel_path = Path(__file__).parent / "Berxel-Depth"
        
        # 文件路径
        self.color_image_path = "/tmp/berxel_color.jpg"
        self.depth_data_path = "/tmp/berxel_depth.bin"
        self.status_file_path = "/tmp/berxel_status.txt"
        self.query_file_path = "/tmp/berxel_query.txt"
        self.response_file_path = "/tmp/berxel_response.txt"
        
        self.depth_width = 0
        self.depth_height = 0
        self.pixel_type = 0
        
        # 深度数据平滑相关
        self.depth_history = []  # 历史深度值
        self.max_history_size = 15  # 增加到15帧用于更好的趋势分析
        self.last_valid_depth = None  # 上一次的有效深度值
        self.depth_jump_threshold = 800.0  # 增大基础跳变阈值(mm)用于移动场景
        self.trend_detection_frames = 5  # 增加到5帧，提高检测稳定性
        self.noise_filter_strength = 0.3  # 降低平滑强度，提高响应性
        
        # 稳定状态检测相关
        self.is_stable = False  # 当前是否稳定
        self.stable_start_time = None  # 开始稳定的时间
        self.stable_duration_threshold = 1.0  # 稳定持续时间阈值(秒)
        self.last_movement_check_time = time.time()  # 上次移动检测时间
        
    def start_camera(self):
        """启动Berxel深度相机桥接程序"""
        try:
            # 确保桥接程序已编译
            bridge_exec = self.berxel_path / "berxel_yolo_bridge"
            if not bridge_exec.exists():
                print("编译桥接程序...")
                result = subprocess.run(
                    ["make", "berxel_yolo_bridge"], 
                    cwd=self.berxel_path, 
                    capture_output=True, 
                    text=True
                )
                if result.returncode != 0:
                    print(f"编译失败: {result.stderr}")
                    return False
            
            # 启动桥接程序
            print("启动Berxel深度相机桥接程序...")
            env = os.environ.copy()
            env['LD_LIBRARY_PATH'] = f"{self.berxel_path}/libs:{env.get('LD_LIBRARY_PATH', '')}"
            
            self.berxel_process = subprocess.Popen(
                [str(bridge_exec)],
                cwd=self.berxel_path,
                env=env,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            
            # 等待启动
            time.sleep(3)
            
            # 检查是否启动成功
            if self.check_camera_status():
                print("Berxel深度相机启动成功！")
                return True
            else:
                print("Berxel深度相机启动失败")
                return False
                
        except Exception as e:
            print(f"启动Berxel相机失败: {e}")
            return False
    
    def check_camera_status(self):
        """检查相机状态"""
        try:
            if os.path.exists(self.status_file_path):
                with open(self.status_file_path, 'r') as f:
                    status = f.readline().strip()
                    return status == "RUNNING"
            return False
        except:
            return False
    
    def get_color_frame(self):
        """获取彩色图像帧"""
        try:
            if os.path.exists(self.color_image_path):
                self.color_image = cv2.imread(self.color_image_path)
                return self.color_image
            return None
        except:
            return None
    
    def load_depth_data(self):
        """加载深度数据"""
        try:
            if os.path.exists(self.depth_data_path):
                with open(self.depth_data_path, 'rb') as f:
                    # 读取尺寸信息
                    self.depth_width = struct.unpack('i', f.read(4))[0]
                    self.depth_height = struct.unpack('i', f.read(4))[0]
                    self.pixel_type = struct.unpack('i', f.read(4))[0]
                    
                    # 读取深度数据
                    depth_size = self.depth_width * self.depth_height * 2  # uint16_t = 2 bytes
                    depth_bytes = f.read(depth_size)
                    
                    # 转换为numpy数组
                    depth_array = np.frombuffer(depth_bytes, dtype=np.uint16)
                    self.depth_image = depth_array.reshape(self.depth_height, self.depth_width)
                    
                return True
            return False
        except Exception as e:
            print(f"加载深度数据失败: {e}")
            return False
    
    def get_min_distance_in_bbox(self, x1, y1, x2, y2):
        """获取检测框内的最小距离（毫米）- 带平滑滤波"""
        try:
            # 写入查询文件 - 格式: x1 y1 x2 y2
            with open(self.query_file_path, 'w') as f:
                f.write(f"{x1} {y1} {x2} {y2}\n")
            
            # 等待响应
            max_wait = 100  # 100ms
            wait_count = 0
            while wait_count < max_wait:
                if os.path.exists(self.response_file_path):
                    with open(self.response_file_path, 'r') as f:
                        distance_str = f.readline().strip()
                        try:
                            raw_distance = float(distance_str)
                            os.remove(self.response_file_path)  # 清理响应文件
                            
                            if raw_distance <= 0:
                                return self.last_valid_depth  # 返回上次有效值
                            
                            # 深度数据平滑处理
                            smoothed_distance = self.smooth_depth_data(raw_distance)
                            return smoothed_distance
                            
                        except:
                            return self.last_valid_depth
                time.sleep(0.001)  # 1ms
                wait_count += 1
            
            return self.last_valid_depth  # 超时返回上次有效值
        except Exception as e:
            print(f"距离查询失败: {e}")
            return self.last_valid_depth
    
    def smooth_depth_data(self, raw_depth):
        """深度数据平滑处理 - 适应移动机器人场景"""
        try:
            # 添加到历史记录
            self.depth_history.append(raw_depth)
            if len(self.depth_history) > self.max_history_size:
                self.depth_history.pop(0)
            
            # 如果历史数据不足，直接使用原始值
            if len(self.depth_history) < 3:
                self.last_valid_depth = raw_depth
                return raw_depth
            
            # 检测移动趋势
            is_moving_trend = self.detect_movement_trend()
            
            # 更新稳定状态
            self.update_stability_status(is_moving_trend)
            
            # 计算自适应阈值
            adaptive_threshold = self.calculate_adaptive_threshold()
            
            # 异常值检测和处理
            if self.last_valid_depth is not None:
                depth_change = abs(raw_depth - self.last_valid_depth)
                
                # 如果检测到移动趋势，放宽跳变阈值
                if is_moving_trend:
                    current_threshold = adaptive_threshold * 2.0  # 移动时阈值加倍
                    print(f"🚶 检测到移动趋势，阈值放宽至 {current_threshold:.0f}mm")
                else:
                    current_threshold = adaptive_threshold
                
                # 如果跳变过大且不是移动趋势，进行噪声过滤
                if depth_change > current_threshold and not is_moving_trend:
                    print(f"🔇 可能为噪声: 跳变{depth_change:.1f}mm > 阈值{current_threshold:.1f}mm")
                    # 使用轻度平滑而不是完全拒绝
                    smoothed = self.last_valid_depth * 0.7 + raw_depth * 0.3
                    self.last_valid_depth = smoothed
                    return smoothed
            
            # 计算平滑后的深度值
            if len(self.depth_history) >= 5:
                # 移动场景下使用较轻的平滑
                if is_moving_trend:
                    # 移动时优先响应新数据
                    recent_values = self.depth_history[-3:]
                    smoothed = np.mean(recent_values)
                else:
                    # 静止时使用更强的平滑
                    recent_values = self.depth_history[-5:]
                    # 使用中位数和均值的组合
                    median_val = np.median(recent_values)
                    mean_val = np.mean(recent_values)
                    smoothed = median_val * 0.6 + mean_val * 0.4
            else:
                # 数据不足时使用简单平均
                smoothed = np.mean(self.depth_history[-3:])
            
            # 应用噪声过滤强度
            if self.last_valid_depth is not None:
                # 在移动时减少平滑强度，提高响应性
                filter_strength = self.noise_filter_strength if not is_moving_trend else self.noise_filter_strength * 0.5
                smoothed = self.last_valid_depth * filter_strength + smoothed * (1 - filter_strength)
            
            self.last_valid_depth = smoothed
            return smoothed
            
        except Exception as e:
            print(f"深度平滑处理失败: {e}")
            return raw_depth
    
    def detect_movement_trend(self):
        """检测是否存在移动趋势 - 放宽判定条件"""
        if len(self.depth_history) < self.trend_detection_frames + 1:
            return False
        
        try:
            # 获取最近几帧数据
            recent_frames = self.depth_history[-(self.trend_detection_frames + 1):]
            
            # 计算连续变化方向和幅度
            changes = []
            for i in range(1, len(recent_frames)):
                change = recent_frames[i] - recent_frames[i-1]
                changes.append(change)
            
            # 过滤掉小幅度变化（可能是噪声）
            significant_changes = [c for c in changes if abs(c) > 150]  # 只考虑>150mm的变化
            
            if len(significant_changes) < 3:  # 如果有效变化少于3次，不算移动
                return False
            
            # 检查是否有一致的变化趋势
            positive_changes = sum(1 for c in significant_changes if c > 0)
            negative_changes = sum(1 for c in significant_changes if c < 0)
            
            # 计算一致性比例
            total_significant = len(significant_changes)
            consistency_ratio = max(positive_changes, negative_changes) / total_significant
            
            # 计算平均变化幅度
            avg_change_magnitude = np.mean([abs(c) for c in significant_changes])
            
            # 计算总体变化幅度（起始到结束）
            total_change = abs(recent_frames[-1] - recent_frames[0])
            
            # 更严格的移动判定条件
            conditions = [
                consistency_ratio >= 0.8,          # 80%以上的变化方向一致（从60%提高）
                avg_change_magnitude > 200,        # 平均变化>200mm（从100mm提高）
                total_change > 500,                # 总变化>500mm（新增条件）
                len(significant_changes) >= 3      # 至少3次有效变化（新增条件）
            ]
            
            is_moving = all(conditions)
            
            # 调试信息（可选）
            if len(self.depth_history) % 10 == 0:  # 每10帧输出一次调试信息
                print(f"🔍 移动检测: 一致性={consistency_ratio:.2f}, 平均变化={avg_change_magnitude:.1f}mm, "
                      f"总变化={total_change:.1f}mm, 有效变化={len(significant_changes)}, 判定={'移动' if is_moving else '静止'}")
            
            return is_moving
            
        except Exception as e:
            print(f"趋势检测失败: {e}")
            return False
    
    def calculate_adaptive_threshold(self):
        """计算自适应跳变阈值"""
        if len(self.depth_history) < 5:
            return self.depth_jump_threshold
        
        try:
            # 基于历史数据的变异性调整阈值
            recent_data = self.depth_history[-10:]
            std_dev = np.std(recent_data)
            
            # 如果深度数据变化较大，增大阈值
            adaptive_factor = 1.0 + (std_dev / 1000.0)  # 每1000mm标准差增加100%阈值
            adaptive_threshold = self.depth_jump_threshold * min(adaptive_factor, 3.0)  # 最大3倍
            
            return adaptive_threshold
            
        except Exception as e:
            return self.depth_jump_threshold
    
    def update_stability_status(self, is_moving):
        """更新稳定状态"""
        current_time = time.time()
        
        if is_moving:
            # 如果正在移动，重置稳定状态
            self.is_stable = False
            self.stable_start_time = None
        else:
            # 如果不在移动
            if self.stable_start_time is None:
                # 刚开始稳定，记录开始时间
                self.stable_start_time = current_time
                self.is_stable = False
            else:
                # 检查是否已经稳定足够长时间
                stable_duration = current_time - self.stable_start_time
                if stable_duration >= self.stable_duration_threshold:
                    self.is_stable = True
                else:
                    self.is_stable = False
    
    def is_data_stable(self):
        """判断数据是否稳定，可以输出到终端"""
        return self.is_stable
    
    def get_stability_info(self):
        """获取稳定状态信息"""
        current_time = time.time()
        
        if self.stable_start_time is None:
            return "🚶移动中", 0.0
        else:
            stable_duration = current_time - self.stable_start_time
            if self.is_stable:
                return f"🧘稳定 ({stable_duration:.1f}s)", stable_duration
            else:
                remaining = self.stable_duration_threshold - stable_duration
                return f"⏱️稳定中 ({remaining:.1f}s)", stable_duration
    
    def stop_camera(self):
        """停止相机"""
        if self.berxel_process:
            self.berxel_process.terminate()
            self.berxel_process.wait()
            self.berxel_process = None
        
        # 清理临时文件
        for path in [self.color_image_path, self.depth_data_path, 
                    self.status_file_path, self.query_file_path, self.response_file_path]:
            try:
                if os.path.exists(path):
                    os.remove(path)
            except:
                pass

class YOLODepthDetector:
    """YOLOv5深度检测器 - 篮筐检测专用"""
    
    def __init__(self, weights='best.pt', device='cpu', conf_thres=0.5, iou_thres=0.45):
        self.device = select_device(device)
        self.model = DetectMultiBackend(weights, device=self.device)
        self.stride, self.names, self.pt = self.model.stride, self.model.names, self.model.pt
        self.imgsz = check_img_size((640, 640), s=self.stride)
        self.conf_thres = conf_thres
        self.iou_thres = iou_thres
        
        # 初始化深度相机
        self.depth_camera = BerxelDepthCamera()
        
        print(f"✓ 篮筐检测模型加载完成，设备: {self.device}")
        print(f"✓ 模型类别数: {len(self.names)}")
        print(f"✓ 检测类别: {list(self.names.values())}")
    
    def preprocess(self, img):
        """图像预处理"""
        img = letterbox(img, self.imgsz, stride=self.stride, auto=self.pt)[0]
        img = img.transpose((2, 0, 1))[::-1]  # HWC to CHW, BGR to RGB
        img = np.ascontiguousarray(img)
        img = torch.from_numpy(img).to(self.device)
        img = img.float() / 255.0
        if len(img.shape) == 3:
            img = img[None]  # expand for batch dim
        return img
    
    def detect_with_distance(self, use_berxel=True):
        """进行检测并输出距离信息"""
        
        if use_berxel:
            # 启动Berxel深度相机
            if not self.depth_camera.start_camera():
                print("⚠️ Berxel深度相机启动失败，切换到普通摄像头模式")
                use_berxel = False
        
        if not use_berxel:
            # 使用普通摄像头
            cap = cv2.VideoCapture(0)
            if not cap.isOpened():
                print("❌ 无法打开摄像头")
                return
        
        print("🚀 开始检测，按 'q' 退出")
        print("=" * 60)
        
        frame_count = 0
        last_print_time = time.time()
        
        # FPS计算相关变量
        fps_start_time = time.time()
        fps_frame_count = 0
        fps = 0.0
        fps_update_interval = 1.0  # 每秒更新一次FPS
        
        try:
            while True:
                frame_start_time = time.time()
                
                if use_berxel:
                    # 从Berxel相机获取图像
                    frame = self.depth_camera.get_color_frame()
                    if frame is None:
                        time.sleep(0.03)  # 30ms
                        continue
                    
                    # 加载深度数据
                    self.depth_camera.load_depth_data()
                else:
                    # 从普通摄像头获取图像
                    ret, frame = cap.read()
                    if not ret:
                        break
                
                # 原始图像尺寸
                h0, w0 = frame.shape[:2]
                
                # 预处理
                img = self.preprocess(frame)
                
                # 推理
                pred = self.model(img)
                pred = non_max_suppression(pred, self.conf_thres, self.iou_thres, max_det=1000)
                
                # 处理检测结果
                annotator = Annotator(frame, line_width=3, example=str(self.names))
                
                detection_info = []
                
                for i, det in enumerate(pred):
                    if len(det):
                        # 调整检测框到原图尺寸
                        det[:, :4] = scale_boxes(img.shape[2:], det[:, :4], frame.shape).round()
                        
                        for *xyxy, conf, cls in reversed(det):
                            # 置信度过滤：只显示置信度 >= 0.5 的检测结果
                            confidence = float(conf)
                            if confidence < 0.5:
                                continue
                            
                            # 类别信息
                            class_name = self.names[int(cls)]
                            
                            # 过滤掉person类别（篮球），只保留篮筐检测
                            if class_name.lower() == 'person':
                                continue
                            
                            x1, y1, x2, y2 = map(int, xyxy)
                            center_x, center_y = (x1 + x2) // 2, (y1 + y2) // 2
                            
                            # 获取距离信息
                            distance = None
                            raw_distance = None
                            if use_berxel:
                                # 获取原始深度值（用于调试显示）
                                distance = self.depth_camera.get_min_distance_in_bbox(x1, y1, x2, y2)
                                # 如果有历史数据，显示平滑效果
                                if (distance is not None and 
                                    len(self.depth_camera.depth_history) > 0):
                                    raw_distance = self.depth_camera.depth_history[-1]  # 最新的原始值
                            
                            # 存储检测信息
                            detection_info.append({
                                'class': class_name,
                                'confidence': confidence,
                                'position': (center_x, center_y),
                                'distance': distance,
                                'bbox': (x1, y1, x2, y2)
                            })
                            
                            # 绘制检测框和距离信息
                            if distance is not None:
                                label = f"{class_name} {confidence:.2f} | {distance:.0f}mm ({distance/1000:.2f}m)"
                                color_code = colors(int(cls), True)
                            else:
                                label = f"{class_name} {confidence:.2f}"
                                color_code = (128, 128, 128)  # 灰色表示无距离数据
                            
                            annotator.box_label(xyxy, label, color=color_code)
                
                # 计算FPS
                fps_frame_count += 1
                current_time = time.time()
                fps_elapsed = current_time - fps_start_time
                
                if fps_elapsed >= fps_update_interval:
                    fps = fps_frame_count / fps_elapsed
                    fps_start_time = current_time
                    fps_frame_count = 0
                
                # 输出检测结果到终端（仅在稳定状态下）
                should_print = (detection_info and 
                               (current_time - last_print_time) > 1.0 and
                               (not use_berxel or self.depth_camera.is_data_stable()))
                
                if should_print:
                    # 获取稳定状态信息
                    if use_berxel:
                        stability_status, stability_duration = self.depth_camera.get_stability_info()
                        print(f"\n🎯 帧 #{frame_count} - 检测到 {len(detection_info)} 个目标 - FPS: {fps:.1f} - {stability_status}")
                    else:
                        print(f"\n🎯 帧 #{frame_count} - 检测到 {len(detection_info)} 个目标 - FPS: {fps:.1f}")
                    print("-" * 60)
                    
                    for i, info in enumerate(detection_info, 1):
                        print(f"{i}. {info['class']}")
                        print(f"   置信度: {info['confidence']:.3f}")
                        print(f"   中心位置: ({info['position'][0]}, {info['position'][1]})")
                        if info['distance'] is not None:
                            print(f"   距离: {info['distance']:.1f}mm ({info['distance']/10:.1f}cm, {info['distance']/1000:.2f}m)")
                            
                            # 显示深度平滑效果（如果有原始数据）
                            if (use_berxel and len(self.depth_camera.depth_history) > 1):
                                raw_val = self.depth_camera.depth_history[-1]
                                smoothed_val = info['distance']
                                diff = abs(raw_val - smoothed_val)
                                history_std = np.std(self.depth_camera.depth_history[-5:]) if len(self.depth_camera.depth_history) >= 5 else 0
                                
                                # 检测移动状态
                                is_moving = self.depth_camera.detect_movement_trend()
                                adaptive_threshold = self.depth_camera.calculate_adaptive_threshold()
                                movement_status = "🚶移动中" if is_moving else "🧘静止"
                                
                                print(f"   原始深度: {raw_val:.1f}mm | 平滑后: {smoothed_val:.1f}mm | 差值: {diff:.1f}mm")
                                print(f"   深度稳定性: {history_std:.1f}mm | 状态: {movement_status} | 阈值: {adaptive_threshold:.0f}mm")
                        else:
                            print(f"   距离: 无深度数据")
                        print()
                    
                    last_print_time = current_time
                elif use_berxel and detection_info:
                    # 如果有检测结果但不稳定，显示简短状态信息
                    if (current_time - last_print_time) > 2.0:  # 每2秒显示一次状态
                        stability_status, _ = self.depth_camera.get_stability_info()
                        print(f"📡 检测中... {stability_status} - 等待稳定后输出详细数据")
                        last_print_time = current_time
                
                # 显示结果
                result_image = annotator.result()
                
                # 在左上角添加FPS信息
                cv2.putText(result_image, f"FPS: {fps:.1f}", 
                           (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                
                # 添加稳定状态信息
                if use_berxel:
                    stability_status, _ = self.depth_camera.get_stability_info()
                    cv2.putText(result_image, stability_status, 
                               (10, 70), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 255, 0), 2)
                
                window_title = 'YOLOv5 + Berxel深度检测' if use_berxel else 'YOLOv5检测 (无深度)'
                cv2.imshow(window_title, result_image)
                
                # 按键退出
                key = cv2.waitKey(1) & 0xFF
                if key == ord('q'):
                    break
                
                frame_count += 1
        
        except KeyboardInterrupt:
            print("\n🛑 检测已停止")
        
        finally:
            # 清理资源
            if use_berxel:
                self.depth_camera.stop_camera()
            else:
                cap.release()
            cv2.destroyAllWindows()

def main():
    """主函数"""
    print("🏀" + "=" * 58 + "🏀")
    print("    YOLOv5 + Berxel深度相机篮筐检测系统")
    print("    功能：实时篮筐检测 + 精确距离测量")
    print("    模型：自训练篮筐检测模型 (best.pt)")
    print("    深度相机：Berxel Hawk")
    print("🏀" + "=" * 58 + "🏀")
    
    try:
        # 检查依赖
        print("\n📋 检查系统依赖...")
        check_requirements(['torch', 'torchvision', 'opencv-python'])
        print("✓ 依赖检查通过")
        
        # 检查模型文件
        weights_path = 'best.pt'
        if not os.path.exists(weights_path):
            print(f"❌ 篮筐检测模型文件 {weights_path} 不存在")
            print("请确保训练好的篮筐检测模型文件在当前目录下")
            return
        print(f"✓ 篮筐检测模型文件: {weights_path}")
        
        # 初始化检测器
        print("\n🏀 初始化篮筐检测器...")
        detector = YOLODepthDetector(
            weights=weights_path,
            device='cpu',          # 改为'cuda'使用GPU
            conf_thres=0.5,        # 置信度阈值(篮筐检测建议使用较高阈值)
            iou_thres=0.45         # NMS IoU阈值
        )
        
        # 开始检测
        detector.detect_with_distance(use_berxel=True)
        
    except KeyboardInterrupt:
        print("\n👋 程序已退出")
    except Exception as e:
        print(f"❌ 错误: {e}")
        import traceback
        traceback.print_exc()

if __name__ == '__main__':
    main() 