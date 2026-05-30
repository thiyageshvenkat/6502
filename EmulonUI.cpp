#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_memory_editor.h"
#include "NESCpu.h"
#include <GLFW/glfw3.h>
#include "NESAssembler.h"
#include "NESInstructions.h"
#include "imfilebrowser.h"
#include "TextEditor.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>

// shared UI/emulator state kept between frames
static CPU cpu; // CPU state shown in the UI
static MemoryEditor mem_edit; // memory viewer widget
static StepTrace lastTrace{}; // last instruction that ran
static bool hasTrace = false; // true after first step
static char asm_buffer[65536] = {}; // text buffer for asm editor
static float cpuPanelWidth = 250.0f; // remembered CPU panel size
static float editorPanelHeight = 260.0f; // remembered editor panel size
static TextEditor asm_editor; // code editor
static bool showSettings = false; // settings toggle
static ImGui::FileBrowser openAsmDialog;

static TextEditor::Language make6502Language() {
  TextEditor::Language lang;

  lang.name = "6502 Assembly";
  lang.caseSensitive = false;
  lang.singleLineComment = ";";

  lang.keywords = {
    "ADC", "AND", "ASL", "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL",
    "BRK", "BVC", "BVS", "CLC", "CLD", "CLI", "CLV", "CMP", "CPX", "CPY",
    "DEC", "DEX", "DEY", "EOR", "INC", "INX", "INY", "JMP", "JSR", "LDA",
    "LDX", "LDY", "LSR", "NOP", "ORA", "PHA", "PHP", "PLA", "PLP", "ROL",
    "ROR", "RTI", "RTS", "SBC", "SEC", "SED", "SEI", "STA", "STX", "STY",
    "TAX", "TAY", "TSX", "TXA", "TXS", "TYA"
  };

  return lang;
}



// helpers to process the program.asm file
static std::string readTextFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) return "";
    std::stringstream buffer;
    buffer << file.rdbuf(); // read whole file into stringstream
    return buffer.str();
}

static void writeTextFile(const std::string& path, const std::string& text) {
    std::ofstream file(path); // overwrite old file contents
    file << text; // write editor buffer to disk
}

static void loadProgramIntoCpu() {
    std::vector<uint8_t> program = assembleFile("program.asm"); // convert asm into bytes

    for (size_t i = 0; i < program.size(); i++) {
        cpu.memory[0x8000 + i] = program[i]; // load program at test start address
    }

    cpu.PC = 0x8000; // start executing from loaded program
}

