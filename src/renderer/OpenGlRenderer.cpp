#include "OpenGlRenderer.h"
#include "utility/Log.h"

namespace new_type_renderer
{
    OpenGlRenderer::~OpenGlRenderer()
    {
        glDeleteProgram(m_ShaderProgram);
        glfwTerminate();
    }

    unsigned int OpenGlRenderer::CompilerLinkShader(unsigned int& vertexShader, unsigned int& fragmentShader)
    {
        // Compile shader
        const char* vertexSource = R"glsl(
             #version 330 core

             layout(location = 0) in vec4 position;

             void main()
             {
                 gl_Position = position;
             }
         )glsl";

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);

        const char* fragmentSource = R"glsl(
             #version 330 core

             layout(location = 0) out vec4 outColor;

             void main()
             {
                 outColor = vec4(1.0, 0.0, 1.0, 1.0);
             }

         )glsl";

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);

        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);

        glValidateProgram(shaderProgram);
        glBindFragDataLocation(m_ShaderProgram, 0, "outColor");

        glLinkProgram(shaderProgram);
        glUseProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return shaderProgram;
    }

    void OpenGlRenderer::Init()
    {
        /* Initialize the library */
        if (!glfwInit())
        {
            return;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        /* Create a windowed mode m_Window and its OpenGL context */
        m_Window = glfwCreateWindow(m_ViewportWidth, m_ViewportHeight, "Viewport", NULL, NULL);

        if (!m_Window)
        {
            glfwTerminate();
            return;
        }

        /* Make the m_Window's context current */
        glfwMakeContextCurrent(m_Window);

        // Glew init has to be called when the context is valid
        if (glewInit() != GLEW_OK)
        {
            LOG_ERR("Glew inited failed !");
            return;
        }

        // create verts
        float vertices[]
        {
             -0.5f, -0.5f, // 0
              0.5f, -0.5f, // 1
              0.5f,  0.5f, // 2
             -0.5f,  0.5f  // 3
        };

        // index buffer data has to be unsingned
        unsigned int indices[]
        {
            0, 1, 2,
            2, 3, 0
        };

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), vertices, GL_STATIC_DRAW);

        unsigned int vertexShader, fragmentShader = 0;
        m_ShaderProgram = CompilerLinkShader(vertexShader, fragmentShader);

        unsigned int ibo;
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // GLint posAttrib = glGetAttribLocation(m_ShaderProgram, "position");
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
        glEnableVertexAttribArray(0);

        m_Initialized = true;
    }

    void OpenGlRenderer::Render()
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        /* Swap front and back buffers */
        glfwSwapBuffers(m_Window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    bool OpenGlRenderer::IsWindowCloased()
    {
        return glfwWindowShouldClose(m_Window);
    }

    void OpenGlRenderer::LoadScene(const Scene& scene)
    {
        
    }
}
