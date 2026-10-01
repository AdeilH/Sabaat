// import std;
//
// #include <allegro5/allegro.h>
// #include <allegro5/allegro_primitives.h>
// #include "imgui.h"
// #include "imgui_impl_allegro5.h"
#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

class App {
public:
    virtual ~App() = default;

    // Called once at startup after ImGui is initialized
    virtual void OnInit() = 0;

    // Called every frame to render the ImGui interface
    virtual void OnUpdate() = 0;

    // Called once at shutdown before ImGui is destroyed
    virtual void OnShutdown() = 0;
};

class MyUiApp : public App {
private:
    float m_color[3] = { 0.45f, 0.55f, 0.60f };
    int m_counter = 0;

public:
    void OnInit() override {
        // Setup fonts, styles, or load persistent settings
        ImGui::StyleColorsDark();
    }

    void OnUpdate() override {
        // Main ImGui Window
        ImGui::Begin("Backend Agnostic Window");

        ImGui::Text("This code runs identically on OpenGL, Vulkan, or DirectX!");
        ImGui::ColorEdit3("Background Color", m_color);

        if (ImGui::Button("Click Me")) {
            m_counter++;
        }
        ImGui::SameLine();
        ImGui::Text("Counter = %d", m_counter);

        ImGui::End();
    }

    void OnShutdown() override {
        // Clean up your own assets here
    }
};




int main() {
    // 1. Setup GLFW and OpenGL context
    if (!glfwInit()) return 1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui Host", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // 2. Setup Dear ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // 3. Setup Backend Bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // 4. Instantiate your agnostic app
    MyUiApp app;
    app.OnInit();

    // 5. Main Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the ImGui frame using the backends
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Call your backend-agnostic UI code
        app.OnUpdate();

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

    // 6. Cleanup
    app.OnShutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