int main() {
    // startup: create the window and connect ImGui/OpenGL
    if (!glfwInit()) { // if GLFW fails to start, exit with error code 1
        return 1;
    }
    const char* glsl_version = "#version 130"; // define shader

    // creates gl context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // initialize app window
    GLFWwindow* window = glfwCreateWindow(
        1000, // window height
        700, // window width
        "Emulon", // window title
        nullptr, // not fullscreen
        nullptr // no shared OpenGL context
    );

    if (!window) { // if window creation fails, shut down GLFW and return error code 1 
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window); // makes OpenGL context active
    glfwSwapInterval(1); // enable vsync to limit refresh rate to monitor refresh rate 

    IMGUI_CHECKVERSION(); // checks if the ImGui version in the headers match the compiled ImGui code
    ImGui::CreateContext(); // creates ImGui’s internal global state.

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.ChildBorderSize = 1.0f;
    // add custom font
    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->AddFontFromFileTTF(
        "/mnt/c/Windows/Fonts/consola.ttf",
        18.0f
    );

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true); // connects ImGui to GLFW
    ImGui_ImplOpenGL3_Init(glsl_version); // Connects ImGui to OpenGL 

    // emulator setup before the UI starts running
    initTable(); // fill opcode lookup table

    asm_editor.SetText(readTextFile("program.asm"));
    asm_editor.SetReadOnlyEnabled(false);
    asm_editor.SetShowLineNumbersEnabled(true);
    asm_editor.SetShowScrollbarMiniMapEnabled(true);
    asm_editor.SetShowWhitespacesEnabled(false);
    asm_editor.SetTabSize(2);
    asm_editor.SetPalette(TextEditor::GetDarkPalette());
    openAsmDialog.SetTitle("File Explorer");
    openAsmDialog.SetTypeFilters({ ".asm", ".*" });
    loadProgramIntoCpu(); // load first version of program before stepping

    // --- Main Loop --- //
    while (!glfwWindowShouldClose(window)) { // run until the user closes the window
      glfwPollEvents(); // process input/window events

      // start a fresh ImGui frame
      ImGui_ImplOpenGL3_NewFrame(); // new frame for OpenGL backend
      ImGui_ImplGlfw_NewFrame(); // new frame for GLFW backend
      ImGui::NewFrame(); // start a new frame

      const ImGuiViewport* viewport = ImGui::GetMainViewport(); // use full app viewport

      ImGui::SetNextWindowPos(viewport->WorkPos); // pin main window to viewport
      ImGui::SetNextWindowSize(viewport->WorkSize); // fill available window space

      // make the main ImGui window act like the app canvas
      ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | // custom full-screen style window
        ImGuiWindowFlags_NoResize | // viewport controls the size
        ImGuiWindowFlags_NoMove | // keep pinned to viewport
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

      ImGui::Begin("Emulon", nullptr, flags); // main app layout window

      
      // File Explorer

      if (ImGui::Button("File Explorer")) {
        openAsmDialog.Open();
      }

      openAsmDialog.Display();

      if (openAsmDialog.HasSelected()) {
        std::string path = openAsmDialog.GetSelected().string();
        std::string text = readTextFile(path);

        asm_editor.SetText(text);

        openAsmDialog.ClearSelected();
      }
      float mainHeight = ImGui::GetContentRegionAvail().y;
      float mainWidth = ImGui::GetContentRegionAvail().x;

      // left side: controls and CPU registers
      ImGui::BeginChild("CPU Panel", ImVec2(cpuPanelWidth, mainHeight), true); // left panel
      ImGui::TextDisabled("CPU");
      ImGui::Separator();
      if (ImGui::Button("Settings")) {
        showSettings = !showSettings;
      }
      if (ImGui::Button("Step")) {
        lastTrace = step(cpu); // run one CPU instruction
        hasTrace = true; // now trace info is valid

        int size = table[lastTrace.opcode].bytes;
        mem_edit.GotoAddrAndHighlight(lastTrace.pc, lastTrace.pc + size); // highlight bytes that just ran
      }

      // CPU register display
      ImGui::Text("A:  %02X", cpu.A);
      ImGui::Text("X:  %02X", cpu.X);
      ImGui::Text("Y:  %02X", cpu.Y);
      ImGui::Text("PC: %04X", cpu.PC);
      ImGui::Text("SP: %02X", cpu.SP);
      ImGui::Text("P:  %02X", cpu.P);

      ImGui::EndChild();

      ImGui::SameLine(); // put splitter beside CPU panel

      // vertical splitter between CPU panel and right side
      ImGui::Button("##cpu_splitter", ImVec2(8.0f, mainHeight)); // ## hides label but keeps unique ID
      if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW); // show horizontal resize cursor
      }
      if (ImGui::IsItemActive()) {
        cpuPanelWidth += ImGui::GetIO().MouseDelta.x; // move divider with mouse

        float minCPUWidth = 120.0f;
        float minRightWidth = 250.0f;
        float maxCPUWidth = mainWidth - minRightWidth; // leave room for right side

        if (maxCPUWidth < minCPUWidth) {
          maxCPUWidth = minCPUWidth; // avoid bad limits on tiny windows
        }

        // clamp CPU panel size
        if (cpuPanelWidth < minCPUWidth) cpuPanelWidth = minCPUWidth;
        if (cpuPanelWidth > maxCPUWidth) cpuPanelWidth = maxCPUWidth;
      }

      ImGui::SameLine();

      // right side: asm editor on top, memory viewer underneath
      ImGui::BeginChild("Right Side", ImVec2(0, mainHeight), false); // 0 width fills remaining space
      float rightHeight = ImGui::GetContentRegionAvail().y;
      float rightWidth = ImGui::GetContentRegionAvail().x;
      float splitterThickness = 8.0f;
      float minEditorHeight = 100.0f;
      float minMemoryHeight = 120.0f;
      float maxEditorHeight = rightHeight - splitterThickness - minMemoryHeight; // leave memory visible

      if (maxEditorHeight < minEditorHeight) maxEditorHeight = minEditorHeight;
      if (editorPanelHeight < minEditorHeight) editorPanelHeight = minEditorHeight;
      if (editorPanelHeight > maxEditorHeight) editorPanelHeight = maxEditorHeight;

      ImGui::BeginChild("Editor Panel", ImVec2(0, editorPanelHeight), true); // top right panel
      ImGui::TextDisabled("Code Editor");
      ImGui::Separator();
      if (ImGui::Button("Save")) {
        writeTextFile("program.asm", asm_editor.GetText());
      }

      ImGui::SameLine();

      if (ImGui::Button("Save + Assemble")) {
        writeTextFile("program.asm", asm_editor.GetText());
        loadProgramIntoCpu();
      }

      asm_editor.Render("##program_asm_editor", ImGui::GetContentRegionAvail(), true);

      ImGui::EndChild();

      ImGui::Button("##editor_splitter", ImVec2(rightWidth, splitterThickness)); // invisible-ish horizontal splitter

      if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS); // show vertical resize cursor
      }

      if (ImGui::IsItemActive()) {
        editorPanelHeight += ImGui::GetIO().MouseDelta.y; // move divider with mouse

        // clamp editor panel size
        if (editorPanelHeight < minEditorHeight) editorPanelHeight = minEditorHeight;
        if (editorPanelHeight > maxEditorHeight) editorPanelHeight = maxEditorHeight;
      }

      ImGui::BeginChild("Memory Panel", ImVec2(0, 0), true); // 0,0 fills the rest of right side
      ImGui::TextDisabled("Memory");
      ImGui::Separator();
      mem_edit.DrawContents(cpu.memory, 65536); // show full 64KB CPU memory
      ImGui::EndChild();

      ImGui::EndChild(); // end right side

      ImGui::End(); // end main Emulon window
      if (showSettings) {
        ImGui::Begin("Settings", &showSettings);

        ImGuiStyle& style = ImGui::GetStyle();
        ImGuiIO& io = ImGui::GetIO();

        ImGui::SliderFloat("UI font scale", &io.FontGlobalScale, 0.5f, 2.0f);
        ImGui::SliderFloat("Frame rounding", &style.FrameRounding, 0.0f, 12.0f);
        ImGui::SliderFloat("Window rounding", &style.WindowRounding, 0.0f, 12.0f);
        ImGui::SliderFloat("Item spacing X", &style.ItemSpacing.x, 0.0f, 20.0f);
        ImGui::SliderFloat("Item spacing Y", &style.ItemSpacing.y, 0.0f, 20.0f);

        if (ImGui::Button("Dark")) {
          ImGui::StyleColorsDark();
          asm_editor.SetPalette(TextEditor::GetDarkPalette());
        }

        ImGui::SameLine();

        if (ImGui::Button("Light")) {
          ImGui::StyleColorsLight();
          asm_editor.SetPalette(TextEditor::GetLightPalette());
        }

        ImGui::End();
      }

      ImGui::Render();
      // render the finished ImGui frame with OpenGL
      // sort framebuffer width and height
      int display_w;
      int display_h;
      glfwGetFramebufferSize(window, &display_w, &display_h); // get actual drawable size of window, since window size may not always equal framebuffer size

      // Instructions to OpenGL to draw the window
      glViewport(0, 0, display_w, display_h); // match OpenGL viewport to framebuffer
      glClearColor(0.08f, 0.08f, 0.09f, 1.0f); // background color
      glClear(GL_COLOR_BUFFER_BIT); // clear old frame

      // Draw ImGui UI using OpenGL
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window); // show back buffer
    }

    // shutdown: release ImGui, window, and GLFW resources
    ImGui_ImplOpenGL3_Shutdown(); // shut down OpenGL backend
    ImGui_ImplGlfw_Shutdown(); // shut down GLFW backend
    ImGui::DestroyContext(); // destory ImGui internal state

    glfwDestroyWindow(window); // destroy window
    glfwTerminate(); // fully shut down GLFWE

    return 0;
}
