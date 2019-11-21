//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
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

    float radius = 1.0f;
    m_view = Matrix::CreateLookAt(Vector3{ radius * sinf(0.5f * totalTime), 0.5f, radius * cosf(0.5f * totalTime) }, Vector3::Zero, Vector3::UnitY);
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

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    context->RSSetState(m_rasterState.Get());

    context->IASetInputLayout(m_scene.pInputLayout.Get());

    uint32_t offsets[4]{ 0u, 0u, 0u, 0u };
    for (const bdr::RenderObject& renderObject : m_scene.renderObjects) {
        if (renderObject.SceneNodeIdx != -1) {
            m_effect->SetMatrices(m_scene.nodeList.globalTransforms[renderObject.SceneNodeIdx], m_view, m_proj);
        }
        else {
            m_effect->SetMatrices(renderObject.modelTransform, m_view, m_proj);
        }
        m_effect->Apply(context);
        const bdr::Mesh& mesh = renderObject.mesh;
        context->IASetIndexBuffer(mesh.indexBuffer, mesh.indexFormat, 0);
        context->IASetVertexBuffers(0, mesh.numPresentAttributes, mesh.vertexBuffers.data(), mesh.strides, offsets);
        context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(mesh.indexCount, 0, 0);
    }

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
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
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
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
    ID3D11Device1* device = m_deviceResources->GetD3DDevice();

    CD3D11_RASTERIZER_DESC rastDesc{
        D3D11_FILL_SOLID, D3D11_CULL_BACK, TRUE,
        D3D11_DEFAULT_DEPTH_BIAS,
        D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
        TRUE, FALSE, TRUE, FALSE,
    };
    DX::ThrowIfFailed(device->CreateRasterizerState(&rastDesc, m_rasterState.ReleaseAndGetAddressOf()));
    m_states = std::make_unique<CommonStates>(device);

    m_effect = std::make_unique<DebugEffect>(device);
    m_effect->SetVertexColorEnabled(false);
    m_effect->SetMode(DebugEffect::Mode_Normals);

    void const* shaderByteCode;
    size_t byteCodeLength;

    m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
    
    bdr::SceneLoader sceneLoader{ m_deviceResources.get(), bdr::MaterialInfo{ shaderByteCode, byteCodeLength } };
    sceneLoader.loadGLTFModel(m_scene, "polly/", "project_polly.gltf");
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    m_view = Matrix::CreateLookAt(Vector3{ 2.0f, 2.0f, 2.0f }, Vector3::Zero, Vector3::UnitY);
    RECT viewPort = m_deviceResources->GetOutputSize();
    float width = float(viewPort.right - viewPort.left);
    float height = float(viewPort.bottom - viewPort.top);
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.0f, width / height, 0.1f, 10.f);

    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj);
}

void Game::OnDeviceLost()
{
    m_rasterState.Reset();
    m_states.reset();
    m_effect.reset();
    m_scene = bdr::Scene{};
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
