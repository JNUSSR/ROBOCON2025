#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
基础篮筐检测示例
演示如何使用YOLODepthDetector进行基本的篮筐检测
"""

import sys
import os
from pathlib import Path

# 添加项目根目录到路径
project_root = Path(__file__).parent.parent
sys.path.append(str(project_root))

from yolo_depth_detection import YOLODepthDetector

def basic_detection_example():
    """基础检测示例"""
    print("🏀 ROBOCON2025 篮筐检测系统 - 基础示例")
    print("=" * 50)
    
    try:
        # 初始化检测器
        print("📋 初始化检测器...")
        detector = YOLODepthDetector(
            weights='best.pt',          # 篮筐检测模型
            device='cpu',               # 使用CPU，如有GPU可改为'cuda'
            conf_thres=0.5,            # 置信度阈值
            iou_thres=0.45             # NMS IoU阈值
        )
        print("✅ 检测器初始化成功！")
        
        # 开始检测
        print("\n🚀 开始篮筐检测...")
        print("💡 提示：")
        print("   - 按 'q' 键退出检测")
        print("   - 系统会自动检测移动状态")
        print("   - 稳定1秒后输出详细检测信息")
        print("   - 支持实时FPS显示")
        
        # 启动检测（优先使用Berxel深度相机）
        detector.detect_with_distance(use_berxel=True)
        
    except KeyboardInterrupt:
        print("\n👋 检测已停止")
    except Exception as e:
        print(f"❌ 检测过程出错: {e}")
        print("💡 请检查：")
        print("   1. 模型文件 best.pt 是否存在")
        print("   2. 摄像头是否正常连接")
        print("   3. 深度相机驱动是否正确安装")

def simple_frame_detection():
    """简单的单帧检测示例"""
    import cv2
    
    print("\n📸 单帧检测示例")
    print("-" * 30)
    
    try:
        # 初始化检测器
        detector = YOLODepthDetector(weights='best.pt', device='cpu')
        
        # 打开摄像头
        cap = cv2.VideoCapture(0)
        if not cap.isOpened():
            print("❌ 无法打开摄像头")
            return
        
        print("📷 摄像头已打开，按空格键检测，按 'q' 退出")
        
        while True:
            ret, frame = cap.read()
            if not ret:
                break
            
            # 显示预览
            cv2.imshow('按空格键检测', frame)
            
            key = cv2.waitKey(1) & 0xFF
            if key == ord(' '):  # 空格键检测
                print("🔍 正在检测...")
                
                # 预处理图像
                img = detector.preprocess(frame)
                
                # 推理
                pred = detector.model(img)
                from utils.general import non_max_suppression, scale_boxes
                pred = non_max_suppression(pred, detector.conf_thres, detector.iou_thres)
                
                # 处理结果
                detections = []
                for i, det in enumerate(pred):
                    if len(det):
                        det[:, :4] = scale_boxes(img.shape[2:], det[:, :4], frame.shape).round()
                        for *xyxy, conf, cls in det:
                            if float(conf) >= 0.5:
                                class_name = detector.names[int(cls)]
                                if class_name.lower() != 'person':  # 过滤掉person
                                    detections.append({
                                        'class': class_name,
                                        'confidence': float(conf),
                                        'bbox': [int(x) for x in xyxy]
                                    })
                
                # 输出结果
                if detections:
                    print(f"✅ 检测到 {len(detections)} 个目标:")
                    for i, det in enumerate(detections, 1):
                        print(f"   {i}. {det['class']} (置信度: {det['confidence']:.3f})")
                else:
                    print("❌ 未检测到篮筐")
                    
            elif key == ord('q'):
                break
        
        cap.release()
        cv2.destroyAllWindows()
        
    except Exception as e:
        print(f"❌ 单帧检测出错: {e}")

if __name__ == '__main__':
    print("选择检测模式：")
    print("1. 实时检测（推荐）")
    print("2. 单帧检测")
    
    choice = input("请输入选择 (1/2): ").strip()
    
    if choice == '1':
        basic_detection_example()
    elif choice == '2':
        simple_frame_detection()
    else:
        print("无效选择，使用默认实时检测模式")
        basic_detection_example() 