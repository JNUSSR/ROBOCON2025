#!/usr/bin/env python3
"""
针对ACM0串口的测试脚本
用于测试ESP32-S3设备的串口通信和ESP-NOW发送功能
"""

import serial
import time
import sys

def test_acm0_communication():
    """测试ACM0串口通信"""
    print("=== ACM0串口转ESP-NOW项目测试 ===")
    print("串口设备: /dev/ttyACM0")
    print("波特率: 115200")
    
    try:
        # 连接ACM0串口
        print("正在连接串口...")
        ser = serial.Serial('/dev/ttyACM0', 115200, timeout=2)
        # time.sleep(2)  # 等待设备启动
        
        print(f"成功连接到串口: {ser.port}")
        
        # 清空接收缓冲区
        ser.reset_input_buffer()
        
        # 等待设备初始化完成
        print("等待设备初始化...")
        time.sleep(5)
        
        # 发送测试数据
        test_data = [
            "sudu:100,x:200,y:300,z:400",  # 带标签格式
                        # 简单格式
        ]
        
        print("\n开始发送测试数据...")
        for i, data in enumerate(test_data, 1):
            print(f"\n测试 {i}: 发送数据 '{data}'")
            
            # 发送数据
            ser.write((data + '\n').encode())
            time.sleep(2)  # 等待处理时间
            
            # 读取响应
        #     if ser.in_waiting > 0:
        #         response = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
        #         print(f"设备响应: {response.strip()}")
        #     else:
        #         print("未收到响应")
            
        #     time.sleep(3)  # 等待更长时间
        
        # print("\n测试完成!")
        return True
        
    except serial.SerialException as e:
        print(f"串口连接失败: {e}")
        print("请检查:")
        print("1. ESP32-S3设备是否正确连接到USB")
        print("2. 串口设备是否为/dev/ttyACM0")
        print("3. 串口权限是否正确设置")
        return False
        
    except Exception as e:
        print(f"测试过程中出现错误: {e}")
        return False
        
    finally:
        if 'ser' in locals():
            ser.close()
            print("串口连接已关闭")

def interactive_acm0_mode():
    """ACM0交互式模式"""
    print("=== ACM0交互式串口通信模式 ===")
    print("串口设备: /dev/ttyACM0")
    print("输入数据格式:")
    print("- 带标签: sudu:100,x:200,y:300,z:400")
    print("- 简单格式: 100,200,300,400")
    print("- 输入 'quit' 退出")
    
    try:
        ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
        time.sleep(3)
        print(f"已连接到串口: {ser.port}")
        
        while True:
            # 读取设备输出
            if ser.in_waiting > 0:
                output = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
                if output.strip():
                    print(f"设备: {output.strip()}")
            
            # 获取用户输入
            try:
                user_input = input("输入数据: ").strip()
                if user_input.lower() == 'quit':
                    break
                
                if user_input:
                    # 发送数据
                    ser.write((user_input + '\n').encode())
                    time.sleep(1)
                    
            except KeyboardInterrupt:
                break
            except EOFError:
                break
                
    except serial.SerialException as e:
        print(f"串口连接失败: {e}")
        return
        
    except Exception as e:
        print(f"交互模式错误: {e}")
    
    finally:
        if 'ser' in locals():
            ser.close()
            print("交互模式结束")

if __name__ == "__main__":
    # if len(sys.argv) > 1 and sys.argv[1] == "--interactive":
    #     interactive_acm0_mode()
    # else:
    #     test_acm0_communication() 
    test_num=int(input("====输入1:自动测试 输入2交互模式===="))
    if test_num==1:
        test_acm0_communication()
    elif test_num==2:
        interactive_acm0_mode()
    else:
        print("你输了个啥")
    