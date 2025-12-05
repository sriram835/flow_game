CXX      = g++
BUILD_DIR = build
SRC_DIR = src
TARGET    = $(BUILD_DIR)/game.exe
SRC      = $(SRC_DIR)/**.cpp
HEADER = header/** 

RAYLIB_PATH = C:/raylib

CXXFLAGS = -v -I$(RAYLIB_PATH)/include -Iheader

LDFLAGS  = -L$(RAYLIB_PATH)/lib -lraylib -lgdi32 -lwinmm -static

all: $(TARGET)

$(TARGET): $(SRC)
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

clean:
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)