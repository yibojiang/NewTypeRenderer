
#include "math/Matrix.h"
#include "utility/Log.h"
#include "test/TestCase.h"
#include "renderer/OpenGlRenderer.h"

#include <ctime>
#include <chrono>

int main(int argc, char *argv[])
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using new_type_renderer::Scene;
    using new_type_renderer::OpenGlRenderer;
    using std::make_shared;
    if (argc == 0)
    {
        LOG_ERR("Scene is not specified");
    }
    const char* fileName = argv[1];

    auto scene = make_shared<Scene>();
    
    scene->LoadFromJson(fileName);
    OpenGlRenderer viewport;
    viewport.Init();
    viewport.LoadScene(scene);

    float elapsedTime = 1.0f / 30.f;

    assert(viewport.IsInitialized());
    while (viewport.IsWindowCloased() == false)
    {
        high_resolution_clock::time_point start = high_resolution_clock::now();
        scene->UpdateAllTransform();
        viewport.Update(elapsedTime);
        viewport.Render();
        
        high_resolution_clock::time_point end = high_resolution_clock::now();
        auto deltaTime = end - start;
        duration<float> timeSpan = duration_cast<duration<float>>(end - start);
        elapsedTime = timeSpan.count();
    }

    return 0;
}