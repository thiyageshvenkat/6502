CXX = g++
CXXFLAGS = -std=c++17 \
	-Iexternal/imgui \
	-Iexternal/imgui/backends \
	-Iexternal/imgui_memory_editor \
	-Iexternal/ImGuiColorTextEdit \
	-Iexternal/imgui-filebrowser

LDFLAGS = -lglfw -lGL -ldl

SRC = EmulonUI.cpp \
	NESCpu.cpp \
	NESInstructions.cpp \
	NESAssembler.cpp \
	external/imgui/imgui.cpp \
	external/imgui/imgui_draw.cpp \
	external/imgui/imgui_tables.cpp \
	external/imgui/imgui_widgets.cpp \
	external/imgui/backends/imgui_impl_glfw.cpp \
	external/imgui/backends/imgui_impl_opengl3.cpp \
	external/ImGuiColorTextEdit/TextEditor.cpp

OUT = Emulon

all:
	$(CXX) $(SRC) $(CXXFLAGS) $(LDFLAGS) -o $(OUT)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)
