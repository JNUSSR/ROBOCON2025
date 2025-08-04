import rclpy
from rclpy.node import Node
from tf2_ros import StaticTransformBroadcaster
from geometry_msgs.msg import TransformStamped
import math
from geometry_msgs.msg import Point
from dpt.cktongxin import SerialSender
from tf2_ros import TransformException
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
from dpt.qjhs import l_v
import numpy as np
import time
import serial
def send_serial(x,y,yaw):
    distance=math.sqrt((4-x)**2+(15-y)**2)
    if distance<50:
        sender = SerialSender(port='/dev/ttyESP', baudrate=115200)
        v=l_v(distance)  # 串口初始化
        message = f"${v:.3f},{x:.3f},{y:.3f},e,{np.arctan((abs(15-y)/(4-x)))-yaw}#".encode()
    sender.send(message)
class TfSubscriber(Node):
    def __init__(self):
        super().__init__('tf_subscriber')
        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)
        self.timer = self.create_timer(0.1, self.check_tf)  # 0.1秒接收一次tf，可调
        self.parent_frame = 'camera_init'
        self.child_frame = 'aft_mapped'
        self.x = 0.0
        self.y = 0.0
        self.sender = SerialSender(port='/dev/ttyESP', baudrate=115200)  # 串口初始化
    def check_tf(self):
        try:
            transform = self.tf_buffer.lookup_transform(
                self.parent_frame,
                self.child_frame,
                rclpy.time.Time(),
                timeout=rclpy.duration.Duration(seconds=1.0))
            translation = transform.transform.translation
            rotation=transform.transform.rotation
            yaw=abs(rotation.z)
            # 直接在这里发送:
            send_serial(translation.x,translation.y,yaw)
        except TransformException as ex:
            self.get_logger().warn(f'Failed to get transform: {ex}')

def main(args=None):
    
    rclpy.init(args=args)
    try:
        ser=serial.Serial("/dev/ttyESP,115200")
        ser.write("$1,1,1,1,1#")
        print("send message seccess")
        ser.close()
    except serial.SerialException as e:
        print("fail to open serial")
    except Exception as e:
        print(e)
    node = TfSubscriber()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.sender.serial_port.close()
        node.destroy_node()
        rclpy.shutdown()

if __name__ ==  '__main__':
    main()
     
     

     
