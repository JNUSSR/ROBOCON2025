#include <opencv2/opencv.hpp>
#include <iostream>
#include <signal.h>
#include <csignal>
#include <fstream>
#include <mutex>
#include <thread>
#include <chrono>
#include <cstdio>
#include <unistd.h>
#include <experimental/filesystem>
#include <sstream>
#include <climits>
#include <algorithm>
#include <vector>
#include "Include/BerxelHawkContext.h"
#include "Include/BerxelHawkDevice.h"
#include "Include/BerxelHawkFrame.h"

using namespace berxel;
using namespace cv;
using namespace std;
namespace fs = std::experimental::filesystem;

// 全局变量
BerxelHawkContext* g_context = nullptr;
BerxelHawkDevice* g_device = nullptr;
Mat g_colorImage, g_depthImage;
BerxelHawkPixelType g_currentPixelType;
bool g_running = true;
mutex g_imageMutex;

// 共享数据文件路径
const string COLOR_IMAGE_PATH = "/tmp/berxel_color.jpg";
const string DEPTH_DATA_PATH = "/tmp/berxel_depth.bin";
const string STATUS_FILE_PATH = "/tmp/berxel_status.txt";
const string QUERY_FILE = "/tmp/berxel_query.txt";
const string RESPONSE_FILE = "/tmp/berxel_response.txt";

// 信号处理函数
void signalHandler(int signal) {
    cout << "\n接收到信号: " << signal << ", 准备退出..." << endl;
    g_running = false;
}

// 初始化相机
bool initializeCamera() {
    cout << "正在初始化 Berxel Hawk 深度相机..." << endl;
    
    // 获取Berxel上下文
    g_context = BerxelHawkContext::getBerxelContext();
    if (!g_context) {
        cerr << "错误: 无法获取Berxel上下文" << endl;
        return false;
    }
    
    // 获取设备列表
    BerxelHawkDeviceInfo* deviceList = nullptr;
    uint32_t deviceCount = 0;
    if (g_context->getDeviceList(&deviceList, &deviceCount) != 0) {
        cerr << "错误: 无法获取设备列表" << endl;
        return false;
    }
    
    if (deviceCount == 0) {
        cerr << "错误: 未发现设备" << endl;
        return false;
    }
    
    cout << "发现 " << deviceCount << " 个设备" << endl;
    
    // 打开第一个设备
    g_device = g_context->openDevice(deviceList[0]);
    if (!g_device) {
        cerr << "错误: 无法打开设备" << endl;
        return false;
    }
    
    cout << "成功打开设备" << endl;
    
    // 启动彩色和深度流
    int streamFlags = BERXEL_HAWK_COLOR_STREAM | BERXEL_HAWK_DEPTH_STREAM;
    if (g_device->startStreams(streamFlags) != 0) {
        cerr << "错误: 无法启动数据流" << endl;
        return false;
    }
    
    cout << "成功启动数据流" << endl;
    return true;
}

// 捕获帧并保存到文件
bool captureAndSaveFrames() {
    BerxelHawkFrame* colorFrame = nullptr;
    BerxelHawkFrame* depthFrame = nullptr;
    
    // 读取彩色帧
    if (g_device->readColorFrame(colorFrame, 100) != 0) {
        return false;
    }
    
    // 读取深度帧
    if (g_device->readDepthFrame(depthFrame, 100) != 0) {
        if (colorFrame) g_device->releaseFrame(colorFrame);
        return false;
    }
    
    lock_guard<mutex> lock(g_imageMutex);
    
    // 转换彩色帧到OpenCV Mat并保存
    if (colorFrame) {
        int colorWidth = colorFrame->getWidth();
        int colorHeight = colorFrame->getHeight();
        uint8_t* colorData = (uint8_t*)colorFrame->getData();
        
        g_colorImage = Mat(colorHeight, colorWidth, CV_8UC3, colorData).clone();
        cvtColor(g_colorImage, g_colorImage, COLOR_RGB2BGR);
        
        // 保存彩色图像
        imwrite(COLOR_IMAGE_PATH, g_colorImage);
    }
    
    // 转换深度帧到OpenCV Mat并保存深度数据
    if (depthFrame) {
        int depthWidth = depthFrame->getWidth();
        int depthHeight = depthFrame->getHeight();
        uint16_t* depthData = (uint16_t*)depthFrame->getData();
        
        g_depthImage = Mat(depthHeight, depthWidth, CV_16UC1, depthData).clone();
        g_currentPixelType = depthFrame->getPixelType();
        
        // 保存深度数据（二进制格式）
        ofstream depthFile(DEPTH_DATA_PATH, ios::binary);
        if (depthFile.is_open()) {
            // 写入尺寸信息
            depthFile.write(reinterpret_cast<const char*>(&depthWidth), sizeof(int));
            depthFile.write(reinterpret_cast<const char*>(&depthHeight), sizeof(int));
            depthFile.write(reinterpret_cast<const char*>(&g_currentPixelType), sizeof(BerxelHawkPixelType));
            
            // 写入深度数据
            depthFile.write(reinterpret_cast<const char*>(g_depthImage.data), 
                          depthWidth * depthHeight * sizeof(uint16_t));
            depthFile.close();
        }
        
        g_device->releaseFrame(colorFrame);
        g_device->releaseFrame(depthFrame);
        return true;
    }
    
    if (colorFrame) g_device->releaseFrame(colorFrame);
    if (depthFrame) g_device->releaseFrame(depthFrame);
    return false;
}

