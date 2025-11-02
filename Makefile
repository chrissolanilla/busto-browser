CXX := g++
CXXFLAGS := -std=c++17 -O2
LDLIBS := $(shell pkg-config --libs glfw3) -lvulkan
# LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SHADER_DIR := shaders
VERT_SRC := $(SHADER_DIR)/triangle.vert
FRAG_SRC := $(SHADER_DIR)/triangle.frag
VERT_SPV := $(SHADER_DIR)/vert.spv
FRAG_SPV := $(SHADER_DIR)/frag.spv
SHADERS := $(VERT_SPV) $(FRAG_SPV)

VulkanTest: main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	rm -rf VulkanTest


