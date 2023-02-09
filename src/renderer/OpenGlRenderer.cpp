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

    double InputHandler::m_MousePosX{ 0.0f };

    double InputHandler::m_MousePosY{ 0.0f };

    double InputHandler::m_LastMousePositionX{ 0.0f };

    double InputHandler::m_LastMousePositionY{ 0.0f };

    int InputHandler::m_MouseStates[NUM_MOUSE_BUTTON];

    bool InputHandler::IsMousePressed(const int buttonId)
    {
        assert(buttonId < NUM_MOUSE_BUTTON);
        return InputHandler::m_MouseStates[buttonId];
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
            layout(location = 1) in vec4 normal;
            out vec4 v_Position;
            out vec4 v_Normal;

            void main()
            {
                gl_Position = u_MVP * position;
                v_Position = position;
                v_Normal = normal;
            }
         )glsl";

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);

        const char* fragmentSource = R"glsl(
            #version 330 core

            uniform vec4 u_ViewDir;
            in vec4 v_Position;
            in vec4 v_Normal;
            layout(location = 0) out vec4 outColor;

            void main()
            {
                float radiance = dot(-u_ViewDir, v_Normal);
                outColor = vec4(radiance, radiance, radiance, 1);
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
        m_AspectRatio = m_ViewportWidth * 1.0f / m_ViewportHeight;

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

        std::vector<shared_ptr<Object>> allObjects;
        scene->m_Root->GetAllObjects(allObjects);

        unsigned int vertexShader, fragmentShader = 0;
        m_ShaderProgram = CompilerLinkShader(vertexShader, fragmentShader);
        // push the mesh into the vertex buffer
        for (int i = 0; i < allObjects.size(); i++)
        {
            auto& object = allObjects[i];
            if (std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(object))
            {
                m_MeshDraws.emplace_back(mesh, m_ShaderProgram);
            }
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
    }

    void OpenGlRenderer::Render()
    {
        const Matrix4x4 view = m_Scene->m_Camera.GetViewMatrix();
        const Matrix4x4 prospective = Matrix4x4::CreatePerspectiveProjectMatrix(m_Scene->m_Camera.m_FOV, m_Scene->m_Camera.m_Near, m_Scene->m_Camera.m_Far, m_ViewportWidth * 1.0f / m_ViewportHeight);

        // World matrix is applied to the positions already
        Matrix4x4 mvp = prospective * view;

        // Convert it to column based as OpenGl uses column base matrix representation
        Matrix4x4 mvpTransposed = mvp.Transposed();
        //
        // glBindFragDataLocation(m_ShaderProgram, 0, "outColor");
        unsigned int mvpLocation = glGetUniformLocation(m_ShaderProgram, "u_MVP");
        glUniformMatrix4fv(mvpLocation, 1, false, &mvpTransposed.cols[0][0]);

        Vector3 viewDir = m_Scene->m_Camera.GetLookAt() - m_Scene->m_Camera.GetLocation();
        viewDir.Normalize();
        unsigned int viewDirLocation = glGetUniformLocation(m_ShaderProgram, "u_ViewDir");
        glUniform4f(viewDirLocation, viewDir.x, viewDir.y, viewDir.z, 0.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        for (int i = 0; i < m_MeshDraws.size(); i++)
        {
            m_MeshDraws[i].Draw();
        }

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
            ImGui::Begin("Debug Menu");
            ImGui::Text("Camera");
            ImGui::SliderFloat("Camera FOV", &m_Scene->m_Camera.m_FOV, 30.0f, 160.f);

            const Matrix4x4 view = m_Scene->m_Camera.GetViewMatrix();
            const Matrix4x4 proj = Matrix4x4::CreatePerspectiveProjectMatrix(m_Scene->m_Camera.m_FOV, m_Scene->m_Camera.m_Near, m_Scene->m_Camera.m_Far, m_ViewportWidth / m_ViewportHeight);

            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();
        }
    }

    bool OpenGlRenderer::IsWindowCloased()
    {
        return glfwWindowShouldClose(m_Window);
    }

    void OpenGlRenderer::Update()
    {
        glfwGetCursorPos(m_Window, &InputHandler::m_MousePosX, &InputHandler::m_MousePosY);
        InputHandler::m_MouseStates[GLFW_MOUSE_BUTTON_LEFT] = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT);
        InputHandler::m_MouseStates[GLFW_MOUSE_BUTTON_MIDDLE] = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_MIDDLE);
        InputHandler::m_MouseStates[GLFW_MOUSE_BUTTON_RIGHT] = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT);

        if (InputHandler::IsMousePressed(0) == GLFW_PRESS)
        {
            double deltaX = InputHandler::m_MousePosX - InputHandler::m_LastMousePositionX;
            double deltaY = InputHandler::m_MousePosY - InputHandler::m_LastMousePositionY;

            InputHandler::m_LastMousePositionX = InputHandler::m_MousePosX;
            InputHandler::m_LastMousePositionY = InputHandler::m_MousePosY;

            Camera& camera = m_Scene->m_Camera;

            camera.m_AngleH += deltaX / 10.0f;
            camera.m_AngleV += deltaY / 10.0f;
            camera.m_AngleV = Clamp(camera.m_AngleV, -90.0f, 90.0f);

            // Update the camera view from the angle
            Vector3 forward{ 0.0f, 0.0f, 1.0f };
            forward.Rotate(ToRadian(camera.m_AngleH), Vector3{ 0.0f, 1.0f, 0.0f });
            forward.Normalize();
            
            Vector3 right = Vector3{ 0.0f, 1.0f, 0.0f }.Cross(forward);
            right.Normalize();
            forward.Rotate(ToRadian(camera.m_AngleV), right);

            forward.Normalize();
            Vector3 up = forward.Cross(right);
            up.Normalize();

            camera.m_Right = right;
            camera.m_Up = up;
            camera.m_Forward = forward;
        }

        if (InputHandler::IsMousePressed(2) == GLFW_PRESS)
        {
            float deltaX = InputHandler::m_MousePosX - InputHandler::m_LastMousePositionX;
            float deltaY = InputHandler::m_MousePosY - InputHandler::m_LastMousePositionY;
            
        }

        if (InputHandler::IsMousePressed(0) == GLFW_RELEASE)
        {
            InputHandler::m_LastMousePositionX = InputHandler::m_MousePosX;
            InputHandler::m_LastMousePositionY = InputHandler::m_MousePosY;
        }
    }
}
