CXX := g++
# CXXFLAGS := -std=c++17 -O2 -Iinclude -Ithird_party/imgui -Ithird_party/imgui/backends
CXXFLAGS := -std=c++17 -O2 -Iinclude -Ithird_party/imgui -Ithird_party/imgui/backends -I$(HOME)/brew/include
GLFW_DIR := third_party/glfw
GLFW_LIB := $(GLFW_DIR)/build/src/libglfw3.a
GLFW_INC := -I$(GLFW_DIR)/include

OSX_FRAMEWORKS := \
	-framework Cocoa \
	-framework IOKit \
	-framework CoreVideo \
	-framework QuartzCore \
	-framework Metal


UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
    CXXFLAGS += -I$(HOME)/brew/include -DNDEBUG -DVK_ENABLE_BETA_EXTENSIONS
    LDFLAGS := -L$(HOME)/brew/lib
    # LDLIBS := -lglfw -lvulkan -lcurl
	LDLIBS := -lvulkan -lcurl $(OSX_FRAMEWORKS)
    OSX_FRAMEWORKS := -framework Cocoa -framework IOKit -framework CoreVideo -framework QuartzCore -framework Metal
    LDLIBS += $(OSX_FRAMEWORKS)
else
    # Linux
    LDLIBS := $(shell pkg-config --libs glfw3) -lvulkan -ldl -lpthread -lcurl
endif

SRC_DIR := src
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:.cpp=.o)

IMGUI := third_party/imgui
IMGUI_SRCS := \
  $(IMGUI)/imgui.cpp \
  $(IMGUI)/imgui_draw.cpp \
  $(IMGUI)/imgui_widgets.cpp \
  $(IMGUI)/imgui_tables.cpp \
  $(IMGUI)/backends/imgui_impl_glfw.cpp \
  $(IMGUI)/backends/imgui_impl_vulkan.cpp

IMGUI_OBJS := $(IMGUI_SRCS:.cpp=.o)

SHADER_DIR := shaders
VERT_SRC := $(SHADER_DIR)/triangle.vert
FRAG_SRC := $(SHADER_DIR)/triangle.frag
VERT_SPV := $(SHADER_DIR)/vert.spv
FRAG_SPV := $(SHADER_DIR)/frag.spv
SHADERS := $(VERT_SPV) $(FRAG_SPV)

BIN := VulkanTest

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS) $(IMGUI_OBJS) $(SHADERS)
	$(CXX) -o $@ $(OBJS) $(IMGUI_OBJS) $(LDFLAGS) $(GLFW_LIB) $(LDLIBS)

# $(BIN): $(OBJS) $(IMGUI_OBJS) $(SHADERS)
# 	$(CXX) -o $@ $(OBJS) $(IMGUI_OBJS) $(LDFLAGS) $(LDLIBS)

third_party/imgui/%.o: third_party/imgui/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(VERT_SPV): $(VERT_SRC)
	glslc $< -o $@

$(FRAG_SPV): $(FRAG_SRC)
	glslc $< -o $@

clean:
	rm -rf $(BIN) $(OBJS) $(IMGUI_OBJS) $(SHADERS)

