
#include "math/Matrix.h"
#include "utility/Log.h"
#include "test/TestCase.h"
#include "renderer/OpenGlRenderer.h"

#include <ctime>
#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::duration_cast;

int main(int argc, char *argv[])
{
    using namespace new_type_renderer;

    auto scene = make_shared<Scene>();
    scene->LoadFromJson("scene/box.json");
    OpenGlRenderer viewport;
    viewport.Init();
    viewport.LoadScene(scene);

    float elapsedTime = 1.0f / 30.f;

    assert(viewport.IsInitialized());
    while (viewport.IsWindowCloased() == false)
    {
        high_resolution_clock::time_point start = high_resolution_clock::now();
        viewport.Update(elapsedTime);
        viewport.Render();
        
        high_resolution_clock::time_point end = high_resolution_clock::now();
        auto deltaTime = end - start;
        duration<float> timeSpan = duration_cast<duration<float>>(end - start);
        elapsedTime = timeSpan.count();
    }

    return 0;
}