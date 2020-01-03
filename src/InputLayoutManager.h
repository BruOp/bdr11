#pragma once
#include "pch.h"

#include "Mesh.h"


namespace bdr
{
    struct InputLayoutDetail
    {
        enum class Type : uint8_t
        {
            FLOAT = 0,
            UINT,
            INT,
        };

        MeshAttributes attrMask = MeshAttributes::POSITION;
        Type type = Type::FLOAT;
        DXGI_FORMAT format = {};
        uint32_t vectorSize = 0;
        std::string semanticName = {};
    };

    class InputLayoutManager
    {
    public:
        static constexpr uint32_t numTexCoordFormats = 3;
        static constexpr uint32_t numJointFormats = 2;
        static constexpr uint32_t numWeightsFormats = 3;
        static constexpr uint32_t numLayouts = numTexCoordFormats * numJointFormats * numWeightsFormats;


        InputLayoutManager() : pDevice{ nullptr }, inputLayouts{ nullptr }
        {
            for (uint32_t i = 0; i < numLayouts; i++) {
                inputLayouts[i] = nullptr;
            }
        };
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
            for (size_t i = 0; i < _countof(inputLayouts); i++) {
                if (inputLayouts[i] != nullptr) {
                    inputLayouts[i]->Release();
                    inputLayouts[i] = nullptr;
                }
            }
        };

        uint32_t getInputLayoutIdx(const InputLayoutDetail* inputLayoutDetails, const uint32_t numDetails)
        {
            int32_t texCoordIndex = -1;
            int32_t jointsIndex = -1;
            int32_t weightsIndex = -1;
            for (size_t i = 0; i < numDetails; ++i) {
                const InputLayoutDetail& inputLayoutDetail = inputLayoutDetails[i];
                switch (inputLayoutDetail.attrMask) {
                case MeshAttributes::TEXCOORD:
                    if (inputLayoutDetail.format == DXGI_FORMAT_R32G32_FLOAT) {
                        texCoordIndex = 0;
                    }
                    else if (inputLayoutDetail.format == DXGI_FORMAT_R8G8_UNORM) {
                        texCoordIndex = 1;
                    }
                    else if (inputLayoutDetail.format == DXGI_FORMAT_R16G16_UNORM) {
                        texCoordIndex = 2;
                    }
                    break;
                case MeshAttributes::BLENDINDICES:
                    if (inputLayoutDetail.format == DXGI_FORMAT_R8G8B8A8_UINT) {
                        jointsIndex = 0;
                    }
                    else if (inputLayoutDetail.format == DXGI_FORMAT_R16G16B16A16_UINT) {
                        jointsIndex = 1;
                    }
                    break;
                case MeshAttributes::BLENDWEIGHT:
                    if (inputLayoutDetail.format == DXGI_FORMAT_R32G32B32A32_FLOAT) {
                        weightsIndex = 0;
                    }
                    else if (inputLayoutDetail.format == DXGI_FORMAT_R8G8B8A8_UNORM) {
                        weightsIndex = 1;
                    }
                    else if (inputLayoutDetail.format == DXGI_FORMAT_R16G16B16A16_UNORM) {
                        weightsIndex = 2;
                    }
                    break;
                }
            }
            ASSERT(texCoordIndex != -1);

            if (jointsIndex == -1) {
                ASSERT(weightsIndex == -1);
                return texCoordIndex;
            }
            else {
                ASSERT(weightsIndex != -1);
                return (numWeightsFormats * numJointFormats) * texCoordIndex + numWeightsFormats * jointsIndex + weightsIndex + numTexCoordFormats;
            }
        };

        ID3D11InputLayout* getInputLayout(const InputLayoutDetail* inputLayoutDetails, const uint32_t numDetails)
        {
            uint32_t idx = getInputLayoutIdx(inputLayoutDetails, numDetails);
            ASSERT(idx < _countof(inputLayouts));
            return inputLayouts[idx];
        }

        uint32_t getOrCreateInputLayout(const InputLayoutDetail* inputLayoutDetails, const uint32_t numDetails)
        {
            uint32_t idx = getInputLayoutIdx(inputLayoutDetails, numDetails);
            ID3D11InputLayout* il = inputLayouts[idx];
            if (il == nullptr) {
                il = createInputLayout(inputLayoutDetails, numDetails);
            }

            return idx;
        }

        ID3D11InputLayout* createInputLayout(const InputLayoutDetail* inputLayoutDetails, uint32_t numDetails);

        inline ID3D11InputLayout* operator[](const size_t index)
        {
            return inputLayouts[index];
        };
        inline const ID3D11InputLayout* operator[](const size_t index) const
        {
            return inputLayouts[index];
        };

    private:
        ID3D11Device* pDevice = nullptr;
        ID3D11InputLayout* inputLayouts[numLayouts];
    };
}
