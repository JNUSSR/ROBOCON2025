import math
import numpy as np
import random
import matplotlib.pyplot as plt
import pandas as pd
import os
current_dir = os.getcwd()
file_path = os.path.join(current_dir, 'velocity_vs_distance.csv')
data=pd.read_csv(file_path)
data_np=data.to_numpy()
distance=data_np[:,0]
velocity=data_np[:,1]
def l_v(d):
     index=d//0.014
     v=velocity[int(index)]
     if index>999:
          return 0
     else :
        return v
#高度计算函数()
def gd(x_t,v,theta):
    # 初始化参数
    m = 0.60  # 篮球质量 (kg)
    R = 0.117  # 篮球半径 (m)
    rho = 1.18  # 空气密度 (kg/m^3)
    Cd = 0.40  # 空气阻力系数
    A = np.pi * R**2  # 篮球横截面积 (m^2)
    x0, y0 = 0, 0  # 初始位置 (m)
    vx0, vy0 = v * np.cos(theta), v * np.sin(theta)  # 初始速度 (m/s)
    omega = 10  # 角速度 (rad/s)
    Gamma = omega * np.pi * R**2  # 环流强度
    g = 9.81  # 重力加速度 (m/s^2)
    dt = 0.05  # 时间步长 (s)
    # 初始化变量
    x, y = x0, y0
    vx, vy = vx0, vy0
    t = 0
    # 模拟篮球运动
    while x < x_t and y >= 0:
        v = np.sqrt(vx**2 + vy**2)  # 速度大小
        theta = np.arctan2(vy, vx)  # 速度方向夹角
        Fdx = -0.5 * Cd * rho * A * v * vx  # 空气阻力 x 分量
        Fdy = -0.5 * Cd * rho * A * v * vy  # 空气阻力 y 分量
        Fmx = -0.5 * rho * v * Gamma * np.sin(theta)  # 马格努斯力 x 分量
        Fmy = 0.5 * rho * v * Gamma * np.cos(theta)  # 马格努斯力 y 分量
        ax = (Fdx + Fmx) / m  # x 方向加速度
        ay = (Fdy + Fmy - m * g) / m  # y 方向加速度
        vx += ax * dt  # 更新 x 方向速度
        vy += ay * dt  # 更新 y 方向速度
        x += vx * dt  # 更新 x 位置
        y += vy * dt  # 更新 y 位置
        t += dt  # 更新时间
    # 如果 x 超过目标 x，插值计算对应的 y
    if x > x_t:
        y = y - (x - x_t) / vx * vy
    return y
#优化目标函数
def object(distance,d_z,v,theta):
    o_value=abs(d_z-gd(distance,v,theta))
    return o_value
def mnth(objective_function, v, t_0, cooling_rate, t_end, m_iter, distance, d_z, theta, lb, ub, num_neighbors=5):
    """
    模拟退火算法
    :param objective_function: 目标函数
    :param v: 初始解
    :param t_0: 初始温度
    :param cooling_rate: 降温率
    :param t_end: 结束温度
    :param m_iter: 最大迭代次数
    :param distance: 目标函数的参数
    :param d_z: 目标函数的参数
    :param theta: 目标函数的参数
    :param lb: 解的下界
    :param ub: 解的上界
    :param num_neighbors: 每次迭代生成的邻域解数量
    :return: 最优解
    """
    c_solution = v
    c_value = objective_function(distance, d_z, v, theta)
    best_solution = c_solution
    best_value = c_value
    iter = 0
    t = t_0

    while t > t_end and iter < m_iter:
        # 生成多个邻域解
        neighbors = []
        for _ in range(num_neighbors):
            new_solution = c_solution + np.random.uniform(-1, 1)
            new_solution = max(lb, min(new_solution, ub))  
            new_value = objective_function(distance, d_z, new_solution, theta)
            neighbors.append((new_solution, new_value))

        # 从多个邻域解中选择最优的一个
        neighbors.sort(key=lambda x: x[1])  # 按目标函数值排序
        new_solution, new_value = neighbors[0]  # 选择目标函数值最小的解

        d_value = new_value - c_value
        if d_value < 0 or random.uniform(0, 1) < math.exp(-d_value / t):
            c_solution = new_solution
            c_value = new_value
            if c_value < best_value:
                best_solution = c_solution
                best_value = c_value

        t *= cooling_rate
        iter += 1

    return best_solution
# def main():
# #test
#     v0=9
#     distance=6
#     theta=math.pi/4
#     d_z=1.53
#     v=mnth(object,v0,100,0.95,1e-3,100,distance,d_z,theta,7,10)
#     print(v)
#     print(gd(distance,v,theta))
