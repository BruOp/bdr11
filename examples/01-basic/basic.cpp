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

        float width = float(renderer.width);
        float height = float(renderer.height);
        BDRid cameraId = createPerspectiveCamera(scene, XM_PI / 4.0f, width, height, 0.1f, 100.0f);
        Camera& camera = getCamera(scene, cameraId);

        cameraController.pitch = XM_PIDIV4;
        cameraController.yaw = XM_PIDIV4;
        cameraController.radius = 3.0f;
        cameraController.setCamera(&camera);

        bdr::View& view = renderSystem.createNewView();
        view.name = "Mesh View";
        view.scene = &scene;
        view.setCamera(&camera);
        // Enable mesh pass
        RenderPassHandle passId = addMeshPass(renderSystem, &view);
        renderSystem.init(&renderer);

        std::string shaderFilePath = "../examples/01-basic/basic_vertex_coloring.hlsl";
        PipelineStateDefinition pipelineDefinition{
            PipelineStage::VERTEX_PIXEL_STAGES,
            InputLayoutDesc{
                2u,
                { MeshAttribute::POSITION, MeshAttribute::COLOR },
                { BufferFormat::FLOAT_3, BufferFormat::UNORM8_4 },
             },
             // Depth, stencil and blend state are default initialized if left out.
             // We don't have any resources we need to bind, so we omit these as well.
        };
        const PipelineStateDefinitionHandle pipelineDefId = registerPipelineStateDefinition(
            renderer,
            shaderFilePath,
            std::move(pipelineDefinition)
        );
        PipelineHandle pipelineStateId = getOrCreatePipelineState(renderer, pipelineDefId, nullptr, 0);

        entity = createEntity(scene);
        Transform transform{};

        assignTransform(scene, entity, transform);

        MeshCreationInfo meshCreationInfo;
        meshCreationInfo.numVertices = _countof(cubePositions) / 3u;
        meshCreationInfo.numIndices = _countof(cubeIndices);
        meshCreationInfo.indexData = (uint8_t*)cubeIndices;
        meshCreationInfo.indexFormat = BufferFormat::UINT16;

        // Attributes can be added in any order
        addAttribute(meshCreationInfo, cubeColors, BufferFormat::UNORM8_4, MeshAttribute::COLOR);
        addAttribute(meshCreationInfo, cubePositions, BufferFormat::FLOAT_3, MeshAttribute::POSITION);

        MeshHandle meshHandle = createMesh(renderer, meshCreationInfo);

        RenderObjectDesc rod = {
            entity,
            passId,
            meshHandle,
            pipelineStateId,
        };
        RenderObjectHandle renderObjectId = assignRenderObject(renderSystem, rod);

    }

    virtual void tick(const float frameTime, const float totalTime) override
    {
        cameraController.update(keyboard->GetState(), mouse->GetState(), frameTime);
        updateMatrices(scene.registry);
        copyDrawData(scene.registry);
    }

private:
    typedef uint32_t Entity;
    typedef uint32_t BDRid;

    Entity entity = UINT32_MAX;

    OrbitCameraController cameraController;
};

ENTRY_IMPLEMENT_MAIN(BasicExample);