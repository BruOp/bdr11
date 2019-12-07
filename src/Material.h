#pragma once
#include "pch.h"
#include "DXHelpers.h"


namespace bdr
{

    struct DrawConstants
    {
        DirectX::SimpleMath::Matrix MVP;
        DirectX::SimpleMath::Matrix invMVP;
    };

    struct GenericMaterialData
    {
        float data[64];

        inline float& operator[](const size_t index)
        {
            return data[index];
        }
        inline const float& operator[](const size_t index) const
        {
            return data[index];
        }
    };

    struct Material
    {
        ID3D11VertexShader* vertexShader;
        ID3D11PixelShader* pixelShader;
        ConstantBuffer<DrawConstants> vertexCB;
        ConstantBuffer<GenericMaterialData> pixelCB;
        
        void Material::reset();
    };


    class MaterialManager
    {
    public:
        MaterialManager() = default;
        ~MaterialManager()
        {
            reset();
        }

        uint32_t initMaterial(ID3D11Device* device, const std::wstring& vsFile, const std::wstring& psFile);
        
        void reset()
        {
            for (auto& material : materials) {
                material.reset();
            }
        }

        inline Material& operator[](const size_t index)
        {
            return materials[index];
        }
        inline const Material& operator[](const size_t index) const
        {
            return materials[index];
        }

    private:
        std::vector<Material> materials;
    };
}

