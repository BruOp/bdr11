#pragma once
#include "pch.h"

#include "Window.h"
#include "StepTimer.h"
#include "Scene.h"
#include "Renderer.h"
#include "Camera.h"
#include "View.h"
#include "RenderPass.h"

namespace bdr
{
    struct AppConfig
    {
        uint32_t width;
        uint32_t height;
    };

    // A basic game implementation that creates a D3D11 device and
    // provides a game loop.
    class BaseGame : public DX::IDeviceNotify
    {
    public:

        BaseGame() noexcept(false);

        int run();
        virtual void shutdown();

        // Initialization and management
        void initialize(AppConfig& appConfig);
        virtual void setup() = 0;

        // Basic game loop
        virtual void tick() = 0;

        // IDeviceNotify
        virtual void OnDeviceLost() override = 0;
        virtual void OnDeviceRestored() override = 0;

        // Messages
        virtual void onActivated();
        virtual void onDeactivated();
        virtual void onSuspending();
        virtual void onResuming();
        void processMessage(UINT message, WPARAM wParam, LPARAM lParam);

        BDRid createScene();
        inline Scene& getScene(BDRid sceneId)
        {
            sceneList[sceneId];
        };

        AppConfig appConfig;
    protected:

        virtual void update(DX::StepTimer const& timer);
        virtual void render();

        // Rendering loop timer.
        DX::StepTimer m_timer;
        std::unique_ptr<DirectX::Keyboard> m_keyboard;
        std::unique_ptr<DirectX::Mouse> m_mouse;

        std::vector<Scene> sceneList;
        Renderer renderer;
        RenderGraph renderGraph;
        Window window;
    };
}

// Exit helper
void ExitGame();