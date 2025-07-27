import rclpy
from rclpy.node import Node
import serial
import time

class SerialSender(Node):
    def __init__(self, port='/dev/ttyACM0', baudrate=115200):
        super().__init__('serial_sender')
        self.get_logger().info('Serial Sender Node initialized')
        self.serial_port = serial.Serial(port, baudrate, timeout=1)

    def send(self, message):
        if isinstance(message, str):
            self.serial_port.write(message.encode())
        else:
            self.serial_port.write(message)
        self.get_logger().info(f'Sent: {message}')
