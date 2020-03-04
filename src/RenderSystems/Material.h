#pragma once
#include "pch.h"

#include "Graphics/Resources.h"

namespace bdr
{
    class RenderSystem;

    struct MaterialFeatureDesc
    {
        std::string passName;
        std::string pipelineName;
        uint32_t shaderFlags;
    };

    // TODO: Have the magic '16' replaced by a sensible constant we can use across MaterialDesc and Material
    struct MaterialDesc
    {
        MaterialFeatureDesc features[16];
    };

    // TODO: Work out validation for PipelineState frame and view constants and our RenderViews
    // e.g. we should not be able to assign a PipelineState to a pass that has different outputs,
    // view constants, etc.

    struct Material
    {
        ResourceBindingLayout perDrawLayout;
        uint32_t passMask;
        uint32_t pipelinesPerPass[16];
    };

    struct MaterialInstance
    {
        uint16_t materialId;
        ResourceBinder binders[16];
    };

    uint32_t createMaterial(RenderSystem& renderSystem, const MaterialDesc& materialDesc);

    MaterialInstance createMaterialInstance(RenderSystem& renderSystem, const Material& material);

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

        uint32_t initMaterial(const MaterialDesc& materialDesc);

        uint32_t get(const std::string& name) const;

        void reset()
        {
            for (auto& material : materials) {
                bdr::reset(material);
            }
            materials.clear();
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

    ResourceBindingObject createRBO(const Material& material);

}

