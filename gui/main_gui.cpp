#include "../external/imgui/backends/imgui_impl_glfw.h"
#include "../external/imgui/backends/imgui_impl_opengl3.h"
#include "../external/imgui/imgui.h"
#include "GuiController.h"

#include <GLFW/glfw3.h>
#include <iostream>

static void glfw_error_callback(int error, const char *description) {
  std::cerr << "GLFW Error " << error << ": " << description << "\n";
}

#define STB_IMAGE_IMPLEMENTATION
#include "../external/IconsFontAwesome6.h"
#include "../external/stb_image.h"

// Helper to load texture
bool LoadTextureFromFile(const char *filename, GLuint *out_texture,
                         int *out_width, int *out_height) {
  int image_width = 0;
  int image_height = 0;
  unsigned char *image_data =
      stbi_load(filename, &image_width, &image_height, NULL, 4);
  if (image_data == NULL)
    return false;

  // Create a OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  GL_CLAMP_TO_EDGE); // This is required on WebGL for non
                                     // power-of-two textures
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

  // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);

  *out_texture = image_texture;
  *out_width = image_width;
  *out_height = image_height;

  return true;
}

int main() {
  // ... [GLFW Setup] ...
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "Log Analyzer Pro", nullptr, nullptr);
  if (window == nullptr)
    return 1;

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Setup ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  // Load Fonts (System Font)
  // macOS usually has fonts in /System/Library/Fonts/
  // Try to load nice fonts
  const float fontSize = 16.0f;
  ImFontConfig fontConfig;
  ImFont *font = nullptr;

  if (std::filesystem::exists("/System/Library/Fonts/SFNS.ttf")) {
    font = io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/SFNS.ttf",
                                        fontSize, &fontConfig);
  } else if (std::filesystem::exists("/System/Library/Fonts/Helvetica.ttc")) {
    font = io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/Helvetica.ttc",
                                        fontSize, &fontConfig);
  } else {
    font = io.Fonts->AddFontDefault();
  }

  // Merge Icons
  static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
  ImFontConfig icons_config;
  icons_config.MergeMode = true;
  icons_config.PixelSnapH = true;
  icons_config.GlyphMinAdvanceX = fontSize;

  const char *faPath = "external/fa-solid-900.ttf";
  if (!std::filesystem::exists(faPath)) {
    faPath = "../external/fa-solid-900.ttf";
  }

  if (std::filesystem::exists(faPath)) {
    io.Fonts->AddFontFromFileTTF(faPath, fontSize * 0.9f, &icons_config,
                                 icons_ranges);
  } else {
    std::cerr << "Warning: Could not find fa-solid-900.ttf in common locations."
              << std::endl;
  }

  // Setup style
  ImGui::StyleColorsDark();

  // ImGui_Impl ...
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Load Background Texture
  GLuint bgTexture = 0;
  int bgWidth = 0, bgHeight = 0;
  bool bgLoaded = LoadTextureFromFile("background_calm.png", &bgTexture,
                                      &bgWidth, &bgHeight);
  if (!bgLoaded) {
    // Fallback to previous
    bgLoaded =
        LoadTextureFromFile("background.png", &bgTexture, &bgWidth, &bgHeight);
  }

  loganalyzer::GuiController controller;

  while (!glfwWindowShouldClose(window) && !controller.shouldClose()) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render Background with Ken Burns Effect
    if (bgLoaded) {
      float t = (float)glfwGetTime();
      float zoom = 0.15f; // 15% zoom

      // Slow oscillating scale and pan
      float s = 1.0f / (1.0f + zoom * (0.5f + 0.5f * sinf(t * 0.05f)));
      float ox = (1.0f - s) * (0.5f + 0.5f * cosf(t * 0.07f));
      float oy = (1.0f - s) * (0.5f + 0.5f * sinf(t * 0.03f));

      ImGui::GetBackgroundDrawList()->AddImage(
          (void *)(intptr_t)bgTexture, ImVec2(0, 0), ImGui::GetIO().DisplaySize,
          ImVec2(ox, oy), ImVec2(ox + s, oy + s));
    }

    controller.render();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
