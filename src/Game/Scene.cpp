#include "pch.h"

#include "Scene.h"


namespace bdr
{
    Camera& getCamera(Scene& scene, const uint32_t cameraId)
    {
        return scene.cameras[cameraId];
    };

    uint32_t createPerspectiveCamera(Scene& scene, float fov, float width, float height, float _near, float _far)
    {
        Camera camera{ };
        camera.projection = math::perspective(fov, width, height, _near, _far);

        auto idx = scene.cameras.size();
        scene.cameras.push_back(std::move(camera));
        return idx;
    }
}
