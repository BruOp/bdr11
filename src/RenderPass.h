#pragma once
#include "pch.h"
#include "Scene.h"


namespace bdr
{
    constexpr uint32_t MAX_NUM_JOINTS = 128u;
    struct RenderObject;

    template<typename T>
    class ConstantBuffer
    {
    public:
        static_assert(sizeof(T) % 16 == 0, "Constant Buffer data structs must be 16 byte aligned");

        T data;
        Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
        bool gpuWritable = false;


        void init(ID3D11Device* device, bool makeGpuWritable = false)
        {
            gpuWritable = makeGpuWritable;
            D3D11_BUFFER_DESC desc;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.MiscFlags = 0;
            desc.ByteWidth = static_cast<uint32_t>(sizeof(T) + (16 - (sizeof(T) % 16)));

            if (gpuWritable) {
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.CPUAccessFlags = 0;
            }
            else {
                desc.Usage = D3D11_USAGE_DYNAMIC;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            }

            DX::ThrowIfFailed(device->CreateBuffer(&desc, nullptr, constantBuffer.ReleaseAndGetAddressOf()));
        }

        inline void updateData(T& newData)
        {
            data = newData;
        }

        inline void copyToGPU(ID3D11DeviceContext* deviceContext)
        {
            ASSERT(constantBuffer != nullptr);

            if (gpuWritable) {
                deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &data, 0, 0);
            }
            else {
                D3D11_MAPPED_SUBRESOURCE mappedResource;
                DXCall(deviceContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
                CopyMemory(mappedResource.pData, &data, sizeof(T));
                deviceContext->Unmap(Buffer, 0);
            }
        }

        inline void copyToGPU(ID3D11DeviceContext* deviceContext, const T& newData) const
        {
            ASSERT(constantBuffer != nullptr);

            if (gpuWritable) {
                deviceContext->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &newData, 0, 0);
            }
            else {
                D3D11_MAPPED_SUBRESOURCE mappedResource;
                DX::ThrowIfFailed(deviceContext->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
                CopyMemory(mappedResource.pData, &newData, sizeof(T));
                deviceContext->Unmap(constantBuffer.Get(), 0);
            }
        }

        void updateAndCopyToGPU(T& newData, ID3D11DeviceContext* deviceContext)
        {
            updateData(newData);
            copyToGPU(deviceContext);
        }

        void reset()
        {
            constantBuffer.Reset();
        }
    };

    struct BasicDrawConstants
    {
        DirectX::SimpleMath::Matrix modelViewProjection;
    };

    struct SkinnedDrawConstants
    {
        DirectX::SimpleMath::Matrix modelViewProjection;
        uint32_t jointCount = 0;
        float padding[3];
        DirectX::SimpleMath::Matrix jointMatrices[MAX_NUM_JOINTS];
    };

    template<typename T>
    class RenderFeatureDataManager
    {
    public:
        using DrawConstants = T;

        inline size_t addData(const DrawConstants& drawConstant)
        {
            drawConstants.push_back(drawConstant);
            return drawConstants.size() - 1;
        };
        
        inline size_t addData(const DrawConstants&& drawConstant)
        {
            drawConstants.push_back(drawConstant);
            return drawConstants.size() - 1;
        };

        void updateData(const size_t idx, const DrawConstants& drawConstant)
        {
            drawConstants.at(idx) = drawConstant;
        };

    private:
        std::vector<DrawConstants> drawConstants;
    };

    enum RenderFeatureFlags : uint32_t
    {
        Skinned = 1u,
    };

    class ShaderInfo
    {
    public:
        std::vector<uint8_t> blob;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader = nullptr;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader = nullptr;
        
        void init(ID3D11Device* device, const wchar_t vsFilePath[], const wchar_t psFilePath[])
        {
            blob = DX::ReadData(vsFilePath);
            DX::ThrowIfFailed(device->CreateVertexShader(blob.data(), blob.size(), nullptr, pVertexShader.ReleaseAndGetAddressOf()));
            std::vector<uint8_t> psBlob = DX::ReadData(psFilePath);
            DX::ThrowIfFailed(device->CreatePixelShader(psBlob.data(), psBlob.size(), nullptr, pPixelShader.ReleaseAndGetAddressOf()));
        }

        void reset()
        {
            pPixelShader.Reset();
            pVertexShader.Reset();
            blob.clear();
        }
    };


    class BasicRenderPass
    {
    public:
        ~BasicRenderPass();

        void init(ID3D11Device* device);
        void reset()
        {
            shaderInfo.reset();
            drawCB.reset();
            for (auto& renderObject : renderObjects) {
                renderObject.mesh.destroy();
            }
            renderObjects.clear();
        };

        ID3D11InputLayout* getInputLayout();
        void createInputLayout(ID3D11Device* device, const D3D11_INPUT_ELEMENT_DESC descs[], const size_t count);

        void render(ID3D11DeviceContext* context, const NodeList& nodeList, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj) const;
        size_t registerRenderObject(const RenderObject& renderObject);

        uint32_t requiredFeatures = 0;

        static constexpr wchar_t vsShaderFileName[] = L"basic_vs.cso";
        static constexpr wchar_t psShaderFileName[] = L"basic_ps.cso";

    private:
        Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
        ConstantBuffer<BasicDrawConstants> drawCB;
        ShaderInfo shaderInfo;
        std::vector<RenderObject> renderObjects;
    };
}

