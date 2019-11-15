#include "pch.h"

#include "Scene.h"
#include <iostream>


#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"
#include "Scene.h"

using namespace DirectX::SimpleMath;

namespace bdr
{
    bool loadImageDataCallback(
        tinygltf::Image* image,
        const int image_idx,
        std::string* err,
        std::string* warn,
        int req_width,
        int req_height,
        const unsigned char* bytes,
        int size,
        void* user_data)
    {
        return true;
    };

    Matrix processLocalTransform(const tinygltf::Node& node)
    {
        Matrix localTransform = Matrix::Identity;
        if (node.scale.size() == 3) {
            Matrix scale = Matrix::CreateScale(
                static_cast<float>(node.scale[0]),
                static_cast<float>(node.scale[1]),
                static_cast<float>(node.scale[2])
            );
            localTransform *= scale;
        }

        if (node.rotation.size() == 4) {
            Matrix rotation = Matrix::Transform(localTransform,
                Quaternion{
                    static_cast<float>(node.rotation[0]),
                    static_cast<float>(node.rotation[1]),
                    static_cast<float>(node.rotation[2]),
                    static_cast<float>(node.rotation[3]),
                }
            );
        }

        if (node.translation.size() == 3) {
            localTransform.Translation(
                Vector3{
                    static_cast<float>(node.translation[0]),
                    static_cast<float>(node.translation[1]),
                    static_cast<float>(node.translation[2]),
                }
            );
        }
        return localTransform;
    };

    /*
    Process the local transform and the parent relationship of our node, returning the next nodeIdx to index into our scene
    */
    int32_t processNode(Scene& scene, const tinygltf::Model& inputModel, int32_t inputNodeIdx, int32_t nodeIdx, int32_t parentIdx)
    {
        ASSERT(nodeIdx < int32_t(scene.nodeList.size()));
        ASSERT(parentIdx < int32_t(scene.nodeList.size()));
        const tinygltf::Node& node = inputModel.nodes[inputNodeIdx];

        scene.nodeList.localTransforms[nodeIdx] = processLocalTransform(node);
        scene.nodeList.parents[nodeIdx] = parentIdx;

        // If there are children, add the reference to the current node
        if (node.children.size() > 0) {
            int32_t childIdx = nodeIdx + 1;
            for (const int inputChildIdx : node.children) {
                childIdx = processNode(scene, inputModel, inputChildIdx, childIdx, nodeIdx);
            }
            return childIdx;
        }
        else {
            return ++nodeIdx;
        }
    }


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
    };

    void loadGLTFModel(Scene& scene, const std::string& gltfFolder, const std::string& gltfFileName)
    {
        tinygltf::TinyGLTF loader;
        loader.SetImageLoader(loadImageDataCallback, nullptr);
        std::string err, warn;
        tinygltf::Model gltfModel;
        bool res = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, gltfFolder + gltfFileName);

        if (!warn.empty()) {
            std::cout << warn << std::endl;
        }

        if (!err.empty()) {
            std::cout << err << std::endl;
        }

        if (!res) {
            throw std::runtime_error("Failed to load GLTF Model");
        }

        if (gltfModel.scenes.size() != 1) {
            throw std::runtime_error("Cannot handle a GLTF with more than one scene at the moment!");
        }

        const tinygltf::Scene& inputScene = gltfModel.scenes[gltfModel.defaultScene];
        scene.nodeList.resize(gltfModel.nodes.size());
        int32_t nodeIdx = 0;
        for (const int inputNodeIdx : inputScene.nodes) {
            nodeIdx = processNode(scene, gltfModel, inputNodeIdx, nodeIdx, -1);
        }

        updateNodes(scene.nodeList);
    }
}
