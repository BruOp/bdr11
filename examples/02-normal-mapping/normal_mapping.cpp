#include "entry.h"
#include "Core/bdrMath.h"
#include "Core/Array.h"
#include "Graphics/Mesh.h"
#include "Game/AnimationSystem.h"
#include "Utils/IcosahedronFactory.h"

using namespace bdr;
using namespace DirectX;

constexpr float cubePositions[] = {
     0.5f,  0.5f,  0.5f, // +Y (Top face)
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f, // +X (Right face)
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f, // -X (Left face)
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f, // -Z (Back face)
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f,  0.5f, // +Z (Front face)
     0.5f,  0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f, // -Y (Bottom face)
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
};

constexpr float cubeNormals[] = {
     0.0f,  1.0f,  0.0f, // +Y (Top face)
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     1.0f,  0.0f,  0.0f, // +X (Right face)
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f, // -X (Left face)
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
     0.0f,  0.0f, -1.0f, // -Z (Back face)
     0.0f,  0.0f, -1.0f,
     0.0f,  0.0f, -1.0f,
     0.0f,  0.0f, -1.0f,
     0.0f,  0.0f,  1.0f, // +Z (Front face)
     0.0f,  0.0f,  1.0f,
     0.0f,  0.0f,  1.0f,
     0.0f,  0.0f,  1.0f,
     0.0f, -1.0f,  0.0f, // -Y (Bottom face)
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
};

constexpr float cubeUVs[] = {
    0.0f, 0.0f, // +Y (Top face)
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f, // +X (Right face)
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f, // -Y (Left face)
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f, // -Z (Back face)
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f, // +Z (Front face)
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f, // -Y (Bottom face)
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
};

constexpr uint16_t cubeIndices[] = {
     0,  2,  1, // Top
     3,  2,  0,
     4,  6,  5, // Right
     7,  6,  4,
     8, 10,  9, // Left
    11, 10,  8,
    12, 14, 13, // Back
    15, 14, 12,
    16, 18, 17, // Front
    19, 18, 16,
    20, 22, 21, // Bottom
    23, 22, 20,
};

class NormalMappingExample : public bdr::BaseGame
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

        std::string shaderFilePath = "../examples/02-normal-mapping/normal_mapping.hlsl";
        PipelineStateDefinition pipelineDefinition{
            PipelineStage(PipelineStage::VERTEX_STAGE | PipelineStage::PIXEL_STAGE),
            InputLayoutDesc{
                3u,
                { MeshAttribute::POSITION, MeshAttribute::NORMAL, MeshAttribute::TEXCOORD },
                { BufferFormat::FLOAT_3, BufferFormat::FLOAT_3,  BufferFormat::FLOAT_2 },
            },
            DepthStencilDesc{
                ComparisonFunc::GREATER,
                true,
                false
            },
            RasterStateDesc{ },
            BlendStateDesc{ },
            {
                BoundResourceDesc{ "albedo_map", BoundResourceType::READABLE_BUFFER, PipelineStage::PIXEL_STAGE },
                BoundResourceDesc{ "albedo_sampler", BoundResourceType::SAMPLER, PipelineStage::PIXEL_STAGE },
            },
            {
                { "NORMAL_MAPPING" }
            },
            {
                BoundResourceDesc{ "normal_map", BoundResourceType::READABLE_BUFFER, PipelineStage::PIXEL_STAGE },
                BoundResourceDesc{ "normal_sampler", BoundResourceType::SAMPLER, PipelineStage::PIXEL_STAGE },
            },
            {
                { "NORMAL_MAPPING", PipelineStateDefinition::BindingMapView{ 0, 2 } }
            },
        };
        const auto normalMappingPipelineDefId = registerPipelineStateDefinition(renderer, shaderFilePath, std::move(pipelineDefinition));

        const ShaderMacro shaderMacros[] = { {"NORMAL_MAPPING"} };
        PipelineHandle pipelineStateId = getOrCreatePipelineState(renderer, normalMappingPipelineDefId, shaderMacros, 1);

        entity = createEntity(scene);
        Transform transform{};

        assignTransform(scene, entity, transform);

        MeshCreationInfo meshCreationInfo;
        meshCreationInfo.numVertices = _countof(cubePositions) / 3u;
        meshCreationInfo.numIndices = _countof(cubeIndices);
        meshCreationInfo.indexData = (uint8_t*)cubeIndices;
        meshCreationInfo.indexFormat = BufferFormat::UINT16;

        // Attributes can be added in any order
        addAttribute(meshCreationInfo, cubePositions, BufferFormat::FLOAT_3, MeshAttribute::POSITION);
        addAttribute(meshCreationInfo, cubeNormals, BufferFormat::FLOAT_3, MeshAttribute::NORMAL);
        addAttribute(meshCreationInfo, cubeUVs, BufferFormat::FLOAT_2, MeshAttribute::TEXCOORD);

        MeshHandle meshHandle = createMesh(renderer, meshCreationInfo);

        TextureCreationInfo texInfo{};
        texInfo.usage = BufferUsage::SHADER_READABLE;

        RenderObjectDesc rod = {
                entity,
                passId,
                meshHandle,
                pipelineStateId
        };
        RenderObjectHandle renderObjectId = assignRenderObject(renderSystem, rod);

        TextureHandle albedoTexture = createTextureFromFile(renderer, "Textures/stone01.DDS", texInfo);
        TextureHandle normalTexture = createTextureFromFile(renderer, "Textures/bump01.DDS", texInfo);

        bindTexture(renderSystem, renderObjectId, "albedo", albedoTexture);
        bindTexture(renderSystem, renderObjectId, "normal", normalTexture);
    }

    virtual void tick(const float frameTime, const float totalTime) override
    {
        // We want to be able to rotate the cube every tick
        Transform& transform = scene.registry.transforms[entity];
        // Will need to figure this out

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

ENTRY_IMPLEMENT_MAIN(NormalMappingExample);
