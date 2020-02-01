#pragma once
#include "pch.h"

#include "DXHelpers.h"
#include "Game/View.h"


namespace bdr
{
    class Scene;
    class Renderer;
    struct Camera;
    class ILight;

    struct RenderPass
    {
        // TODO State
        // TODO Targets/Outputs
        // TODO Transient Inputs
        std::function<void(Renderer * renderer)> setup;
        std::function<void(Renderer * renderer)> render;
        std::function<void(Renderer * renderer, const View & view)> renderView;
        std::function<void(Renderer * renderer)> tearDown;
        std::wstring name = L"";
        std::vector<View*> views;

    };

    // Note: not a real frame/render graph just yet
    class RenderGraph
    {
    public:
        void run(Renderer* renderer) const;

        RenderPass& createNewPass()
        {
            size_t idx = renderPasses.size();
            renderPasses.emplace_back();
            return renderPasses[idx];
        };

        View& createNewView()
        {
            size_t idx = views.size();
            views.emplace_back();
            return views[idx];
        }

    private:
        std::vector<View> views;
        std::vector<RenderPass> renderPasses;
    };

    void addSkinningPass(RenderGraph& renderGraph, View* view);
    void addBasicPass(RenderGraph& renderGraph, View* view);
}

