//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "Scene.h"
#include "GltfSceneLoader.h"
#include "AnimationSystem.h"

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_renderer.deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_renderer.setWindow(window, width, height);

    m_renderer.createDeviceResources();
    CreateDeviceDependentResources();

    m_renderer.createWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]() {
        Update(m_timer);
        });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    const auto& kb = m_keyboard->GetState();
    if (kb.Escape) {
        ExitGame();
    }

    float frameTime = float(timer.GetElapsedSeconds());
    float totalTime = float(timer.GetTotalSeconds());

    m_cameraController.update(kb, m_mouse->GetState(), frameTime);

    bdr::ECSRegistry& registry = m_scene.registry;

    if (m_scene.animations.size() > 0 && m_scene.animations[0].playingState != bdr::Animation::State::Playing) {
        m_scene.animations[0].playingState = bdr::Animation::State::Playing;
        m_scene.animations[0].startTime = totalTime;
    }

    for (bdr::Animation& animation : m_scene.animations) {
        updateAnimation(registry, animation, totalTime);
    }
    updateMatrices(registry);
    copyDrawData(registry);

    m_mouse->ResetScrollWheelValue();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0) {
        return;
    }

    Clear();

    m_renderer.deviceResources->PIXBeginEvent(L"Render");
    auto context = m_renderer.deviceResources->GetD3DDeviceContext();

    // TODO: Remove this and put it inside the passes.
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    context->RSSetState(m_rasterState.Get());

    m_renderGraph.run(&m_renderer);

    m_renderer.deviceResources->PIXEndEvent();

    // Show the new frame.
    m_renderer.deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_renderer.deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_renderer.deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_renderer.deviceResources->GetRenderTargetView();
    auto depthStencil = m_renderer.deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_renderer.deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_renderer.deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto r = m_renderer.getOutputSize();
    m_renderer.hasWindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_renderer.hasWindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1600;
    height = 900;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    ID3D11Device1* device = m_renderer.getDevice();

    CD3D11_RASTERIZER_DESC rastDesc{
        D3D11_FILL_SOLID, D3D11_CULL_BACK, TRUE,
        D3D11_DEFAULT_DEPTH_BIAS,
        D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
        TRUE, FALSE, TRUE, FALSE,
    };
    DX::ThrowIfFailed(device->CreateRasterizerState(&rastDesc, m_rasterState.ReleaseAndGetAddressOf()));
    m_states = std::make_unique<CommonStates>(device);

    m_renderer.materials.initMaterial(m_renderer.getDevice(), L"basic_vs.cso", L"basic_ps.cso");

    std::vector<uint8_t> blob = DX::ReadData(L"skinning.cso");
    DX::ThrowIfFailed(m_renderer.getDevice()->CreateComputeShader(blob.data(), blob.size(), nullptr, m_renderer.computeShader.ReleaseAndGetAddressOf()));
    bdr::gltf::SceneData sceneData{ &m_scene, &m_renderer, "FlightHelmet/", "FlightHelmet.gltf" };
    bdr::gltf::loadModel(sceneData);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    float width = float(m_renderer.width);
    float height = float(m_renderer.height);
    m_camera.projection = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.0f, width / height, 0.1f, 100.f);

    m_cameraController.pitch = XM_PIDIV4;
    m_cameraController.yaw = XM_PIDIV4;
    m_cameraController.radius = 3.0f;
    m_cameraController.setCamera(&m_camera);

    bdr::View& baseView = m_renderGraph.createNewView();
    baseView.name = "Basic Mesh View";
    baseView.scene = &m_scene;
    baseView.setCamera(&m_camera);

    bdr::addBasicPass(m_renderGraph, &baseView);
    bdr::addSkinningPass(m_renderGraph, &baseView);
}

void Game::OnDeviceLost()
{
    m_rasterState.Reset();
    m_states.reset();
    m_scene.reset();
    m_renderer.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
