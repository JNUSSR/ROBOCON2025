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
     

        try:
            # 获取最新的变换，等待最多1秒
            transform = self.tf_buffer.lookup_transform(
                self.parent_frame,
                self.child_frame,
                rclpy.time.Time(),
                timeout=rclpy.duration.Duration(seconds=1.0))
            
            translation = transform.transform.translation
            rotation = transform.transform.rotation
            
            self.get_logger().info(f'Translation: ({translation.x}, {translation.y}, {translation.z})')
            self.get_logger().info(f'Rotation: ({rotation.x}, {rotation.y}, {rotation.z}, {rotation.w})')
            self.x=translation.x
            self.y=translation.y
            self.received_transform = True
            self.timer.cancel()  # 停止定时器
            self.get_logger().info("Successfully received transform")
            #rclpy.shutdown()
            
        except TransformException as ex:
            self.get_logger().warn(f'Failed to get transform: {ex}')
            # 可以添加重试次数限制

def find_vector(x,y,z,theta):
    h=2.43-z #篮筐发射器高度差
    hoop_x=15
    hoop_y=4 
    d=((hoop_x-x)**2+(hoop_y-y)**2)**0.5
    g=9.8
    a=(g)*d**2
    b=2*((math.cos(theta))**2)*(d*math.tan(theta)-h)
    vector=(a/b)**0.5
    return float(vector)
#####################################################################
def main(args=None):
    rclpy.init(args=args)
    node = TfSubscriber()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    x=node.x
    y=node.y
    node.destroy_node()
    # rclpy.shutdown()
    distance=np.sqrt((4-abs(x))**2+(15-y)**2)
    d_z=1.53
    theta=np.pi/4
    vector=mnth(object,8,100,0.95,1e-3,100,distance,d_z,theta,7,10)
    print(vector)
    # serial_send_node = SerialSender(vector)
    # rclpy.spin(serial_send_node)
    #rclpy.spin(node)#循环节点
    # serial_send_node.destroy_node()
    rclpy.shutdown()
if __name__ ==  '__main__':
    main()
     
     
