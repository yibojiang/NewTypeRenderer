#include "OpenGlRenderer.h"
#include "utility/Log.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "basic/Shader.h"
#include "gui/InputManager.h"
#include "PathtracingRenderer.h"

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
        // fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        //     (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        //     type, severity, message);

        if (type == GL_DEBUG_TYPE_ERROR)
        {
            __debugbreak();
        }
    }

    void OpenGlRenderer::OnKeyPressed(int key, int scancode, int action, int mods)
    {
        
    }

    void OpenGlRenderer::OnKeyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        OpenGlRenderer* obj = static_cast<OpenGlRenderer*>(glfwGetWindowUserPointer(window));
        obj->OnKeyPressed(key, scancode, action, mods);
    }

    OpenGlRenderer::~OpenGlRenderer()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwTerminate();
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

        glfwSetWindowUserPointer(m_Window, this);

        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
        ImGui_ImplOpenGL3_Init(glslVersion);
        ImGui::StyleColorsDark();

        // Bind the input callback
        glfwSetKeyCallback(m_Window, OnKeyPressedCallback);

        m_Initialized = true;
    }

    void OpenGlRenderer::LoadScene(shared_ptr<Scene>& scene)
    {
        m_Scene = scene;

        std::vector<shared_ptr<Shape>> allObjects;
        scene->GetRootNode()->GetAllShapes(allObjects);

        // push the mesh into the vertex buffer
        for (int i = 0; i < allObjects.size(); i++)
        {
            auto& object = allObjects[i];
            if (std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(object))
            {
                m_MeshDraws.emplace_back(MeshDraw{ mesh, mesh->GetMaterial() });
            }
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);

        // Render the pbr render result into a texture
        glGenTextures(1, &m_PreviewTextureId);
        glBindTexture(GL_TEXTURE_2D, m_PreviewTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void OpenGlRenderer::Render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Camera& camera = m_Scene->GetCamera();
        const Matrix4x4 view = camera.GetViewMatrix();
        const Matrix4x4 prospective = Matrix4x4::CreatePerspectiveProjectMatrix(camera.m_FOV, camera.m_Near, camera.m_Far, m_ViewportWidth * 1.0f / m_ViewportHeight);
        // World matrix is applied to the positions already
        Matrix4x4 vp = prospective * view;
        Vector3 viewDir = camera.GetLookAt() - camera.GetLocation();
        viewDir.Normalize();

        for (int i = 0; i < m_MeshDraws.size(); i++)
        {
            m_MeshDraws[i].Draw(vp, viewDir);
        }

        // Bind the pbr result texture
        Image& image = m_PBRRenderer.GetImage();
        glBindTexture(GL_TEXTURE_2D, m_PreviewTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.GetWidth(), image.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetByteData());

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
            Camera& camera = m_Scene->GetCamera();
            ImGui::Begin("Debug Menu");
            ImGui::Text("Camera");
            ImGui::Text("Pos %s", camera.GetLocation().ToString().c_str());
            ImGui::Text("Fwd %s", camera.GetForward().ToString().c_str());
            ImGui::SliderFloat("FOV", &camera.m_FOV, 30.0f, 160.f);
            ImGui::SliderFloat("Speed", &m_CameraSpeedMultiplier, 1.0f, 20.f);
            ImGui::InputInt("Thread: ", &m_NumThreads, 1, 40);

            const Matrix4x4 view = camera.GetViewMatrix();
            const Matrix4x4 proj = Matrix4x4::CreatePerspectiveProjectMatrix(camera.m_FOV, camera.m_Near, camera.m_Far, m_ViewportWidth / m_ViewportHeight);

            for (auto& meshDraw : m_MeshDraws)
            {
                auto mesh = meshDraw.GetMesh();
                ImGui::Text("Mesh: %s", mesh->name.c_str());
                ImGui::Text("Tris: %d", mesh->m_Faces.size());
            }

            if (m_PBRRenderer.IsRendering())
            {
                ImGui::Text("Progress %.2f%%", m_PBRRenderer.GetProgress() * 100);
            }
            else
            {
                if (ImGui::Button("Render"))
                {
                    m_PBRRenderer.Init();
                    m_PBRRenderer.LoadScene(m_Scene);
                    m_PBRRenderer.SetNumThreads(m_NumThreads);
                    m_PBRRenderer.Render();
                }
            }

            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();

            ImGui::Begin("Result");
            Image& image = m_PBRRenderer.GetImage();
            const int width = image.GetWidth();
            const int height = image.GetHeight();
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            
            drawList->AddImage((void*)m_PreviewTextureId, ImVec2(0, 0), ImVec2(width, height), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));
            ImGui::End();
        }
    }

    bool OpenGlRenderer::IsWindowCloased()
    {
        return glfwWindowShouldClose(m_Window);
    }

    void OpenGlRenderer::Update(const float elapsedTime)
    {
        glfwGetCursorPos(m_Window, &InputManager::m_MousePosX, &InputManager::m_MousePosY);
        InputManager::m_MouseStates[GLFW_MOUSE_BUTTON_LEFT] = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT);
        InputManager::m_MouseStates[GLFW_MOUSE_BUTTON_MIDDLE] = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_MIDDLE);
        InputManager::m_MouseStates[GLFW_MOUSE_BUTTON_RIGHT] = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT);

        Camera& camera = m_Scene->GetCamera();
        if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
        {
            Vector3 camPos = camera.GetLocation();
            Vector3 fwd = camera.GetForward();
            camera.SetLocation(camPos + fwd * m_CameraSpeedMultiplier * elapsedTime);
        }

        if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
        {
            Vector3 camPos = camera.GetLocation();
            Vector3 fwd = camera.GetForward();
            camera.SetLocation(camPos - fwd * m_CameraSpeedMultiplier * elapsedTime);
        }

        if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
        {
            Vector3 camPos = camera.GetLocation();
            Vector3 right = camera.GetRight();
            camera.SetLocation(camPos - right * m_CameraSpeedMultiplier * elapsedTime);
        }

        if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
        {
            Vector3 camPos = camera.GetLocation();
            Vector3 right = camera.GetRight();
            camera.SetLocation(camPos + right * m_CameraSpeedMultiplier * elapsedTime);
        }

        if (glfwGetKey(m_Window, GLFW_KEY_E) == GLFW_PRESS)
        {
            Vector3 camPos = camera.GetLocation();
            Vector3 up = Vector3{ 0, 1, 0 };
            camera.SetLocation(camPos + up * m_CameraSpeedMultiplier * elapsedTime);
        }

        if (glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            Vector3 camPos = camera.GetLocation();
            Vector3 up = Vector3{ 0, 1, 0 };
            camera.SetLocation(camPos - up * m_CameraSpeedMultiplier * elapsedTime);
        }

        if (InputManager::IsMousePressed(1) == GLFW_PRESS)
        {
            double deltaX = InputManager::m_MousePosX - InputManager::m_LastMousePositionX;
            double deltaY = InputManager::m_MousePosY - InputManager::m_LastMousePositionY;

            InputManager::m_LastMousePositionX = InputManager::m_MousePosX;
            InputManager::m_LastMousePositionY = InputManager::m_MousePosY;

            const float mouseMoveSpeed = 0.1f;
            camera.m_AngleH += deltaX * mouseMoveSpeed;
            camera.m_AngleV += deltaY * mouseMoveSpeed;
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

        if (InputManager::IsMousePressed(2) == GLFW_PRESS)
        {
            float deltaX = InputManager::m_MousePosX - InputManager::m_LastMousePositionX;
            float deltaY = InputManager::m_MousePosY - InputManager::m_LastMousePositionY;
        }

        if (InputManager::IsMousePressed(1) == GLFW_RELEASE)
        {
            InputManager::m_LastMousePositionX = InputManager::m_MousePosX;
            InputManager::m_LastMousePositionY = InputManager::m_MousePosY;
        }
    }
}
