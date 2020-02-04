#pragma once
#include "pch.h"

#include <functional>

#include "View.h"


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
        std::function<void()> destroy;
        std::wstring name = L"";
        std::vector<View*> views;

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

    void addSkinningPass(RenderSystem& renderGraph, View* view);
    void addBasicPass(RenderSystem& renderGraph, View* view);
}

