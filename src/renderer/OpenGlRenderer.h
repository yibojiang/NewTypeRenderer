#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Renderer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace new_type_renderer
{
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

    private:
        GLFWwindow* m_Window{ nullptr };

        bool m_Initialized{ false };

        unsigned int m_ViewportWidth{ 1280 };

        unsigned int m_ViewportHeight{ 800 };

        unsigned int m_ShaderProgram{ 0 };

        int m_IndicesCount{ 0 };
    };
}

