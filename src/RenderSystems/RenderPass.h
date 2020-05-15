#pragma once
#include "pch.h"

#include <functional>

//#include "Core/Map.h"
#include "Core/Array.h"
#include "Graphics/Resources.h"
#include "./Resources.h"
#include "View.h"


namespace bdr
{
    class Scene;
    class Renderer;
    struct Camera;
    class ILight;

    class RenderObjectManager
    {
    public:
        RenderObjectHandle addRenderObject(const RenderPassHandle passId, RenderObject renderObject)
        {
            constexpr size_t maxAllowableListSize = UINT32_MAX >> 8;
            RenderObjectHandle renderObjectHandle{};
            if (renderObjects.size > maxAllowableListSize) {
                HALT("Cannot handle so many objects! Only %u allowed!", maxAllowableListSize);
            }
            renderObjectHandle.idx = (renderObjects.size << 8) | uint32_t(passId.idx);
            renderObjectHandle.pipelineId = renderObject.pipelineId;
            renderObjects.pushBack(renderObject);
            return renderObjectHandle;
        };

        RenderObject& getRenderObject(const RenderObjectHandle renderObjectHandle)
        {
            return renderObjects[renderObjectHandle.idx >> 8];
        };

        // TODO: Each pipeline has a separate list
        //SimpleMap<Array<RenderObject>> renderObjectsListByPipeline;

        // For now: one big list, rebind for each object
        // Also, while ResourceBinders are allocated from the heap and our implementation will
        // eventually need to free their slots as the associated object gets destroyed, this
        // Manager is only getting destroyed at the end of the program, so we don't have to clean
        // anything up (the Heap already has its own deletion functions)
        Array<RenderObject> renderObjects;
    };

    struct RenderPass
    {
        RenderPassHandle id = {};
        // TODO Targets/Outputs
        // TODO Transient Inputs
        std::function<void(Renderer* renderer)> setup;
        std::function<void(Renderer* renderer)> render;
        std::function<void(Renderer* renderer, const View& view)> renderView;
        std::function<void(Renderer* renderer)> tearDown;
        std::function<void()> destroy;
        std::wstring name = L"";
        std::vector<View*> views;
        RenderObjectManager renderObjectsManager;
    };

    // Note: not a real frame/render graph just yet
    class RenderSystem
    {
    public:
        RenderSystem() = default;
        ~RenderSystem();

        UNCOPIABLE(RenderSystem);
        UNMOVABLE(RenderSystem);

        void run(Renderer* renderer) const;
        void init(Renderer* renderer);

        RenderPassHandle createNewPass()
        {
            size_t idx = renderPasses.size();
            if (idx > UINT8_MAX) {
                HALT("Can't create that many passes!");
            }
            renderPasses.emplace_back();
            return { uint8_t(idx) };
        };

        RenderPass& getPass(const RenderPassHandle handle)
        {
            return renderPasses[handle.idx];
        }

        View& createNewView()
        {
            size_t idx = views.size();
            views.emplace_back();
            return views[idx];
        }

        Renderer* renderer;
    private:
        std::vector<View> views;
        std::vector<RenderPass> renderPasses;
    };

    RenderObjectHandle assignRenderObject(
        RenderSystem& renderSystem,
        const RenderObjectDesc renderObjectDesc
    );

    RenderObject& getRenderObject(
        RenderSystem& renderSystem,
        const RenderObjectHandle renderObjectHandle
    );

    void bindTexture(
        RenderSystem& renderSystem,
        RenderObjectHandle renderObjectId,
        const std::string& name,
        const TextureHandle textureHandle
    );

    //RenderPassHandle addSkinningPass(RenderSystem& renderSystem, View* view);
    RenderPassHandle addMeshPass(RenderSystem& renderSystem, View* view);
}

