#include "pch.h"
#include "RenderFeature.h"
#include "Scene.h"

namespace bdr
{
    SkinnedRenderFeature::~SkinnedRenderFeature()
    {
        pVertexShader->Release();
        pPixelShader->Release();

        for (auto drawCB : drawCBs) {
            drawCB->Release();
        }
    }
    
    void SkinnedRenderFeature::init(ID3D11Device* device)
    {
        pDevice = device;
        // Need to initialize our fragment and pixel shader
        auto blob = DX::ReadData(L"skinned.cso");
        DX::ThrowIfFailed(pDevice->CreateVertexShader(blob.data(), blob.size(), nullptr, &pVertexShader));
        DX::ThrowIfFailed(pDevice->CreatePixelShader(blob.data(), blob.size(), nullptr, &pPixelShader));
    }

    size_t SkinnedRenderFeature::addRenderObject(const RenderObject& renderObject, const DrawConstants& drawConstants)
    {
        ID3D11Buffer* drawCB = nullptr;
        // Supply the vertex shader constant data.
        
        D3D11_BUFFER_DESC cbDesc;
        cbDesc.ByteWidth = sizeof(SkinnedDrawConstants);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbDesc.MiscFlags = 0;
        cbDesc.StructureByteStride = 0;

        // Fill in the subresource data.
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = &drawConstants;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;
        
        // Create the buffer.
        DX::ThrowIfFailed(pDevice->CreateBuffer(&cbDesc, &InitData, &drawCB));
        drawCBs.push_back(drawCB);
        renderObjects.push_back(renderObject);
    }
    
    void SkinnedRenderFeature::updateSkinnedData(const size_t idx, const DrawConstants& drawConstant)
    {
        drawCBs[idx]
    }
}