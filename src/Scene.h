#pragma once
#include "pch.h"
#include <vector>

namespace bdr
{
    struct NodeList
    {
        std::vector<DirectX::SimpleMath::Matrix> localTransforms;
        std::vector<DirectX::SimpleMath::Matrix> globalTransforms;
        std::vector<int32_t> parents;
        
        size_t size() const
        {
            ASSERT(localTransforms.size() == globalTransforms.size() == parents.size());
            return localTransforms.size();
        };
    };

    void updateNodes(NodeList& nodeList);

    struct Scene
    {
        NodeList nodeList;
        // Need a list of Models
    };

    void loadGLTFModel(Scene& scene, const std::string& gltfFileName);
}