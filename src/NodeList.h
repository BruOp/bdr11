#pragma once
#include "pch.h"


namespace bdr
{
    enum TransformType : uint8_t
    {
        Rotation = 1,
        Translation = 2,
        Scale = 4,
        // Weights not supported
    };

    struct Node
    {
        DirectX::SimpleMath::Vector4 rotation;
        DirectX::SimpleMath::Vector3 scale;
        DirectX::SimpleMath::Vector3 translation;
        int32_t parent = -1;
        uint8_t transformMask = 0;
    };

    struct NodeList
    {
        std::vector<DirectX::SimpleMath::Matrix> localTransforms;
        std::vector<DirectX::SimpleMath::Matrix> globalTransforms;
        std::vector<Node> nodes;

        size_t size() const
        {
            ASSERT(localTransforms.size() == globalTransforms.size());
            ASSERT(localTransforms.size() == nodes.size());
            return localTransforms.size();
        };

        void resize(size_t newSize)
        {
            localTransforms.resize(newSize);
            globalTransforms.resize(newSize);
            nodes.resize(newSize);
        };
    };

    DirectX::SimpleMath::Matrix calcLocalTransform(const Node& node);
    DirectX::SimpleMath::Matrix processGlobalTransform(const NodeList& nodeList, int32_t nodeIdx);
    void updateNodes(NodeList& nodeList);
}