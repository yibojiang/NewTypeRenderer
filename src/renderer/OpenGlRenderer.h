#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Renderer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "MeshDraw.h"

#define NUM_MOUSE_BUTTON 8

namespace new_type_renderer
{
    class InputHandler
    {
    public:
        static bool IsMousePressed(const int buttonId);

    public:
        static double m_MousePosX;

        static double m_MousePosY;

        static double m_LastMousePositionX;

        static double m_LastMousePositionY;

        static int m_MouseStates[NUM_MOUSE_BUTTON];
    };

    class OpenGlRenderer : public Renderer
    {
    public:
        OpenGlRenderer() {}

        OpenGlRenderer(unsigned int viewportWidth, unsigned int viewportHeight) : m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight) {}

        ~OpenGlRenderer();

        unsigned int CompilerLinkShader(unsigned int& vertexShader, unsigned int& fragmentShader);

        void Init() override;

        void LoadScene(shared_ptr<Scene>& scene) override;

        void Render() override;

        void OnGUI();

        bool IsWindowCloased();

        bool IsInitialized() const { return m_Initialized; }

        void Update() override;

    private:
        GLFWwindow* m_Window{ nullptr };

        bool m_Initialized{ false };

        unsigned int m_ViewportWidth{ 1280 };

        unsigned int m_ViewportHeight{ 800 };

        unsigned int m_ShaderProgram{ 0 };

        std::vector<shared_ptr<Object>> m_AllObjects;

        float m_AspectRatio{ 0.0f };

        std::vector<MeshDraw> m_MeshDraws;

        Vector3 m_LastCameraLocation{};
    };
}

