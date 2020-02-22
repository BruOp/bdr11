#pragma once
#include "pch.h"

#include <unordered_map>
#include "Mesh.h"


namespace bdr
{
    class InputLayoutManager
    {
    public:
        InputLayoutManager() = default;

        UNCOPIABLE(InputLayoutManager);
        UNMOVABLE(InputLayoutManager);

        ~InputLayoutManager()
        {
            reset();
        }

        void init(ID3D11Device* device)
        {
            pDevice = device;
        }

        void reset()
        {
            for (auto& inputLayout : inputLayouts) {
                if (inputLayout.second != nullptr) {
                    inputLayout.second->Release();
                    inputLayout.second = nullptr;
                }
            }
            inputLayouts.clear();
        };

        ID3D11InputLayout* getOrCreateInputLayout(const MeshDesc& meshCreationInfo)
        {
            uint64_t key = getKey(meshCreationInfo);
            if (inputLayouts.count(key) == 0) {
                inputLayouts[key] = createInputLayout(meshCreationInfo);
            }

            return inputLayouts[key];
        }


    private:
        ID3D11Device* pDevice = nullptr;
        std::unordered_map<uint64_t, ID3D11InputLayout*> inputLayouts;

        uint64_t getKey(const MeshDesc& meshCreationInfo);
        ID3D11InputLayout* createInputLayout(const MeshDesc& meshCreationInfo);
    };
}
