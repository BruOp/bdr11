#pragma once
#include "pch.h"

#include "DXHelpers.h"
#include "Resources.h"

namespace bdr
{
    void reset(Material& material);

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

        inline void init(ID3D11Device* device)
        {
            this->pDevice = device;
        }

        uint32_t initMaterial(const std::wstring& vsFile, const std::wstring& psFile);
        uint32_t initMaterial(ID3DBlob* vsBlob, ID3DBlob* psBlob);

        uint32_t MaterialManager::getMaterial(const MaterialType type, const uint16_t permutation) const;

        void reset()
        {
            for (auto& material : materials) {
                bdr::reset(material);
            }
        }
        inline size_t size() const
        {
            return materials.size();
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
        ID3D11Device* pDevice;
        std::vector<Material> materials;
    };

    struct PBRConstants
    {
        float baseColorFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        float emissiveFactor[3] = { 0.0f, 0.0f, 0.0f };
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        float alphaCutoff = 1.0f;
        uint32_t alphaMode = 0;
        float padding[53];
    };
    //static_assert(sizeof(PBRConstants) == sizeof(GenericMaterialData), "PBR Constants must be the same size as GenericMaterialData");

    // TODO: Move to a sensible place
    std::string readFile(const char* filePath);
    void compileShader(const char* code, const D3D_SHADER_MACRO* macros, ID3DBlob** vsBlob, ID3DBlob** psBlob);

    // Get or create new PBR material based on permutation flags
    uint32_t getOrCreatePBRMaterial(MaterialManager& materialManager, const uint16_t textureFlags);
}

