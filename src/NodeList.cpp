#include "pch.h"
#include "NodeList.h"

using namespace DirectX::SimpleMath;

namespace bdr
{
    Matrix calcLocalTransform(const Node& node)
    {
        Matrix localTransform = Matrix::Identity;
        if (node.transformMask & TransformType::Scale) {
            localTransform *= Matrix::CreateScale(node.scale);
        }
        if (node.transformMask & TransformType::Rotation) {
            localTransform = Matrix::Transform(localTransform, node.rotation);
        }
        if (node.transformMask & TransformType::Translation) {
            localTransform *= Matrix::CreateTranslation(node.translation);
        }
        return localTransform;
    };

    Matrix processGlobalTransform(const NodeList& nodeList, int32_t nodeIdx)
    {
        const int32_t parent = nodeList.nodes[nodeIdx].parent;
        if (parent == -1) {
            return nodeList.localTransforms[nodeIdx];
        }
        else {
            ASSERT(parent < nodeList.globalTransforms.size());
            ASSERT(parent < nodeIdx);
            return nodeList.localTransforms[nodeIdx] * nodeList.globalTransforms[parent];
        }
    }

    void updateNodes(NodeList& nodeList)
    {
        for (int32_t i = 0; i < nodeList.size(); i++) {
            // We can do this since we never re-order nodes and parents are guaranteed to be before their children!
            nodeList.localTransforms[i] = calcLocalTransform(nodeList.nodes[i]);
            nodeList.globalTransforms[i] = processGlobalTransform(nodeList, i);
        }
    }
}