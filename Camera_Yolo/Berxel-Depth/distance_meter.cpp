#include <opencv2/opencv.hpp>
#include <iostream>
#include <signal.h>
#include <csignal>
#include "Include/BerxelHawkContext.h"
#include "Include/BerxelHawkDevice.h"
#include "Include/BerxelHawkFrame.h"

using namespace berxel;
using namespace cv;
using namespace std;

// 全局变量
BerxelHawkContext* g_context = nullptr;
BerxelHawkDevice* g_device = nullptr;
Mat g_colorImage, g_depthImage;
BerxelHawkPixelType g_currentPixelType;
bool g_running = true;
int g_colorMapType = COLORMAP_JET;  // 默认颜色映射

// 信号处理函数
void signalHandler(int signal) {
    cout << "\n接收到信号: " << signal << ", 准备退出..." << endl;
    g_running = false;
}

// 鼠标回调函数
void onMouse(int event, int x, int y, int flags, void* userdata) {
    if (event == EVENT_LBUTTONDOWN && !g_depthImage.empty()) {
        if (x >= 0 && y >= 0 && x < g_depthImage.cols && y < g_depthImage.rows) {
            uint16_t rawDepthValue = g_depthImage.at<uint16_t>(y, x);
            
            if (rawDepthValue > 0) {
                float depthInMm = 0.0f;
                if (g_currentPixelType == BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D) {
                    depthInMm = rawDepthValue * 0.125f;
                } else {
                    depthInMm = rawDepthValue * 1.0f;
                }
                
                cout << "点击位置: (" << x << ", " << y << ") - "
                     << "距离: " << depthInMm << "mm ("
                     << depthInMm / 10.0f << "cm, "
                     << depthInMm / 1000.0f << "m) "
                     << "像素类型: " << g_currentPixelType << endl;
            } else {
                cout << "点击位置: (" << x << ", " << y << ") - 无有效深度数据" << endl;
            }
        }
    }
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

// 捕获帧
bool captureFrames() {
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
    
    // 转换彩色帧到OpenCV Mat
    if (colorFrame) {
        int colorWidth = colorFrame->getWidth();
        int colorHeight = colorFrame->getHeight();
        uint8_t* colorData = (uint8_t*)colorFrame->getData();
        
        g_colorImage = Mat(colorHeight, colorWidth, CV_8UC3, colorData).clone();
        cvtColor(g_colorImage, g_colorImage, COLOR_RGB2BGR);
    }
    
    // 转换深度帧到OpenCV Mat并获取像素类型
    if (depthFrame) {
        int depthWidth = depthFrame->getWidth();
        int depthHeight = depthFrame->getHeight();
        uint16_t* depthData = (uint16_t*)depthFrame->getData();
        
        g_depthImage = Mat(depthHeight, depthWidth, CV_16UC1, depthData).clone();
        g_currentPixelType = depthFrame->getPixelType();
        
        g_device->releaseFrame(colorFrame);
        g_device->releaseFrame(depthFrame);
        return true;
    }
    
    if (colorFrame) g_device->releaseFrame(colorFrame);
    if (depthFrame) g_device->releaseFrame(depthFrame);
    return false;
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
    
    destroyAllWindows();
    cout << "清理完成" << endl;
}

int main() {
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    cout << "=== Berxel Hawk 距离测量工具 ===" << endl;
    cout << "使用说明:" << endl;
    cout << "1. 在彩色图像窗口中左键点击可查看距离" << endl;
    cout << "2. 按 ESC 键退出程序" << endl;
    cout << "3. 也可使用 Ctrl+C 强制退出" << endl;
    cout << "4. 按数字键 1-6 切换深度图颜色:" << endl;
    cout << "   1-JET(彩虹), 2-HOT(热力), 3-COOL(冷色), 4-GRAY(灰度), 5-BONE(骨架), 6-AUTUMN(秋色)" << endl;
    cout << "=================================" << endl;
    
    // 初始化相机
    if (!initializeCamera()) {
        cerr << "相机初始化失败" << endl;
        return -1;
    }
    
    // 创建窗口
    namedWindow("彩色图像 (点击测距)", WINDOW_AUTOSIZE);
    namedWindow("深度图像", WINDOW_AUTOSIZE);
    
    // 设置鼠标回调
    setMouseCallback("彩色图像 (点击测距)", onMouse, nullptr);
    
    cout << "相机初始化成功,开始显示图像..." << endl;
    
    // 主循环
    while (g_running) {
        if (captureFrames()) {
            if (!g_colorImage.empty()) {
                imshow("彩色图像 (点击测距)", g_colorImage);
            }
            
            if (!g_depthImage.empty()) {
                Mat depthDisplay;
                g_depthImage.convertTo(depthDisplay, CV_8UC1, 255.0 / 4000.0);
                
                // 应用颜色映射
                if (g_colorMapType == -1) {
                    // 灰度显示（原始风格）
                    depthDisplay.copyTo(depthDisplay);
                } else {
                    applyColorMap(depthDisplay, depthDisplay, g_colorMapType);
                }
                
                imshow("深度图像", depthDisplay);
            }
        }
        
        // 检查按键
        int key = waitKey(1) & 0xFF;
        if (key == 27) { // ESC键
            break;
        }
        // 颜色映射切换
        else if (key >= '1' && key <= '6') {
            switch(key) {
                case '1': g_colorMapType = COLORMAP_JET; cout << "切换到 JET 颜色映射（彩虹色）" << endl; break;
                case '2': g_colorMapType = COLORMAP_HOT; cout << "切换到 HOT 颜色映射（热力图）" << endl; break;
                case '3': g_colorMapType = COLORMAP_COOL; cout << "切换到 COOL 颜色映射（冷色调）" << endl; break;
                case '4': g_colorMapType = -1; cout << "切换到 GRAY 颜色映射（灰度）" << endl; break;
                case '5': g_colorMapType = COLORMAP_BONE; cout << "切换到 BONE 颜色映射（骨架风格）" << endl; break;
                case '6': g_colorMapType = COLORMAP_AUTUMN; cout << "切换到 AUTUMN 颜色映射（秋色）" << endl; break;
            }
        }
    }
    
    cleanup();
    return 0;
} 