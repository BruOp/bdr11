#pragma once
#include "pch.h"

#include <functional>
#include <algorithm>

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
            size_t numRenderLists = pipelineHandles.size();
            uint32_t renderAoAIdx = UINT32_MAX;
            for (size_t i = 0; i < numRenderLists; i++) {
                if (pipelineHandles[i].idx == renderObject.pipelineId.idx) {
                    ASSERT(i < maxAllowableListSize, "Cannot handle this many different pipelines being used in the same view");
                    renderAoAIdx = uint32_t(i);
                }
            }
            if (renderAoAIdx == UINT32_MAX) {
                // We need a new render objects list
                renderAoAIdx = pipelineHandles.size();
                pipelineHandles.push_back(renderObject.pipelineId);
                renderObjectsAoA.emplace_back();
                ASSERT(pipelineHandles.size() == renderObjectsAoA.size());
            }

            auto& renderObjectsList = renderObjectsAoA[renderAoAIdx];
            ASSERT(
                renderObjectsList.size() < UINT32_MAX,
                "Cannot handle so many render objects! Only %u allowed per pipeline!", UINT32_MAX
            );
            renderObjectHandle.renderAoAIdxPassId = (renderAoAIdx << 8) | uint32_t(passId.idx);

            ASSERT(renderAoAIdx < UINT32_MAX);
            renderObjectHandle.idx = uint32_t(renderObjectsList.size());
            renderObjectsList.push_back(renderObject);
            return renderObjectHandle;
        };

        RenderObject& getRenderObject(const RenderObjectHandle renderObjectHandle)
        {
            return renderObjectsAoA[getRenderAoAIdx(renderObjectHandle)][getRenderListIdx(renderObjectHandle)];
        };

        // For now: one big list, rebind for each object
        // Also, while ResourceBinders are allocated from the heap and our implementation will
        // eventually need to free their slots as the associated object gets destroyed, this
        // Manager is only getting destroyed at the end of the program, so we don't have to clean
        // anything up (the Heap already has its own deletion functions)

        std::vector<PipelineHandle> pipelineHandles;
        std::vector<std::vector<RenderObject>> renderObjectsAoA;
    };

    struct RenderPass
    {
        RenderPassHandle id = {};
        // TODO Targets/Outputs
        // TODO Transient Inputs
        std::function<void(Renderer * renderer)> setup;
        std::function<void(Renderer * renderer)> render;
        std::function<void(Renderer * renderer, const View & view)> renderView;
        std::function<void(Renderer * renderer)> tearDown;
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

    inline bool hasResources(const PipelineState& pipeline)
    {
        const ResourceBindingLayout& layout = pipeline.resourceLayout;
        return layout.readableBufferCount != 0 || layout.writableBufferCount != 0 || layout.samplerCount != 0;
    };
}

