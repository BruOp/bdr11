#include "entry.h"
#include "Core/bdrMath.h"
#include "Graphics/Mesh.h"
#include "Game/AnimationSystem.h"
#include "Utils/IcosahedronFactory.h"

using namespace bdr;
using namespace DirectX;

class NormalMappingExample : public bdr::BaseGame
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

        assignTransform(scene, entity, transform);

        auto icosahedronFactory = IcosahedronFactory(1u);

        MeshCreationInfo meshCreationInfo;
        meshCreationInfo.numVertices = icosahedronFactory.vertices.size();
        meshCreationInfo.numIndices = icosahedronFactory.indices.size();
        meshCreationInfo.indexData = (uint8_t*)icosahedronFactory.indices.data();
        meshCreationInfo.indexFormat = BufferFormat::UINT16;

        // Attributes can be added in any order
        addAttribute(meshCreationInfo, icosahedronFactory.vertices.data(), BufferFormat::FLOAT_3, MeshAttribute::POSITION);
        addAttribute(meshCreationInfo, icosahedronFactory.normals.data(), BufferFormat::FLOAT_3, MeshAttribute::NORMAL);
        addAttribute(meshCreationInfo, icosahedronFactory.uvs.data(), BufferFormat::FLOAT_2, MeshAttribute::TEXCOORD);

        BDRid meshHandle = createMesh(renderer, meshCreationInfo);
        assignMesh(scene, entity, meshHandle);

        BDRid materialId = createCustomMaterial(renderer, "../examples/02-normal-mapping/normal_mapping.hlsl", MeshAttribute::POSITION | MeshAttribute::NORMAL | MeshAttribute::TEXCOORD);
        assignMaterial(scene, entity, materialId);

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