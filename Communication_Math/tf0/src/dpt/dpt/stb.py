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
from dpt.qjhs import mnth,gd,object
import numpy as np
import time
class TfSubscriber(Node):
    def __init__(self):
        super().__init__('tf_subscriber')
        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)
        self.timer = self.create_timer(0.1, self.check_tf)  # 0.1秒接收一次tf，可调
        self.parent_frame = 'camera_init'
        self.child_frame = 'odom'
        self.x = 0.0
        self.y = 0.0
        self.sender = SerialSender(port='/dev/ttyACM0', baudrate=115200)  # 串口初始化

    def check_tf(self):
        try:
            transform = self.tf_buffer.lookup_transform(
                self.parent_frame,
                self.child_frame,
                rclpy.time.Time(),
                timeout=rclpy.duration.Duration(seconds=1.0))
            translation = transform.transform.translation
            self.x = translation.x
            self.y = translation.y
            # 直接在这里发送
            message = f"$e,{self.x:.3f},{self.y:.3f},7#"  
            self.sender.send(message)
            self.get_logger().info(f"Send ,{self.x:.3f},{self.y:.3f},")
        except TransformException as ex:
            self.get_logger().warn(f'Failed to get transform: {ex}')

def main(args=None):
    rclpy.init(args=args)
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
     
     
