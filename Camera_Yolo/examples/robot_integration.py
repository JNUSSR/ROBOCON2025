#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ROBOCON2025 投球车集成示例
演示如何将篮筐检测系统集成到投球车的控制系统中
"""

import sys
import time
import threading
from pathlib import Path
from dataclasses import dataclass
from typing import Optional, Tuple

# 添加项目根目录到路径
project_root = Path(__file__).parent.parent
sys.path.append(str(project_root))

from yolo_depth_detection import YOLODepthDetector

@dataclass
class TargetInfo:
    """目标信息数据类"""
    position: Tuple[int, int]  # 图像坐标 (x, y)
    distance: float           # 距离 (mm)
    confidence: float         # 置信度
    timestamp: float          # 时间戳
    is_stable: bool          # 是否稳定

class BasketballRobot:
    """投球车控制类"""
    
    def __init__(self):
        """初始化投球车"""
        self.detector = None
        self.target_info = None
        self.detection_thread = None
        self.running = False
        
        # 投篮参数配置
        self.min_confidence = 0.7      # 最小置信度要求
        self.optimal_distance = 3000   # 最佳投篮距离 (mm)
        self.distance_tolerance = 500  # 距离容忍度 (mm)
        
        print("🤖 投球车系统初始化完成")
    
    def initialize_vision_system(self):
        """初始化视觉系统"""
        try:
            print("📷 初始化视觉检测系统...")
            self.detector = YOLODepthDetector(
                weights='best.pt',
                device='cpu',  # 可根据硬件配置改为'cuda'
                conf_thres=0.6,  # 提高置信度要求
                iou_thres=0.45
            )
            print("✅ 视觉系统初始化成功")
            return True
        except Exception as e:
            print(f"❌ 视觉系统初始化失败: {e}")
            return False
    
    def start_detection_thread(self):
        """启动检测线程"""
        if self.detector is None:
            print("❌ 请先初始化视觉系统")
            return False
        
        self.running = True
        self.detection_thread = threading.Thread(target=self._detection_worker)
        self.detection_thread.daemon = True
        self.detection_thread.start()
        print("🔍 检测线程已启动")
        return True
    
    def _detection_worker(self):
        """检测工作线程"""
        try:
            # 启动深度相机
            if not self.detector.depth_camera.start_camera():
                print("⚠️ 深度相机启动失败，使用普通摄像头")
                use_berxel = False
            else:
                use_berxel = True
            
            import cv2
            if not use_berxel:
                cap = cv2.VideoCapture(0)
                if not cap.isOpened():
                    print("❌ 无法打开摄像头")
                    return
            
            while self.running:
                try:
                    # 获取图像
                    if use_berxel:
                        frame = self.detector.depth_camera.get_color_frame()
                        if frame is None:
                            time.sleep(0.03)
                            continue
                        self.detector.depth_camera.load_depth_data()
                    else:
                        ret, frame = cap.read()
                        if not ret:
                            continue
                    
                    # 检测处理
                    detection_results = self._process_frame(frame, use_berxel)
                    
                    # 更新目标信息
                    if detection_results:
                        best_target = self._select_best_target(detection_results)
                        if best_target and self._is_target_valid(best_target):
                            self.target_info = best_target
                    
                    time.sleep(0.04)  # 25 FPS
                    
                except Exception as e:
                    print(f"⚠️ 检测线程错误: {e}")
                    time.sleep(0.1)
            
            # 清理资源
            if use_berxel:
                self.detector.depth_camera.stop_camera()
            else:
                cap.release()
                
        except Exception as e:
            print(f"❌ 检测工作线程异常: {e}")
    
    def _process_frame(self, frame, use_berxel=True):
        """处理单帧图像"""
        # 预处理
        img = self.detector.preprocess(frame)
        
        # 推理
        pred = self.detector.model(img)
        from utils.general import non_max_suppression, scale_boxes
        pred = non_max_suppression(pred, self.detector.conf_thres, self.detector.iou_thres)
        
        # 处理检测结果
        detections = []
        for i, det in enumerate(pred):
            if len(det):
                det[:, :4] = scale_boxes(img.shape[2:], det[:, :4], frame.shape).round()
                for *xyxy, conf, cls in det:
                    confidence = float(conf)
                    if confidence >= 0.5:
                        class_name = self.detector.names[int(cls)]
                        if class_name.lower() != 'person':  # 过滤person
                            x1, y1, x2, y2 = map(int, xyxy)
                            center_x, center_y = (x1 + x2) // 2, (y1 + y2) // 2
                            
                            # 获取距离
                            distance = None
                            if use_berxel:
                                distance = self.detector.depth_camera.get_min_distance_in_bbox(x1, y1, x2, y2)
                            
                            # 检查稳定性
                            is_stable = False
                            if use_berxel:
                                is_stable = self.detector.depth_camera.is_data_stable()
                            
                            detections.append(TargetInfo(
                                position=(center_x, center_y),
                                distance=distance if distance else 0,
                                confidence=confidence,
                                timestamp=time.time(),
                                is_stable=is_stable
                            ))
        
        return detections
    
    def _select_best_target(self, detections):
        """选择最佳目标"""
        if not detections:
            return None
        
        # 优先选择稳定且置信度高的目标
        stable_targets = [d for d in detections if d.is_stable]
        candidates = stable_targets if stable_targets else detections
        
        # 按置信度排序
        candidates.sort(key=lambda x: x.confidence, reverse=True)
        return candidates[0]
    
    def _is_target_valid(self, target):
        """验证目标是否有效"""
        if target.confidence < self.min_confidence:
            return False
        
        if target.distance > 0:
            # 距离检查
            if target.distance < 1000 or target.distance > 10000:  # 1m-10m范围
                return False
        
        return True
    
    def get_current_target(self) -> Optional[TargetInfo]:
        """获取当前目标信息"""
        if self.target_info is None:
            return None
        
        # 检查目标是否过期 (500ms)
        if time.time() - self.target_info.timestamp > 0.5:
            return None
        
        return self.target_info
    
    def calculate_shot_parameters(self, target: TargetInfo):
        """计算投篮参数"""
        if target is None:
            return None
        
        # 简化的投篮参数计算
        distance_m = target.distance / 1000.0  # 转换为米
        
        # 根据距离计算投篮角度和力度
        if distance_m <= 2.0:
            angle = 45  # 近距离低弧度
            power = 60
        elif distance_m <= 4.0:
            angle = 50  # 中距离标准弧度
            power = 75
        else:
            angle = 55  # 远距离高弧度
            power = 90
        
        # 根据水平位置计算方向调整
        img_center_x = 320  # 假设图像宽度640
        horizontal_offset = target.position[0] - img_center_x
        direction_adjustment = horizontal_offset * 0.1  # 简化的方向调整
        
        return {
            'angle': angle,
            'power': power,
            'direction': direction_adjustment,
            'distance': distance_m,
            'confidence': target.confidence
        }
    
    def execute_shot(self):
        """执行投篮"""
        target = self.get_current_target()
        if target is None:
            print("❌ 无有效目标，无法投篮")
            return False
        
        shot_params = self.calculate_shot_parameters(target)
        if shot_params is None:
            print("❌ 无法计算投篮参数")
            return False
        
        print(f"🎯 目标锁定！执行投篮...")
        print(f"   距离: {shot_params['distance']:.2f}m")
        print(f"   置信度: {shot_params['confidence']:.3f}")
        print(f"   角度: {shot_params['angle']}°")
        print(f"   力度: {shot_params['power']}%")
        print(f"   方向调整: {shot_params['direction']:.1f}")
        
        # 这里应该调用实际的投篮机构控制代码
        # self.shooting_mechanism.set_angle(shot_params['angle'])
        # self.shooting_mechanism.set_power(shot_params['power'])
        # self.shooting_mechanism.adjust_direction(shot_params['direction'])
        # self.shooting_mechanism.fire()
        
        print("🚀 投篮执行完成!")
        return True
    
    def autonomous_mode(self):
        """自主模式运行"""
        print("🤖 进入自主模式")
        
        shot_count = 0
        max_shots = 5  # 最大投篮次数
        
        while self.running and shot_count < max_shots:
            target = self.get_current_target()
            
            if target and target.is_stable:
                # 检查是否在最佳投篮距离
                distance_diff = abs(target.distance - self.optimal_distance)
                
                if distance_diff <= self.distance_tolerance:
                    print(f"✅ 目标在最佳距离范围内，准备投篮...")
                    if self.execute_shot():
                        shot_count += 1
                        time.sleep(3)  # 投篮后等待3秒
                else:
                    # 输出距离提示
                    if target.distance > self.optimal_distance + self.distance_tolerance:
                        print(f"📏 目标距离 {target.distance/1000:.2f}m，需要靠近")
                    else:
                        print(f"📏 目标距离 {target.distance/1000:.2f}m，需要后退")
            else:
                print("🔍 正在搜索目标...")
            
            time.sleep(0.5)  # 500ms循环
        
        print(f"🏁 自主模式结束，共投篮 {shot_count} 次")
    
    def stop(self):
        """停止系统"""
        print("🛑 正在停止投球车系统...")
        self.running = False
        if self.detection_thread:
            self.detection_thread.join(timeout=2)
        print("✅ 系统已停止")

def main():
    """主函数 - 投球车集成示例"""
    print("🏀 ROBOCON2025 投球车集成示例")
    print("=" * 50)
    
    robot = BasketballRobot()
    
    try:
        # 初始化视觉系统
        if not robot.initialize_vision_system():
            return
        
        # 启动检测线程
        if not robot.start_detection_thread():
            return
        
        print("\n选择运行模式：")
        print("1. 监控模式 (显示检测状态)")
        print("2. 自主投篮模式")
        print("3. 手动投篮模式")
        
        choice = input("请选择 (1/2/3): ").strip()
        
        if choice == '1':
            # 监控模式
            print("📊 进入监控模式，按 Ctrl+C 退出")
            while True:
                target = robot.get_current_target()
                if target:
                    print(f"🎯 目标: 位置({target.position[0]}, {target.position[1]}) "
                          f"距离{target.distance/1000:.2f}m 置信度{target.confidence:.3f} "
                          f"{'🧘稳定' if target.is_stable else '🚶移动'}")
                else:
                    print("🔍 未检测到目标")
                time.sleep(1)
                
        elif choice == '2':
            # 自主投篮模式
            robot.autonomous_mode()
            
        elif choice == '3':
            # 手动投篮模式
            print("🎮 手动投篮模式，按 Enter 投篮，输入 'q' 退出")
            while True:
                user_input = input("按 Enter 投篮: ")
                if user_input.lower() == 'q':
                    break
                robot.execute_shot()
        
    except KeyboardInterrupt:
        print("\n👋 用户中断")
    except Exception as e:
        print(f"❌ 系统错误: {e}")
    finally:
        robot.stop()

if __name__ == '__main__':
    main() 