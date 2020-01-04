#pragma once
#include "pch.h"

#include "DXHelpers.h"
#include "ECSRegistry.h"

namespace bdr
{
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

        UNMOVABLE(MaterialManager);
        UNCOPIABLE(MaterialManager);

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

