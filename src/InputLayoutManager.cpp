#include "pch.h"

#include "d3dcompiler.h"
#include "InputLayoutManager.h"

ID3D11InputLayout* bdr::InputLayoutManager::createInputLayout(const InputLayoutDetail* inputLayoutDetails, uint32_t numDetails)
{
    // Create a matching shader for the vertex signature, taken from
    // https://www.gamedev.net/forums/topic/688029-how-to-cleanly-create-input-layouts/
    std::string elementsStr;
    for (size_t i = 0; i < numDetails; ++i) {
        const InputLayoutDetail& details = inputLayoutDetails[i];
        // Noticed that all the floating point formats I allow are powers of two.
        if (details.type == InputLayoutDetail::Type::FLOAT) {
            elementsStr += "float";
        }
        else if (details.type == InputLayoutDetail::Type::UINT) {
            elementsStr += "uint";
        }
        else {
            elementsStr += "int";
        }

        char buf[16];
        _itoa(details.vectorSize, buf, 10);
        elementsStr += buf;

        elementsStr += " a";
        _itoa(i, buf, 10);
        elementsStr += buf;

        elementsStr += " : ";
        elementsStr += details.semanticName;
        elementsStr += ";\n";
    }

    const char* shaderTemplate = "\
struct SimpleVertex \n\
{ \n\
    %s \
}; \n\
float4 main(SimpleVertex vi) : SV_Position \n\
{ \n\
    return float4(0.0, 0.0, 0.0, 0.0); \n\
}\0";

    char shaderBuf[1024];
    sprintf_s(shaderBuf, shaderTemplate, elementsStr.c_str());

    D3D11_INPUT_ELEMENT_DESC descs[Mesh::maxAttrCount]{};
    for (uint32_t i = 0; i < numDetails; i++) {
        const auto& details = inputLayoutDetails[i];

        descs[i] = {
            details.semanticName.c_str(),
            0,
            details.format,
            i,
            D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA,
            0,
        };
    }

    ID3DBlob* blob;
    DX::ThrowIfFailed(D3DCompile(
        shaderBuf,
        strlen(shaderBuf),
        nullptr, nullptr, nullptr,
        "main", "vs_4_0_level_9_3",
        0, 0,
        &blob,
        nullptr
    ));
    

    uint32_t idx = getInputLayoutIdx(inputLayoutDetails, numDetails);
    DX::ThrowIfFailed(pDevice->CreateInputLayout(
        descs,
        numDetails,
        blob->GetBufferPointer(),
        blob->GetBufferSize(),
        &inputLayouts[idx]
    ));

    return inputLayouts[idx];
}
