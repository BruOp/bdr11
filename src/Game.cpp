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

void renderScene(bdr::Renderer& renderer, bdr::Scene& scene, bdr::View& view)
{
    const uint32_t offsets[6] = { 0, 0, 0, 0, 0, 0 };
    bdr::ECSRegistry& registry = scene.registry;

    ID3D11DeviceContext1* context = renderer.deviceResources->GetD3DDeviceContext();
    renderer.deviceResources->PIXBeginEvent(L"Skinning");

    for (size_t entityId = 0; entityId < registry.numEntities; ++entityId) {
        const uint32_t cmpMask = registry.cmpMasks[entityId];

        if (cmpMask & bdr::CmpMasks::SKIN) {
            // Compute joint matrices
            const bdr::Skin& skin = scene.skins[registry.skinIds[entityId]];
            bdr::Mesh& mesh = renderer.meshes[registry.meshes[entityId]];
            bdr::Mesh& preskin = renderer.meshes[mesh.preskinMeshIdx];
            bdr::JointBuffer& jointBuffer = renderer.jointBuffers[registry.jointBuffer[entityId]];
            std::vector<Matrix> jointMatrices(skin.inverseBindMatrices.size());
            const Matrix invModel{ registry.globalMatrices[entityId].Invert() };
            
            for (size_t joint = 0; joint < jointMatrices.size(); ++joint) {
                uint32_t jointEntity = skin.jointEntities[joint];
                jointMatrices[joint] = (skin.inverseBindMatrices[joint] * registry.globalMatrices[jointEntity] * invModel).Transpose();
            }

            ASSERT(jointBuffer.buffer != nullptr);
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            DX::ThrowIfFailed(context->Map(jointBuffer.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
            CopyMemory(mappedResource.pData, jointMatrices.data(), sizeof(Matrix) * jointMatrices.size());
            context->Unmap(jointBuffer.buffer, 0);

            ID3D11ShaderResourceView * srvs[]{ jointBuffer.srv };
            context->CSSetShaderResources(0u, 1u, srvs);
            context->CSSetShaderResources(1u, 4u, preskin.srvs);
            context->CSSetUnorderedAccessViews(0u, 1u, mesh.uavs, nullptr);
            context->CSSetShader(renderer.computeShader.Get(), nullptr, 0);
            uint32_t numDispatches = uint32_t(ceilf(float(mesh.numVertices) / 64.0f));
            context->Dispatch(numDispatches, 1, 1);
        }
    }

    ID3D11UnorderedAccessView* nullUAV = NULL;
    context->CSSetUnorderedAccessViews(1, 1, &nullUAV, nullptr);
    
    renderer.deviceResources->PIXEndEvent();

    renderer.deviceResources->PIXBeginEvent(L"Render Meshes");

    // TODO: Build Constant buffer data in batches
    Matrix viewProjTransform = view.viewTransform * view.projection;

    for (size_t entityId = 0; entityId < registry.numEntities; ++entityId) {
        const uint32_t cmpMask = registry.cmpMasks[entityId];
        const uint32_t requirements = bdr::CmpMasks::MESH | bdr::CmpMasks::MATERIAL;
        if ((cmpMask & requirements) == requirements) {
            bdr::DrawConstants& drawConstants = registry.drawConstants[entityId];
            const bdr::Material& material = registry.materials[entityId];
            const bdr::Mesh& mesh = renderer.meshes[registry.meshes[entityId]];

            // Set IAInputLayout
            context->IASetVertexBuffers(0, mesh.numPresentAttr, mesh.vertexBuffers, mesh.strides, offsets);
            context->IASetIndexBuffer(mesh.indexBuffer, mesh.indexFormat, 0);
            context->IASetInputLayout(renderer.inputLayoutManager[mesh.inputLayoutHandle]);

            // Set shaders
            context->VSSetShader(material.vertexShader, nullptr, 0);
            context->PSSetShader(material.pixelShader, nullptr, 0);

            // Set constant buffers
            drawConstants.MVP = (registry.globalMatrices[entityId] * viewProjTransform).Transpose();
            material.vertexCB.copyToGPU(context, drawConstants);

            ID3D11Buffer* vsBuffers[] = { material.vertexCB };
            context->VSSetConstantBuffers(0, 1, vsBuffers);

            //ID3D11Buffer* psBuffers[] = { material.pixelCB };
            //context->PSSetConstantBuffers(0, 1, psBuffers);
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context->DrawIndexed(mesh.numIndices, 0, 0);
        }
    }
    ID3D11Buffer* nullVB = nullptr;
    uint32_t nullStrides[]{ 0 };
    uint32_t nullOffsets[]{ 0 };
    context->IASetVertexBuffers(0, 1, &nullVB, nullStrides, nullOffsets);
    renderer.deviceResources->PIXEndEvent();
}

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

    bdr::ECSRegistry& registry = m_scene.registry;
    
    for (const bdr::Animation& animation : m_scene.animations) {
        updateAnimation(registry, animation, totalTime);
    }
    updateMatrices(registry);
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

    bdr::View view = { m_view, m_proj };
    renderScene(m_renderer, m_scene, view);

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
    bdr::gltf::SceneData sceneData{ &m_scene, &m_renderer, "polly/", "project_polly.gltf" };
    bdr::gltf::loadModel(sceneData);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    m_view = Matrix::CreateLookAt(Vector3{ 2.0f, 2.0f, 2.0f }, Vector3::Zero, Vector3::UnitY);
    float width = float(m_renderer.width);
    float height = float(m_renderer.height);
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.0f, width / height, 0.1f, 100.f);
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
