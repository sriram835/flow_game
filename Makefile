
CXX      = g++
BUILD_DIR = build
SRC_DIR = src
TARGET    = $(BUILD_DIR)/game.exe
SOURCES  = $(SRC_DIR)/main.cpp $(SRC_DIR)/board.cpp $(SRC_DIR)/algorithm.cpp
HEADER = header/**

RAYLIB_PATH = C:/msys64/ucrt64

CXXFLAGS = -v -I$(RAYLIB_PATH)/include -Iheader

LDFLAGS  = -L$(RAYLIB_PATH)/lib -lraylib -lgdi32 -lwinmm

all: $(TARGET)

$(TARGET): $(SOURCES)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(SOURCES) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)
