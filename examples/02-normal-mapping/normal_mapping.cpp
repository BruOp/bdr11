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

uint32_t createLayout(Renderer& renderer, const ResourceBindingLayoutDesc& layoutDesc)
{
    const auto layoutId = renderer.layouts.size();
    renderer.layouts.emplace_back();
    ResourceBindingLayout& layout = renderer.layouts[layoutId];

    layout.usage = layoutDesc.usage;

    for (const auto& resourceDesc : layoutDesc.resourceDescs) {
        if (resourceDesc.type == BoundResourceType::CONSTANT_BUFFER) {
            // TODO
            HALT("This resource type is not supported!");
        }
        else if (resourceDesc.type == BoundResourceType::READABLE_BUFFER) {
            layout.resourceMap.insert(resourceDesc.name, { resourceDesc.type, layout.readableBufferCount++ });
        }
        else if (resourceDesc.type == BoundResourceType::WRITABLE_BUFFER) {
            layout.resourceMap.insert(resourceDesc.name, { resourceDesc.type, layout.writableBufferCount++ });
        }
        else if (resourceDesc.type == BoundResourceType::SAMPLER) {
            layout.resourceMap.insert(resourceDesc.name, { resourceDesc.type, layout.samplerCount++ });
        }
        else if (resourceDesc.type == BoundResourceType::INVALID) {
            break;
        }
    }
    return layoutId;
};

void bindTexture(
    Renderer& renderer,
    const MaterialInstance materialInstance,
    const std::string& name,
    const uint32_t textureHandle)
{
    ResourceBinder binder = renderer.binders[materialInstance.resourceBinderId];
    ResourceBindingHeap& heap = renderer.bindingHeap;
    const ResourceBindingLayout& layout = renderer.layouts[binder.layoutId];
    const Texture& texture = renderer.textures[textureHandle];

    ResourceView resourceView;
    bool resourceFound = layout.resourceMap.get(name + "_map", &resourceView);
    ASSERT(resourceFound, "Unable to find associated map");
    auto srvOffset = binder.readableBufferOffset + resourceView.offset;


    resourceFound = layout.resourceMap.get(name + "_sampler", &resourceView);
    ASSERT(resourceFound, "Unable to find associated map");
    auto samplerOffset = binder.samplerOffset + resourceView.offset;
    heap.srvs[srvOffset] = texture.srv;
    heap.samplers[samplerOffset] = texture.sampler;
}

uint32_t allocateResourceBinder(Renderer& renderer, const uint32_t& layoutId)
{
    const ResourceBindingLayout& layout = renderer.layouts[layoutId];
    ResourceBindingHeap& heap = renderer.bindingHeap;
    ResourceBinder binder{ uint16_t(layoutId) };
    binder.readableBufferOffset = heap.srvs.size();
    heap.srvs.resize(binder.readableBufferOffset + layout.readableBufferCount);

    binder.writableBufferOffset = heap.uavs.size();
    heap.uavs.resize(binder.writableBufferOffset + layout.writableBufferCount);

    binder.samplerOffset = heap.samplers.size();
    heap.samplers.resize(binder.samplerOffset + layout.samplerCount);
    auto id = renderer.binders.size();
    renderer.binders.push_back(binder);
    return id;
}

class NormalMappingExample : public bdr::BaseGame
{
    virtual void setup() override
    {
        AppConfig appConfig{
            1280,
            1024,
        };
        initialize(appConfig);

        std::string pipelineName = "normal_mapped";
        PipelineStateDefinition pipelineDefinition{
            pipelineName,
            "../examples/02-normal-mapping/normal_mapping.hlsl",
            PipelineStage(PipelineStage::VERTEX_STAGE | PipelineStage::PIXEL_STAGE),
            MeshAttribute(MeshAttribute::POSITION | MeshAttribute::NORMAL | MeshAttribute::TEXCOORD),
            { },
            { },
            {
                { "albedo_map", BoundResourceType::READABLE_BUFFER, PipelineStage::PIXEL_STAGE },
                { "normal_map", BoundResourceType::READABLE_BUFFER, PipelineStage::PIXEL_STAGE },
                { "normal_sampler", BoundResourceType::SAMPLER, PipelineStage::PIXEL_STAGE },
                { "albedo_sampler", BoundResourceType::SAMPLER, PipelineStage::PIXEL_STAGE },
            },
        };
        registerPipelineStateDefinition(renderer, std::move(pipelineDefinition));

        Array<ShaderMacro> shaderMacros{};
        uint32_t pipelineStateId = createPipelineState(renderer, pipelineName, shaderMacros);
        MaterialInstance materialInstance = createMaterialInstance(renderer, pipelineStateId);

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

        BDRid meshHandle = createMesh(renderer, meshCreationInfo);
        assignMesh(scene, entity, meshHandle);

        BDRid materialId = createCustomMaterial(renderer, , MeshAttribute::POSITION | MeshAttribute::NORMAL | MeshAttribute::TEXCOORD);
        assignMaterial(scene, entity, materialId);

        TextureCreationInfo texInfo{};
        texInfo.usage = BufferUsage::SHADER_READABLE;

        BDRid albedoTexture = createTextureFromFile(renderer, "Textures/stone01.DDS", texInfo);
        BDRid normalTexture = createTextureFromFile(renderer, "Textures/bump01.DDS", texInfo);

        assignMaterialInstance(scene, entity, materialInstance);
        bindTexture(renderer, materialInstance, "albedo", albedoTexture);
        bindTexture(renderer, materialInstance, "normal", normalTexture);

        float width = float(renderer.width);
        float height = float(renderer.height);
        BDRid cameraId = createPerspectiveCamera(scene, XM_PI / 4.0f, width, height, 0.1f, 100.0f);
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

ENTRY_IMPLEMENT_MAIN(NormalMappingExample);