// 获取指定点的距离
float getDistanceAtPoint(int x, int y) {
    lock_guard<mutex> lock(g_imageMutex);
    
    if (g_depthImage.empty() || x < 0 || y < 0 || 
        x >= g_depthImage.cols || y >= g_depthImage.rows) {
        return -1.0f;
    }
    
    uint16_t rawDepthValue = g_depthImage.at<uint16_t>(y, x);
    
    if (rawDepthValue > 0) {
        float depthInMm = 0.0f;
        if (g_currentPixelType == BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D) {
            depthInMm = rawDepthValue * 0.125f;
        } else {
            depthInMm = rawDepthValue * 1.0f;
        }
        return depthInMm;
    }
    
    return -1.0f;
}

// 写入状态文件
void writeStatusFile() {
    ofstream statusFile(STATUS_FILE_PATH);
    if (statusFile.is_open()) {
        statusFile << "RUNNING\n";
        statusFile << time(nullptr) << "\n";
        if (!g_colorImage.empty()) {
            statusFile << g_colorImage.cols << " " << g_colorImage.rows << "\n";
        } else {
            statusFile << "0 0\n";
        }
        statusFile.close();
    }
}

/**
 * 获取指定区域内的最小距离 - 针对篮筐等网状物体优化
 * @param x1, y1, x2, y2 矩形区域坐标
 * @return 最小距离值(mm)，0表示无效
 */
