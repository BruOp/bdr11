//
// Game.h
//

#pragma once

#include "StepTimer.h"
#include "Scene.h"
#include "Renderer.h"
#include "Camera.h"
#include "View.h"
#include "RenderPass.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize(int& width, int& height) const;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState;
    std::unique_ptr<DirectX::CommonStates> m_states = nullptr;
    bdr::Scene m_scene = {};
    bdr::Renderer m_renderer = {};

    bdr::Camera m_camera;
    bdr::RenderGraph m_renderGraph;
};
