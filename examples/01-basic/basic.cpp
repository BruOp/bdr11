#include "entry.h"
#include "Core/bdrMath.h"
#include "Graphics/Mesh.h"
#include "Game/AnimationSystem.h"

using namespace bdr;
using namespace DirectX;

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
    2, 1, 0,
    3, 2, 0,
    5, 1, 2,
    5, 6, 1,
    4, 3, 0,
    7, 4, 0,
    1, 7, 0,
    6, 7, 1,
    4, 2, 3,
    4, 5, 2,
    7, 5, 4,
    7, 6, 5
};

class BasicExample : public bdr::BaseGame
{
    virtual void setup() override
    {
        AppConfig appConfig{
            1280,
            1024,
        };
        initialize(appConfig);

        entity = createEntity(scene);
        Transform transform{};
        //transform.rotation = Quaternion::FromAxisAngle();
        assignTransform(scene, entity, transform);

        MeshCreationInfo meshCreationInfo;
        meshCreationInfo.numVertices = _countof(cubePositions);
        meshCreationInfo.numIndices = _countof(cubeIndices);
        meshCreationInfo.indexData = (uint8_t*)cubeIndices;
        meshCreationInfo.indexFormat = BufferFormat::UINT16;

        // Attributes can be added in any order
        addAttribute(meshCreationInfo, cubeColors, BufferFormat::UNORM8_4, MeshAttribute::COLOR);
        addAttribute(meshCreationInfo, cubePositions, BufferFormat::FLOAT_3, MeshAttribute::POSITION);

        BDRid meshHandle = createMesh(renderer, meshCreationInfo);
        assignMesh(scene, entity, meshHandle);

        BDRid materialId = getOrCreateBasicMaterial(renderer);
        assignMaterial(scene, entity, materialId);

        float width = float(renderer.width);
        float height = float(renderer.height);
        BDRid cameraId = createPerspectiveCamera(scene, XM_PI / 4.0f, width / height, 0.1f, 100.0f);
        Camera& camera = getCamera(scene, cameraId);

        cameraController.pitch = XM_PIDIV4;
        cameraController.yaw = XM_PIDIV4;
        cameraController.radius = 3.0f;
        cameraController.setCamera(&camera);

        bdr::View& view = renderGraph.createNewView();
        view.name = "Mesh View";
        view.scene = &scene;
        view.setCamera(&camera);
        // Enable mesh pass
        addBasicPass(renderGraph, &view);
        renderGraph.init(&renderer);
    }

    virtual void tick(const float frameTime, const float totalTime) override
    {
        // We want to be able to rotate the cube every tick
        Transform& transform = scene.registry.transforms[entity];
        // Will need to figure this out

        //transform.rotation = Quaternion::CreateFromYawPitchRoll(sinf(totalTime), 0.0f, 0.0f);
        cameraController.update(keyboard->GetState(), mouse->GetState(), frameTime);
        updateMatrices(scene.registry);
        copyDrawData(scene.registry);
        //prepare(scene);
    }

private:
    typedef uint32_t Entity;
    typedef uint32_t BDRid;

    Entity entity = UINT32_MAX;

    OrbitCameraController cameraController;
};

ENTRY_IMPLEMENT_MAIN(BasicExample);