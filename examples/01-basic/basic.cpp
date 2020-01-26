#include "pch.h"
#include "entry.h"
#include "Mesh.h"

using namespace bdr;
using namespace DirectX::SimpleMath;

constexpr float cubePositions[] = {
    -0.5f,  0.5f, -0.5f, // +Y (top face)
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,  // -Y (bottom face)
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
};

constexpr uint32_t cubeColors[] = {
        0xff00ff00, // +Y (top face)
        0xff00ffff,
        0xffffffff,
        0xffffff00,
        0xffff0000, // -Y (bottom face)
        0xffff00ff,
        0xff0000ff,
        0xff000000,
};

constexpr uint16_t cubeIndices[] = {
    0, 1, 2,
    0, 2, 3,
    2, 1, 5,
    1, 6, 5,
    0, 3, 4,
    0, 4, 7,
    0, 7, 1,
    1, 7, 6,
    3, 2, 4,
    2, 5, 4,
    4, 5, 7,
    5, 6, 7
};

class BasicExample : public BaseGame
{
    virtual void setup()
    {
        AppConfig appConfig{
            1280,
            1024,
        };
        initialize(appConfig);

        sceneId = createScene();
        Scene& scene = getScene(sceneId);

        Entity entity = createEntity(scene);
        Transform transform{};
        //transform.rotation = Quaternion::FromAxisAngle();
        assignTransform(scene, entity, transform);

        constexpr size_t numVerts = _countof(cubePositions);
        constexpr size_t numIndices = _countof(cubeIndices);
        MeshCreationInfo meshCreationInfo;
        meshCreationInfo.indexData = (uint8_t*)cubeIndices;
        meshCreationInfo.indexFormat = BufferFormat::UINT16;
        addPositionAttribute(meshCreationInfo, cubePositions, BufferFormat::FLOAT_3);
        addColorAttribute(meshCreationInfo, cubeColors, BufferFormat::UINT32);

        BDRid meshHandle = createMesh(meshCreationInfo);
        assignMesh(scene, entity, meshHandle);

        BDRid materialId = createBasicMaterial();
        assignMaterial(scene, entity, materialId);

        // TODO: Remove reference to g_renderer;
        float width = float(g_renderer.width);
        float height = float(g_renderer.height);
        BDRid cameraId = createPerspectiveCamera(scene, XM_PI / 4.0f, width / height, 0.1f, 100.0f);
        Camera& camera = getCamera(scene, cameraId);

        cameraController.pitch = XM_PIDIV4;
        cameraController.yaw = XM_PIDIV4;
        cameraController.radius = 3.0f;
        cameraController.setCamera(&camera);

        View view{};
        view.name = "Mesh View";
        view.scene = &scene;
        view.setCamera(&camera);
        // Enable mesh pass
        addBasicPass(renderGraph, view);
    }

    virtual void tick(const float frameTime, const float totalTime)
    {
        Scene& scene = getScene(sceneId);
        // We want to be able to rotate the cube every tick
        Transform& transform = scene.registry.transforms[entity];
        // Will need to figure this out
        transform.rotation = Quaternion::CreateFromAxisAngle(sin(totalTime));

        update(scene);
        //prepare(scene);
        render();
    }

private:
    typedef uint32_t Entity;
    typedef uint32_t BDRid;

    Entity entity;
    BDRid sceneId = UINT32_MAX;

    OrbitCameraController cameraController;
};