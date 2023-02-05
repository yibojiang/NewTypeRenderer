#pragma once
#include "Renderer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace new_type_renderer
{
    class OpenGlRenderer : public Renderer
    {
    public:
        OpenGlRenderer() {}

        OpenGlRenderer(uint16_t viewportWidth, uint16_t viewportHeight) : m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight) {}

        ~OpenGlRenderer();

        unsigned int CompilerLinkShader(unsigned int& vertexShader, unsigned int& fragmentShader);

        void Init() override;

        void Render() override;

        bool IsWindowCloased();

        bool IsInitialized() const { return m_Initialized; }

        void LoadScene(const Scene& scene) override;

    private:
        GLFWwindow* m_Window{ nullptr };

        bool m_Initialized{ false };

        uint16_t m_ViewportWidth{ 1280 };

        uint16_t m_ViewportHeight{ 800 };

        unsigned int m_ShaderProgram { 0 };
    };
}