double getMinDistanceInRegion(int x1, int y1, int x2, int y2) {
    if (g_depthImage.empty()) {
        return 0.0;
    }
    
    // 确保坐标在有效范围内
    int width = g_depthImage.cols;
    int height = g_depthImage.rows;
    
    x1 = max(0, min(x1, width - 1));
    y1 = max(0, min(y1, height - 1));
    x2 = max(0, min(x2, width - 1));
    y2 = max(0, min(y2, height - 1));
    
    // 确保x1 <= x2, y1 <= y2
    if (x1 > x2) swap(x1, x2);
    if (y1 > y2) swap(y1, y2);
    
    double min_distance = DBL_MAX;
    int valid_points = 0;
    vector<double> valid_distances;
    
    // 策略1: 密集采样整个区域（每隔1个像素采样一次，提高密度）
    for (int y = y1; y <= y2; y += 1) {
        for (int x = x1; x <= x2; x += 1) {
            uint16_t rawDepthValue = g_depthImage.at<uint16_t>(y, x);
            
            if (rawDepthValue > 0) {
                double depthInMm = 0.0;
                if (g_currentPixelType == BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D) {
                    depthInMm = rawDepthValue * 0.125;
                } else {
                    depthInMm = rawDepthValue * 1.0;
                }
                
                // 有效深度值检查 (20mm-10米范围内，排除过近的噪声)
                if (depthInMm > 20 && depthInMm < 10000) {
                    valid_distances.push_back(depthInMm);
                    if (depthInMm < min_distance) {
                        min_distance = depthInMm;
                    }
                    valid_points++;
                }
            }
        }
    }
    
    // 策略2: 如果有效点不够，尝试边框重点采样（篮筐边缘更可能有深度数据）
    if (valid_points < 10) {
        int box_width = x2 - x1 + 1;
        int box_height = y2 - y1 + 1;
        
        // 采样上边框
        for (int x = x1; x <= x2; x += max(1, box_width / 20)) {
            for (int y_offset = 0; y_offset <= min(5, box_height / 4); y_offset++) {
                int y = y1 + y_offset;
                if (y > y2) break;
                
                uint16_t rawDepthValue = g_depthImage.at<uint16_t>(y, x);
                if (rawDepthValue > 0) {
                    double depthInMm = (g_currentPixelType == BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D) 
                                      ? rawDepthValue * 0.125 : rawDepthValue * 1.0;
                    
                    if (depthInMm > 20 && depthInMm < 10000) {
                        valid_distances.push_back(depthInMm);
                        if (depthInMm < min_distance) {
                            min_distance = depthInMm;
                        }
                        valid_points++;
                    }
                }
            }
        }
        
        // 采样下边框
        for (int x = x1; x <= x2; x += max(1, box_width / 20)) {
            for (int y_offset = 0; y_offset <= min(5, box_height / 4); y_offset++) {
                int y = y2 - y_offset;
                if (y < y1) break;
                
                uint16_t rawDepthValue = g_depthImage.at<uint16_t>(y, x);
                if (rawDepthValue > 0) {
                    double depthInMm = (g_currentPixelType == BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D) 
                                      ? rawDepthValue * 0.125 : rawDepthValue * 1.0;
                    
                    if (depthInMm > 20 && depthInMm < 10000) {
                        valid_distances.push_back(depthInMm);
                        if (depthInMm < min_distance) {
                            min_distance = depthInMm;
                        }
                        valid_points++;
                    }
                }
            }
        }
        
        // 采样左右边框
        for (int y = y1; y <= y2; y += max(1, box_height / 20)) {
            // 左边框
            for (int x_offset = 0; x_offset <= min(5, box_width / 4); x_offset++) {
                int x = x1 + x_offset;
                if (x > x2) break;
                
                uint16_t rawDepthValue = g_depthImage.at<uint16_t>(y, x);
                if (rawDepthValue > 0) {
                    double depthInMm = (g_currentPixelType == BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D) 
                                      ? rawDepthValue * 0.125 : rawDepthValue * 1.0;
                    
                    if (depthInMm > 20 && depthInMm < 10000) {
                        valid_distances.push_back(depthInMm);
                        if (depthInMm < min_distance) {
                            min_distance = depthInMm;
                        }
                        valid_points++;
                    }
                }
            }
            
            // 右边框
            for (int x_offset = 0; x_offset <= min(5, box_width / 4); x_offset++) {
                int x = x2 - x_offset;
                if (x < x1) break;
                
                uint16_t rawDepthValue = g_depthImage.at<uint16_t>(y, x);
                if (rawDepthValue > 0) {
                    double depthInMm = (g_currentPixelType == BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D) 
                                      ? rawDepthValue * 0.125 : rawDepthValue * 1.0;
                    
                    if (depthInMm > 20 && depthInMm < 10000) {
                        valid_distances.push_back(depthInMm);
                        if (depthInMm < min_distance) {
                            min_distance = depthInMm;
                        }
                        valid_points++;
                    }
                }
            }
        }
    }
    
    // 策略3: 如果仍然没有足够的有效点，扩展搜索区域
    if (valid_points < 5) {
        int expand_size = min(10, min(width, height) / 20);  // 扩展10像素或图像尺寸的5%
        
        int expand_x1 = max(0, x1 - expand_size);
        int expand_y1 = max(0, y1 - expand_size);
        int expand_x2 = min(width - 1, x2 + expand_size);
        int expand_y2 = min(height - 1, y2 + expand_size);
        
        // 在扩展区域进行稀疏采样
        for (int y = expand_y1; y <= expand_y2; y += 3) {
            for (int x = expand_x1; x <= expand_x2; x += 3) {
                // 跳过原始区域，只采样扩展的部分
                if (x >= x1 && x <= x2 && y >= y1 && y <= y2) {
                    continue;
                }
                
                uint16_t rawDepthValue = g_depthImage.at<uint16_t>(y, x);
                if (rawDepthValue > 0) {
                    double depthInMm = (g_currentPixelType == BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D) 
                                      ? rawDepthValue * 0.125 : rawDepthValue * 1.0;
                    
                    if (depthInMm > 20 && depthInMm < 10000) {
                        valid_distances.push_back(depthInMm);
                        if (depthInMm < min_distance) {
                            min_distance = depthInMm;
                        }
                        valid_points++;
                    }
                }
            }
        }
    }
    
    // 数据验证和过滤
    if (valid_points == 0 || min_distance == DBL_MAX) {
        return 0.0;
    }
    
    // 如果有效点较少（<3个），可能不可靠，返回0
    if (valid_points < 3) {
        return 0.0;
    }
    
    // 对于篮筐检测，过滤掉可能的异常近距离值（背景穿透等）
    if (valid_distances.size() > 1) {
        sort(valid_distances.begin(), valid_distances.end());
        
        // 取最小的几个值的平均，避免单点噪声
        int sample_count = min(5, (int)valid_distances.size());
        double sum = 0.0;
        for (int i = 0; i < sample_count; i++) {
            sum += valid_distances[i];
        }
        return sum / sample_count;
    }
    
    return min_distance;
}

