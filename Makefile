CXX := g++
CXXFLAGS := -std=c++17 -O2 -Iinclude -Ithird_party/imgui -Ithird_party/imgui/backends
LDLIBS := $(shell pkg-config --libs glfw3) -lvulkan -ldl -lpthread
# If needed on your setup, append: -lX11 -lXxf86vm -lXrandr -lXi

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
	$(CXX) -o $@ $(OBJS) $(IMGUI_OBJS) $(LDLIBS)

# compile any .cpp in src/
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# compile ImGui sources
third_party/imgui/%.o: third_party/imgui/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(VERT_SPV): $(VERT_SRC)
	glslc $< -o $@

$(FRAG_SPV): $(FRAG_SRC)
	glslc $< -o $@

clean:
	rm -rf $(BIN) $(OBJS) $(IMGUI_OBJS) $(SHADERS)

# CXX := g++
# CXXFLAGS := -std=c++17 -O2 -Ithird_party/imgui -Ithird_party/imgui/backends
# LDLIBS := $(shell pkg-config --libs glfw3) -lvulkan -ldl -lpthread
# # LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
#
# SHADER_DIR := shaders
# VERT_SRC := $(SHADER_DIR)/triangle.vert
# FRAG_SRC := $(SHADER_DIR)/triangle.frag
# VERT_SPV := $(SHADER_DIR)/vert.spv
# FRAG_SPV := $(SHADER_DIR)/frag.spv
# SHADERS := $(VERT_SPV) $(FRAG_SPV)
#
# IMGUI := third_party/imgui
# IMGUI_SRCS := \
# 	$(IMGUI)/imgui.cpp \
# 	$(IMGUI)/imgui_draw.cpp \
# 	$(IMGUI)/imgui_widgets.cpp \
# 	$(IMGUI)/imgui_tables.cpp \
# 	$(IMGUI)/backends/imgui_impl_glfw.cpp \
# 	$(IMGUI)/backends/imgui_impl_vulkan.cpp
#
# OBJS:= main.o $(IMGUI_SRCS:.cpp=.o)
#
# # VulkanTest: main.cpp
# # 	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)
# #
# VulkanTest: $(SHADERS) $(OBJS)
# 	$(CXX) -o $@ $(OBJS) $(LDLIBS)
#
# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@
#
# $(VERT_SPV): $(VERT_SRC)
# 	glslc $< -o $@
#
# $(FRAG_SPV): $(FRAG_SRC)
# 	glslc $< -o $@
#
# .PHONY: clean
# clean:
# 	rm -rf VulkanTest $(OBJS) $(SHADERS)
#
#
