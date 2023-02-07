
#include "math/Matrix.h"
#include "utility/Log.h"
#include "test/TestCase.h"
#include "renderer/OpenGlRenderer.h"

int main(int argc, char *argv[])
{
    using namespace new_type_renderer;

    auto scene = make_shared<Scene>();
    scene->LoadFromJson("scene/box.json");
    OpenGlRenderer viewport;
    viewport.Init();
    viewport.LoadScene(scene);

    assert(viewport.IsInitialized());
    while (viewport.IsWindowCloased() == false)
    {
        viewport.Render();
    }

    return 0;
}