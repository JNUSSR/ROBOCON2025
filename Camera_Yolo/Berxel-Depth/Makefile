# 简化的 Berxel 距离测量工具 Makefile

# 编译器设置
CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall

# 路径设置
INCLUDE_DIR = ./Include
LIBS_DIR = ./libs
OPENCV_INCLUDE = /usr/include/opencv4

# 库设置
LIBS = -lBerxelHawk -lBerxelInterface -Wl,-rpath,./libs
OPENCV_LIBS = -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs

# 目标文件
TARGET1 = distance_meter
TARGET2 = berxel_yolo_bridge
SOURCE1 = distance_meter.cpp
SOURCE2 = berxel_yolo_bridge.cpp

# 默认目标
all: $(TARGET1) $(TARGET2)

# 编译距离测量程序
$(TARGET1): $(SOURCE1)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -I$(OPENCV_INCLUDE) -o $(TARGET1) $(SOURCE1) -L$(LIBS_DIR) $(LIBS) $(OPENCV_LIBS)

# 编译桥接程序
$(TARGET2): $(SOURCE2)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -I$(OPENCV_INCLUDE) -o $(TARGET2) $(SOURCE2) -L$(LIBS_DIR) $(LIBS) $(OPENCV_LIBS) -lstdc++fs

# 运行距离测量程序
run: $(TARGET1)
	LD_LIBRARY_PATH=$(LIBS_DIR):$$LD_LIBRARY_PATH ./$(TARGET1)

# 运行桥接程序
run-bridge: $(TARGET2)
	LD_LIBRARY_PATH=$(LIBS_DIR):$$LD_LIBRARY_PATH ./$(TARGET2)

# 检查依赖
check:
	@echo "检查OpenCV..."
	@pkg-config --exists opencv4 && echo "OpenCV 4.x: OK" || echo "OpenCV 4.x: 未找到"
	@echo "检查Berxel库..."
	@test -f $(LIBS_DIR)/libBerxelHawk.so && echo "BerxelHawk: OK" || echo "BerxelHawk: 未找到"
	@test -f $(LIBS_DIR)/libBerxelInterface.so && echo "BerxelInterface: OK" || echo "BerxelInterface: 未找到"

# 清理
clean:
	rm -f $(TARGET1) $(TARGET2)

# 帮助
help:
	@echo "可用目标:"
	@echo "  all          - 编译所有程序"
	@echo "  distance_meter - 编译距离测量程序"
	@echo "  berxel_yolo_bridge - 编译桥接程序"
	@echo "  run          - 运行距离测量程序"
	@echo "  run-bridge   - 运行桥接程序"
	@echo "  check        - 检查依赖"
	@echo "  clean        - 清理编译文件"
	@echo "  help         - 显示此帮助"

.PHONY: all run run-bridge check clean help 