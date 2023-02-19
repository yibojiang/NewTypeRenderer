#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "renderer/Renderer.h"

#include "MeshDraw.h"
#include "PathtracingRenderer.h"
#include "gui/InputManager.h"

namespace new_type_renderer
{
    using std::vector;

    class OpenGlRenderer : public Renderer
    {
    public:
        OpenGlRenderer() {}

        OpenGlRenderer(unsigned int viewportWidth, unsigned int viewportHeight) : m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight) {}

        ~OpenGlRenderer();

        void Init() override;

        void LoadScene(shared_ptr<Scene>& scene) override;

        void Render() override;

        void OnGUI();

        bool IsWindowCloased();

        bool IsInitialized() const { return m_Initialized; }

        void Update(const float elapsedTime) override;

        void OnKeyPressed(int key, int scancode, int action, int mods);

        static void OnKeyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    private:
        GLFWwindow* m_Window{ nullptr };

        bool m_Initialized{ false };

        unsigned int m_ViewportWidth{ 1280 };

        unsigned int m_ViewportHeight{ 800 };

        vector<shared_ptr<Object>> m_AllObjects;

        vector<MeshDraw> m_MeshDraws;

        float m_AspectRatio{ 1.0f };

        float m_CameraSpeedMultiplier{ 10.0f };

        int m_NumThread{ 8 };

        PathtracingRenderer m_PBRRenderer{ 1280, 800 };
    };
}

