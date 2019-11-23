#pragma once
#include "pch.h"

namespace bdr
{
    constexpr uint32_t MAX_NUM_JOINTS = 128u;
    struct RenderObject;

    struct SkinnedDrawConstants
    {
        DirectX::SimpleMath::Matrix modelViewProject;
        DirectX::SimpleMath::Matrix jointMatrices[MAX_NUM_JOINTS];
        uint32_t jointCount = 0;
        float padding[3];
    };

    class SkinnedRenderFeature
    {
    public:
        using DrawConstants = SkinnedDrawConstants;

        ~SkinnedRenderFeature();

        void init(ID3D11Device* device);
        size_t addRenderObject(const RenderObject& renderObject, const DrawConstants& drawConstant);
        
        void updateSkinnedData(const size_t idx, const DrawConstants& drawConstant);

    private:
        ID3D11Device* pDevice = nullptr;
        ID3D11VertexShader* pVertexShader = nullptr;
        ID3D11PixelShader* pPixelShader = nullptr;
        std::vector<ID3D11Buffer*> drawCBs;
        std::vector<RenderObject> renderObjects;
    };
}