/**
 * 处理距离查询请求
 */
void processDistanceQuery() {
    if (!fs::exists(QUERY_FILE)) {
        return;
    }
    
    ifstream query_file(QUERY_FILE);
    if (!query_file.is_open()) {
        return;
    }
    
    string line;
    if (getline(query_file, line)) {
        query_file.close();
        
        // 解析坐标 - 现在期望格式为 "x1 y1 x2 y2"
        istringstream iss(line);
        int x1, y1, x2, y2;
        
        if (iss >> x1 >> y1 >> x2 >> y2) {
            // 获取区域内最小距离
            lock_guard<mutex> lock(g_imageMutex);
            double distance = getMinDistanceInRegion(x1, y1, x2, y2);
            
            // 写入响应文件
            ofstream response_file(RESPONSE_FILE);
            if (response_file.is_open()) {
                response_file << distance << endl;
                response_file.close();
            }
        }
        
        // 删除查询文件
        fs::remove(QUERY_FILE);
    } else {
        query_file.close();
    }
}

// 清理资源
void cleanup() {
    cout << "正在清理资源..." << endl;
    
    if (g_device) {
        g_device->stopStreams(BERXEL_HAWK_COLOR_STREAM | BERXEL_HAWK_DEPTH_STREAM);
        g_context->closeDevice(g_device);
        g_device = nullptr;
    }
    
    if (g_context) {
        BerxelHawkContext::destroyBerxelContext(g_context);
        g_context = nullptr;
    }
    
    // 清理临时文件
    remove(COLOR_IMAGE_PATH.c_str());
    remove(DEPTH_DATA_PATH.c_str());
    remove(STATUS_FILE_PATH.c_str());
    remove(QUERY_FILE.c_str());
    remove(RESPONSE_FILE.c_str());
    
    cout << "清理完成" << endl;
}

int main() {
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    cout << "=== Berxel Hawk 数据桥接程序 ===" << endl;
    cout << "功能: 为Python程序提供深度相机数据" << endl;
    cout << "数据输出路径:" << endl;
    cout << "- 彩色图像: " << COLOR_IMAGE_PATH << endl;
    cout << "- 深度数据: " << DEPTH_DATA_PATH << endl;
    cout << "- 状态文件: " << STATUS_FILE_PATH << endl;
    cout << "=================================" << endl;
    
    // 初始化相机
    if (!initializeCamera()) {
        cerr << "相机初始化失败" << endl;
        return -1;
    }
    
    cout << "数据桥接程序启动成功，开始提供数据..." << endl;
    
    // 主循环
    auto lastFrameTime = chrono::steady_clock::now();
    while (g_running) {
        if (captureAndSaveFrames()) {
            writeStatusFile();
            
            // 每帧输出一次信息（限制频率）
            auto currentTime = chrono::steady_clock::now();
            auto elapsed = chrono::duration_cast<chrono::milliseconds>(currentTime - lastFrameTime);
            if (elapsed.count() > 1000) {  // 每秒输出一次
                cout << "数据更新: " << time(nullptr) << endl;
                lastFrameTime = currentTime;
            }
        }
        
        // 处理距离查询
        processDistanceQuery();
        
        // 短暂休眠
        this_thread::sleep_for(chrono::milliseconds(33));  // ~30 FPS
    }
    
    cleanup();
    return 0;
} 