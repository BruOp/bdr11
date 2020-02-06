#include "pch.h"
#include "IcosahedronFactory.h"

namespace bdr
{
    float t = (1.0f + glm::sqrt(5.0f)) / 2.0f;

    std::vector<glm::vec3> basicIcosahedronPositions = {
        {-1.0f,  t,  0.0f},
        { 1.0,  t,  0.0f},
        {-1.0f, -t,  0.0f},
        { 1.0, -t,  0.0f},

        { 0.0f, -1.0f,  t},
        { 0.0f,  1.0,  t},
        { 0.0f, -1.0f, -t},
        { 0.0f,  1.0, -t},

        { t, 0.0f, -1.0f},
        { t, 0.0f,  1.0},
        {-t, 0.0f, -1.0f},
        {-t, 0.0f,  1.0},
    };
    //The number of points horizontally
    constexpr float w = 5.5f;
    //The number of points vertically
    constexpr float h = 3.0f;

    std::vector<uint16_t> basicIcosahedronIndices = {
        0, 11, 5,
        0, 5, 1,
        0, 1, 7,
        0, 7, 10,
        0, 10, 11,
        1, 5, 9,
        5, 11, 4,
        11, 10, 2,
        10, 7, 6,
        7, 1, 8,
        3, 9, 4,
        3, 4, 2,
        3, 2, 6,
        3, 6, 8,
        3, 8, 9,
        4, 9, 5,
        2, 4, 11,
        6, 2, 10,
        8, 6, 7,
        9, 8, 1
    };

    IcosahedronFactory::IcosahedronFactory(uint8_t detail)
        : detail{ detail },
        vertices{ basicIcosahedronPositions },
        indices{ basicIcosahedronIndices },
        normals(basicIcosahedronPositions.size()),
        uvs(basicIcosahedronPositions.size())
    {
        const size_t N = basicIcosahedronPositions.size();
        for (size_t i = 0; i < N; ++i) {
            glm::vec3& v = vertices[i];
            v = glm::normalize(v);
            normals[i] = v;
            uvs[i] = glm::vec2{
                atan2f(v.y, v.x),
                atan2f(sqrtf(pow(v.x, 2.0f) + pow(v.y, 2.0f)), v.z)
            };
        }

        for (uint8_t subdivisionLevel = 0; subdivisionLevel < detail; ++subdivisionLevel) {
            size_t numIndices = indices.size();
            std::vector<uint16_t> newIndices(numIndices * 4);
            // For each face
            for (size_t i = 0; i < numIndices; i += 3) {
                uint16_t i_a = indices[i];
                uint16_t i_b = indices[i + 1];
                uint16_t i_c = indices[i + 2];

                uint16_t i_ab = getOrCreateMidPoint(i_a, i_b);
                uint16_t i_bc = getOrCreateMidPoint(i_b, i_c);
                uint16_t i_ac = getOrCreateMidPoint(i_a, i_c);

                newIndices.insert(newIndices.end(), {
                    i_a, i_ab, i_ac,
                    i_b, i_bc, i_ab,
                    i_c, i_ac, i_bc,
                    i_ab, i_bc, i_ac
                    });
            }
            indices = std::move(newIndices);
        }
    }

    //Mesh IcosahedronFactory::getMesh()
    //{
    //    const bgfx::Memory* vertMemory = bgfx::copy(vertices.data(), uint32_t(vertices.size()) * sizeof(vertices[0]));
    //    const bgfx::Memory* indexMemory = bgfx::copy(indices.data(), uint32_t(indices.size()) * sizeof(indices[0]));

    //    Mesh mesh{};
    //    bgfx::VertexDecl decl;
    //    decl.begin()
    //        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
    //        .end();
    //    mesh.addVertexHandle(bgfx::createVertexBuffer(vertMemory, decl));
    //    mesh.indexHandle = bgfx::createIndexBuffer(indexMemory);

    //    return mesh;
    //}

    uint16_t IcosahedronFactory::getOrCreateMidPoint(uint16_t first, uint16_t second)
    {
        uint32_t smaller = glm::min(first, second);
        uint32_t larger = glm::max(first, second);
        uint32_t key = (smaller << 16) | larger;
        const auto& iter = newVertices.find(key);

        if (iter != newVertices.end()) {
            return iter->second;
        }
        // Calculate and store new vertex
        glm::vec3 firstPos = vertices[first];
        glm::vec3 secondPos = vertices[second];

        glm::vec3 midPoint{ glm::normalize(glm::vec3{ 0.5f * (secondPos + firstPos) }) };
        glm::vec2 uvMidPoint{ glm::normalize(glm::vec2{ 0.5f * (uvs[first] + uvs[second]) }) };
        uint16_t newIndex = uint16_t(vertices.size());

        vertices.push_back(midPoint);
        uvs.push_back(uvMidPoint);
        normals.push_back(midPoint);

        newVertices[key] = newIndex;
        return newIndex;
    };
}
