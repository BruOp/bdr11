#include "pch.h"
#include "Scene.h"

namespace bdr
{
    void updateNodes(NodeList& nodeList)
    {
        for (size_t i = 0; i < nodeList.size(); i++) {
            const int32_t parent = nodeList.parents[i];
            if (parent == -1) {
                nodeList.globalTransforms[i] = nodeList.localTransforms[i];
            }
            else {
                ASSERT(parent < nodeList.globalTransforms.size());
                ASSERT(parent < i);
                nodeList.globalTransforms[i] = nodeList.localTransforms[i] * nodeList.globalTransforms[parent];
            }
        }
    }

    void loadGLTFModel(Scene& scene, const std::string& gltfFolder, const std::string& gltfFileName)
    {
    }
}
