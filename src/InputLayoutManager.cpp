#include "pch.h"

#include "d3dcompiler.h"
#include "InputLayoutManager.h"

namespace bdr
{
    std::string getSemanticName(const MeshAttribute attrBit)
    {
        switch (attrBit) {
        case MeshAttribute::POSITION:
            return "SV_Position";
        case MeshAttribute::NORMAL:
            return "NORMAL";
        case MeshAttribute::TEXCOORD:
            return "TEXCOORD";
        case MeshAttribute::BLENDINDICES:
            return "BLENDINDICES";
        case MeshAttribute::BLENDWEIGHT:
            return "BLENDWEIGHT";
        case MeshAttribute::COLOR:
            return "COLOR";
        }
    }

    uint64_t InputLayoutManager::getKey(const MeshCreationInfo& meshCreationInfo)
    {
        uint64_t attrMask = 0;
        uint64_t formats = 0;
        uint8_t bufferNumber = 0;
        for (size_t i = 0; i < meshCreationInfo.numAttributes; ++i) {
            if (meshCreationInfo.bufferUsages[i] == BufferUsage::Unused) {
                continue;
            }
            attrMask |= meshCreationInfo.attributes[i];
            formats |= uint64_t(meshCreationInfo.bufferFormats[i]) << (8u * bufferNumber++);
        }
        return attrMask | (formats << 8);
    }

    ID3D11InputLayout* InputLayoutManager::createInputLayout(const MeshCreationInfo& meshCreationInfo)
    {
        // Create a matching shader for the vertex signature, taken from
        // https://www.gamedev.net/forums/topic/688029-how-to-cleanly-create-input-layouts/
        uint8_t bufferNumber = 0;
        std::string elementsStr{};
        for (size_t i = 0; i < meshCreationInfo.numAttributes; ++i) {
            if (meshCreationInfo.bufferUsages[i] == BufferUsage::Unused) {
                continue;
            }
            // Noticed that all the floating point formats I allow are powers of two.
            switch (meshCreationInfo.bufferFormats[i]) {
            case BufferFormat::FLOAT_2:
                elementsStr += "float2";
                break;
            case BufferFormat::FLOAT_3:
                elementsStr += "float3";
                break;
            case BufferFormat::FLOAT_4:
                elementsStr += "float4";
                break;
            case BufferFormat::UINT16:
            case BufferFormat::UINT32:
                elementsStr += "uint";
                break;
            case BufferFormat::UNORM8_2:
            case BufferFormat::UNORM16_2:
                elementsStr += "float2";
                break;
            case BufferFormat::UINT8_4:
            case BufferFormat::UINT16_4:
                elementsStr += "uint4";
                break;
            case BufferFormat::UNORM8_4:
            case BufferFormat::UNORM16_4:
                elementsStr += "float4";
                break;
            }

            char buf[16]{};
            elementsStr += " a";
            _itoa_s(i, buf, 10);
            elementsStr += buf;

            elementsStr += " : ";
            elementsStr += getSemanticName(meshCreationInfo.attributes[i]);
            elementsStr += ";\n";
        }

        const char* shaderTemplate = "\
struct SimpleVertex \n\
{ \n\
    %s \
}; \n\
float4 main(SimpleVertex vi) : SV_Position \n\
{ \n\
    return float4(0.0, 0.0, 0.0, 1.0); \n\
}\0";

        char shaderBuf[1024];
        sprintf_s(shaderBuf, shaderTemplate, elementsStr.c_str());

        std::string semanticNames[Mesh::maxAttrCount]{};
        bufferNumber = 0;
        D3D11_INPUT_ELEMENT_DESC descs[Mesh::maxAttrCount]{};
        for (uint32_t i = 0; i < meshCreationInfo.numAttributes; i++) {
            if (meshCreationInfo.bufferUsages[i] == BufferUsage::Unused) {
                continue;
            }
            semanticNames[bufferNumber] = getSemanticName(meshCreationInfo.attributes[i]);
            descs[bufferNumber] = {
                semanticNames[bufferNumber].c_str(),
                0,
                mapFormatToDXGI(meshCreationInfo.bufferFormats[i]),
                bufferNumber,
                D3D11_APPEND_ALIGNED_ELEMENT,
                D3D11_INPUT_PER_VERTEX_DATA,
                0,
            };
            bufferNumber++;
        }

        ID3DBlob* error = nullptr;
        ID3DBlob* blob;
        auto result = D3DCompile(
            shaderBuf,
            strlen(shaderBuf),
            nullptr, nullptr, nullptr,
            "main", "vs_4_0_level_9_3",
            0, 0,
            &blob,
            &error
        );
        if (error) {
            ERROR((char*)error->GetBufferPointer());
        }
        DX::ThrowIfFailed(result);

        uint64_t key = getKey(meshCreationInfo);
        DX::ThrowIfFailed(pDevice->CreateInputLayout(
            descs,
            bufferNumber,
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            &inputLayouts[key]
        ));
        blob->Release();

        return inputLayouts[key];
    }
}