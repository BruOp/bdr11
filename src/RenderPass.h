#pragma once
#include "pch.h"
#include "Scene.h"


namespace bdr
{
    /*constexpr uint32_t MAX_NUM_JOINTS = 62u;

    struct BasicDrawConstants
    {
        DirectX::SimpleMath::Matrix modelViewProjection;
    };

    struct SkinnedDrawConstants
    {
        DirectX::SimpleMath::Matrix modelViewProjection;
        uint32_t jointCount = 0;
        float padding[3] = { 0.0f, 0.0f, 0.0f };
        DirectX::SimpleMath::Matrix jointMatrices[MAX_NUM_JOINTS];
    };*/

    //class ShaderInfo
    //{
    //public:
    //    Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader = nullptr;
    //    Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader = nullptr;

    //    void init(ID3D11Device* device, const wchar_t vsFilePath[], const wchar_t psFilePath[])
    //    {
    //        const auto& blob = DX::ReadData(vsFilePath);
    //        DX::ThrowIfFailed(device->CreateVertexShader(blob.data(), blob.size(), nullptr, pVertexShader.ReleaseAndGetAddressOf()));
    //        std::vector<uint8_t> psBlob = DX::ReadData(psFilePath);
    //        DX::ThrowIfFailed(device->CreatePixelShader(psBlob.data(), psBlob.size(), nullptr, pPixelShader.ReleaseAndGetAddressOf()));
    //    }

    //    void reset()
    //    {
    //        pPixelShader.Reset();
    //        pVertexShader.Reset();
    //    }
    //};


    //class BasicRenderPass
    //{
    //public:
    //    ~BasicRenderPass();

    //    void init(ID3D11Device* device);
    //    void reset()
    //    {
    //        shaderInfo.reset();
    //        drawCB.reset();
    //        for (auto& renderObject : renderObjects) {
    //            renderObject.mesh.destroy();
    //        }
    //        renderObjects.clear();
    //    };

    //    inline ID3D11InputLayout* getInputLayout() const
    //    {
    //        return pInputLayout.Get();
    //    }
    //    void createInputLayout(ID3D11Device* device, const D3D11_INPUT_ELEMENT_DESC descs[], const size_t count);

    //    void render(ID3D11DeviceContext* context, const NodeList& nodeList, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj) const;
    //    size_t registerRenderObject(const RenderObject& renderObject);

    //    uint32_t requiredFeatures = 0;

    //    static constexpr wchar_t vsShaderFileName[] = L"basic_vs.cso";
    //    static constexpr wchar_t psShaderFileName[] = L"basic_ps.cso";

    //private:
    //    Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
    //    ConstantBuffer<BasicDrawConstants> drawCB;
    //    ShaderInfo shaderInfo;
    //    std::vector<RenderObject> renderObjects;
    //};

    //class SkinnedRenderPass
    //{
    //public:
    //    ~SkinnedRenderPass() { reset(); };

    //    void init(ID3D11Device* device);
    //    void reset()
    //    {
    //        shaderInfo.reset();
    //        drawCB.reset();
    //        for (auto& renderObject : renderObjects) {
    //            renderObject.mesh.destroy();
    //        }
    //        renderObjects.clear();
    //    };

    //    inline ID3D11InputLayout* getInputLayout() const
    //    {
    //        return pInputLayout.Get();
    //    }
    //    void createInputLayout(ID3D11Device* device, const D3D11_INPUT_ELEMENT_DESC descs[], const size_t count);

    //    void render(ID3D11DeviceContext* context, const Scene& scene, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj) const;
    //    size_t registerRenderObject(const RenderObject& renderObject);

    //    uint32_t requiredFeatures = 0;

    //    static constexpr wchar_t vsShaderFileName[] = L"skinned_basic_vs.cso";
    //    static constexpr wchar_t psShaderFileName[] = L"skinned_basic_ps.cso";

    //private:
    //    Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
    //    ConstantBuffer<SkinnedDrawConstants> drawCB;
    //    ShaderInfo shaderInfo;
    //    std::vector<RenderObject> renderObjects;
    //    //RenderFeatureDataManager<SkinnedDrawConstants> renderFeatureData;
    //};



    //class RenderPassManager
    //{
    //public:
    //    void init(ID3D11Device* device)
    //    {
    //        basic.init(device);
    //        skinnedBasic.init(device);
    //    }
    //    void reset()
    //    {
    //        basic.reset();
    //        skinnedBasic.reset();
    //    }

    //    void render(ID3D11DeviceContext* context, const Scene& scene, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj) const
    //    {
    //        basic.render(context, scene.nodeList, view, proj);
    //        skinnedBasic.render(context, scene, view, proj);
    //    }

    //    size_t registerRenderObject(const RenderObject& renderObject);
    //    ID3D11InputLayout* getInputLayout(const RenderObject& renderObject);
    //    void createInputLayout(const RenderObject& renderObject, ID3D11Device* device, const D3D11_INPUT_ELEMENT_DESC descs[], const size_t count);


    //private:

    //    BasicRenderPass basic;
    //    SkinnedRenderPass skinnedBasic;
    //};
}

