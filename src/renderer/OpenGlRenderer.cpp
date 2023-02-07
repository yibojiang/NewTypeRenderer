#include "OpenGlRenderer.h"
#include "utility/Log.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace new_type_renderer
{
    // GL error handling https://www.khronos.org/opengl/wiki/OpenGL_Error
    void GLAPIENTRY
        MessageCallback(GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar* message,
            const void* userParam)
    {
        fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);

        if (type == GL_DEBUG_TYPE_ERROR)
        {
            __debugbreak();
        }
    }

    OpenGlRenderer::~OpenGlRenderer()
    {
        glDeleteProgram(m_ShaderProgram);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwTerminate();
    }

    unsigned int OpenGlRenderer::CompilerLinkShader(unsigned int& vertexShader, unsigned int& fragmentShader)
    {
        // Compile shader
        const char* vertexSource = R"glsl(
            #version 330 core

            uniform mat4 u_MVP;
            layout(location = 0) in vec4 position;

            void main()
            {
                gl_Position = u_MVP * position;
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
                outColor = vec4(1, 1, 1, 1);
            }

         )glsl";

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);

        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);

        glValidateProgram(shaderProgram);

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

        // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
        const char* glslVersion = "#version 100";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
        const char* glslVersion = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
        const char* glslVersion = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

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

        // During init, enable debug output
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);

        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
        ImGui_ImplOpenGL3_Init(glslVersion);
        ImGui::StyleColorsDark();

        m_Initialized = true;
    }

    void OpenGlRenderer::LoadScene(shared_ptr<Scene>& scene)
    {
        m_Scene = scene;
        std::vector<float> positions;
        std::vector<unsigned int> indices;

        std::vector<shared_ptr<Object>> allObjects;
        scene->m_Root->GetAllObjects(allObjects);

        for (int i = 0; i < allObjects.size(); i++)
        {
            auto& object = allObjects[i];
            if (std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(object))
            {
                LOG_INFO("previewing mesh %s", mesh->name.c_str());

                for (int j = 0; j < mesh->m_Positions.size(); j++)
                {
                    positions.push_back(mesh->m_Positions[j].x);
                    positions.push_back(mesh->m_Positions[j].y);
                    positions.push_back(mesh->m_Positions[j].z);
                }

                for (int j = 0; j < mesh->m_Indices.size(); j++)
                {
                    indices.push_back(mesh->m_Indices[j]);
                    m_IndicesCount++;
                }
            }
        }

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);

        unsigned int vertexShader, fragmentShader = 0;
        m_ShaderProgram = CompilerLinkShader(vertexShader, fragmentShader);

        unsigned int ibo;
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // GLint posAttrib = glGetAttribLocation(m_ShaderProgram, "position");
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
        glEnableVertexAttribArray(0);

        
    }

    void OpenGlRenderer::Render()
    {
        const Matrix4x4 view = m_Scene->m_Camera.GetViewMatrix();
        const Matrix4x4 prospective = Matrix4x4::CreatePerspectiveProjectMatrix(m_Scene->m_Camera.m_FOV, m_Scene->m_Camera.m_Near, m_Scene->m_Camera.m_Far, m_ViewportWidth / m_ViewportHeight);

        // World matrix is applied to the positions already
        Matrix4x4 mvp = prospective * view;

        // Convert it to column based as OpenGl uses column base matrix representation
        Matrix4x4 mvpTransposed = mvp.Transposed();

        // glBindFragDataLocation(m_ShaderProgram, 0, "outColor");
        unsigned int location = glGetUniformLocation(m_ShaderProgram, "u_MVP");
        glUniformMatrix4fv(location, 1, false, &mvpTransposed.cols[0][0]);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // glDrawArrays(GL_TRIANGLES, 0, 3);
        auto meshCount = m_IndicesCount / 3;
        glDrawElements(GL_TRIANGLES, meshCount, GL_UNSIGNED_INT, nullptr);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        OnGUI();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(m_Window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    void OpenGlRenderer::OnGUI()
    {
        {
            ImGui::Begin("Debug Menu");                          // Create a window called "Hello, world!" and append into it.
            ImGui::Text("Camera");
            
            ImGui::SliderFloat3("Camera Location", &m_Scene->m_Camera.m_Location.x, -500.0f, 500.0f);
            ImGui::SliderFloat3("Camera Rotation", &m_Scene->m_Camera.m_Rotation.x, -500.0f, 500.0f);
            ImGui::SliderFloat("Camera FOV", &m_Scene->m_Camera.m_FOV, 30.0f, 160.f);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
    }

    bool OpenGlRenderer::IsWindowCloased()
    {
        return glfwWindowShouldClose(m_Window);
    }
}