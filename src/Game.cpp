//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "Scene.h"
#include "GltfSceneLoader.h"

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
    m_renderer.deviceResources->SetWindow(window, width, height);

    m_renderer.deviceResources->CreateDeviceResources();
    m_renderer.inputLayoutManager.init(m_renderer.deviceResources->GetD3DDevice());
    CreateDeviceDependentResources();

    m_renderer.deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
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
    //float frameTime = float(timer.GetElapsedSeconds());
    float totalTime = float(timer.GetTotalSeconds());

    float radius = 3.0f;
    m_view = Matrix::CreateLookAt(Vector3{ radius * sinf(0.5f * totalTime), 0.5f, radius * cosf(0.5f * totalTime) }, Vector3::Zero, Vector3::UnitY);
    /*for (const auto& animation : m_scene.animations) {
        bdr::updateAnimation(m_scene.nodeList, animation, totalTime);
    }*/
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

    // TODO: Add your rendering code here.
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    context->RSSetState(m_rasterState.Get());

    //renderPasses.render(context, m_scene, m_view, m_proj);

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
    auto r = m_renderer.deviceResources->GetOutputSize();
    m_renderer.deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_renderer.deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
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

    //renderPasses.init(device);

    bdr::gltf::SceneData sceneData{ &m_scene, &m_renderer, "polly/", "project_polly.gltf" };
    bdr::gltf::loadModel(sceneData);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    m_view = Matrix::CreateLookAt(Vector3{ 2.0f, 2.0f, 2.0f }, Vector3::Zero, Vector3::UnitY);
    RECT viewPort = m_renderer.deviceResources->GetOutputSize();
    float width = float(viewPort.right - viewPort.left);
    float height = float(viewPort.bottom - viewPort.top);
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.0f, width / height, 0.1f, 10.f);
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
