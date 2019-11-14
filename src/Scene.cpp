#include "Scene.h"
#include <iostream>

#include "pch.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

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

        if (gltfModel.scenes.size()) {
            throw std::runtime_error("Cannot handle a GLTF with more than one scene at the moment!");
        }

        const tinygltf::Scene& inputScene = gltfModel.scenes[gltfModel.defaultScene];
        scene.nodeList.resize(inputScene.nodes.size());
        for (const int nodeIdx : inputScene.nodes) {
            const tinygltf::Node& node = gltfModel.nodes[nodeIdx];
            Matrix& localTransform = scene.nodeList.localTransforms[nodeIdx];

            // Process our current node's local transform
            localTransform = processLocalTransform(node);

            // If there are children, add the reference to the current node
            if (node.children.size() > 0) {
                for (const int childIdx : node.children) {
                    ASSERT(childIdx > nodeIdx);
                    ASSERT(childIdx < scene.nodeList.size());
                    scene.nodeList.parents[childIdx] = nodeIdx;
                }
            }
        }

        updateNodes(scene.nodeList);
    }

}
