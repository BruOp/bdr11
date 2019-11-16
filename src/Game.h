//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Scene.h"


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
    void GetDefaultSize( int& width, int& height ) const;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState;
    std::unique_ptr<DirectX::CommonStates> m_states = nullptr;
    std::unique_ptr<DirectX::DebugEffect> m_effect = nullptr;
    bdr::Scene m_scene{};

    DirectX::SimpleMath::Matrix m_world = DirectX::SimpleMath::Matrix::Identity;
    DirectX::SimpleMath::Matrix m_view = DirectX::SimpleMath::Matrix::Identity;
    DirectX::SimpleMath::Matrix m_proj = DirectX::SimpleMath::Matrix::Identity;
};
