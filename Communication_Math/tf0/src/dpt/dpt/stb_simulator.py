import rclpy
from rclpy.node import Node
import serial
import time
import math
import numpy as np

class SerialSender:
    def __init__(self, port='/dev/ttyACM0', baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.serial_port = None
        try:
            self.serial_port = serial.Serial(port, baudrate, timeout=1)
            print(f'串口 {port} 初始化成功')
        except Exception as e:
            print(f'串口初始化失败: {e}')
            print('将使用模拟模式（不实际发送串口数据）')
            self.serial_port = None

    def send(self, message):
        if self.serial_port:
            try:
                if isinstance(message, str):
                    self.serial_port.write(message.encode())
                else:
                    self.serial_port.write(message)
                print(f'串口发送: {message}')
            except Exception as e:
                print(f'串口发送失败: {e}')
        else:
            print(f'模拟发送: {message}')

    def close(self):
        if self.serial_port:
            self.serial_port.close()

class StbSimulator(Node):
    def __init__(self):
        super().__init__('stb_simulator')
        
        # 串口初始化
        self.sender = SerialSender(port='/dev/ttyACM0', baudrate=115200)
        
        # 模拟参数
        self.time_start = time.time()
        self.x = 0.0
        self.y = 0.0
        
        # 创建定时器，0.1秒发送一次，与原stb节点相同
        self.timer = self.create_timer(0.1, self.simulate_and_send)
        
        self.get_logger().info('STB模拟器节点已启动')

    def simulate_and_send(self):
        """模拟坐标变化并发送数据"""
        current_time = time.time()
        elapsed_time = current_time - self.time_start
        
        # 模拟不同的运动模式
        # 模式1: 圆周运动
        # radius = 2.0
        # angular_speed = 0.5  # rad/s
        # self.x = radius * math.cos(angular_speed * elapsed_time)
        # self.y = radius * math.sin(angular_speed * elapsed_time)
        
        # 模式2: 直线运动（注释掉上面的圆周运动，取消注释下面的代码）
        # speed = 0.5  # m/s
        # self.x = speed * elapsed_time
        # self.y = 0.0
        
        # 模式3: 8字形运动（注释掉上面的模式，取消注释下面的代码）
        a = 2.0
        self.x = a * math.sin(elapsed_time)
        self.y = a * math.sin(elapsed_time) * math.cos(elapsed_time)
        
        # 模式4: 随机游走（注释掉上面的模式，取消注释下面的代码）
        # self.x += np.random.normal(0, 0.1)
        # self.y += np.random.normal(0, 0.1)
        
        # 发送数据，格式与原stb节点相同
        message = f"$0,{self.x:.3f},{self.y:.3f},7#"  
        self.sender.send(message)
        
        self.get_logger().info(f'模拟坐标: x={self.x:.3f}, y={self.y:.3f}')

def main(args=None):
    rclpy.init(args=args)
    node = StbSimulator()
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        print('\n收到中断信号，正在关闭...')
    finally:
        node.sender.close()
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main() 