#pragma once
#include "pch.h"

#include "./Core/Window.h"
#include "StepTimer.h"
#include "Game/Scene.h"
#include "Game/Camera.h"
#include "Graphics/Renderer.h"
#include "RenderSystems/View.h"
#include "RenderSystems/RenderPass.h"

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
        BaseGame() = default;

        int run();
        virtual void shutdown();

        // Initialization and management
        void initialize(AppConfig& appConfig);
        virtual void setup() = 0;

        // Basic game loop
        virtual void update();
        virtual void tick(const float frameTime, const float totalTime) = 0;
        virtual void render();
        virtual void clear();

        // IDeviceNotify
        virtual void OnDeviceLost() override;
        virtual void OnDeviceRestored() override;

        // Messages
        virtual void onActivated();
        virtual void onDeactivated();
        virtual void onSuspending();
        virtual void onResuming();
        void processMessage(UINT message, WPARAM wParam, LPARAM lParam);


        //virtual void render();

        // Rendering loop timer.
        AppConfig appConfig;
        DX::StepTimer timer;
        std::unique_ptr<DirectX::Keyboard> keyboard;
        std::unique_ptr<DirectX::Mouse> mouse;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterState;
        std::unique_ptr<DirectX::CommonStates> states = nullptr;

        ID3D11DepthStencilState* depthState;

        Scene scene;
        Renderer renderer;
        RenderSystem renderGraph;
        Window window = Window{};
    };
}

// Taken from BGFX
#ifndef ENTRY_IMPLEMENT_MAIN
#define ENTRY_IMPLEMENT_MAIN(_app)  \
int main()                      \
{                               \
		_app app{};             \
		return app.run();       \
};
#endif