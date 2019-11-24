#include "pch.h"
#include "RenderPass.h"

using namespace DirectX::SimpleMath;

namespace bdr
{

    BasicRenderPass::~BasicRenderPass()
    {
        reset();
    }

    void BasicRenderPass::init(ID3D11Device* device)
    {
        drawCB.init(device, false);
        shaderInfo.init(device, vsShaderFileName, psShaderFileName);
    }
    
    ID3D11InputLayout* BasicRenderPass::getInputLayout()
    {
        return pInputLayout.Get();
    }
    
    void BasicRenderPass::createInputLayout(ID3D11Device* device, const D3D11_INPUT_ELEMENT_DESC descs[], const size_t count)
    {
        DX::ThrowIfFailed(device->CreateInputLayout(
            descs,
            count,
            shaderInfo.blob.data(),
            shaderInfo.blob.size(),
            pInputLayout.ReleaseAndGetAddressOf()
        ));
    }
    
    void BasicRenderPass::render(ID3D11DeviceContext* context, const NodeList& nodeList, const Matrix& view, const Matrix& proj) const
    {
        // TODO: Manage rendering state?
        
        // Need to set the input layout
        context->IASetInputLayout(pInputLayout.Get());
        uint32_t offsets[6]{ 0u, 0u, 0u, 0u, 0u, 0u };
        for (const bdr::RenderObject& renderObject : renderObjects) {
            // TODO: Use render pass to set constant buffers
            // set the vertex and pixel shaders?
            context->VSSetShader(shaderInfo.pVertexShader.Get(), nullptr, 0);
            context->PSSetShader(shaderInfo.pPixelShader.Get(), nullptr, 0);
            // Bind the constant buffer
            const bdr::BasicDrawConstants drawConstants{
                (nodeList.globalTransforms[renderObject.SceneNodeIdx] * view * proj).Transpose()
            };
            drawCB.copyToGPU(context, drawConstants);
            ID3D11Buffer* cb = drawCB.constantBuffer.Get();
            context->VSSetConstantBuffers(0, 1, &cb);

            const bdr::Mesh& mesh = renderObject.mesh;
            context->IASetIndexBuffer(mesh.indexBuffer, mesh.indexFormat, 0);
            context->IASetVertexBuffers(0, mesh.vertexBuffers.numPresentAttributes, mesh.vertexBuffers.vertexBuffers.data(), mesh.vertexBuffers.strides, offsets);
            context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context->DrawIndexed(mesh.indexCount, 0, 0);
        }
    }
    
    size_t BasicRenderPass::registerRenderObject(const RenderObject & renderObject)
    {
        renderObjects.push_back(renderObject);
        return renderObjects.size() - 1;
    }
}