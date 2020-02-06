#include "pch.h"
#include <unordered_map>


namespace bdr
{
    class IcosahedronFactory
    {
    public:
        IcosahedronFactory(uint8_t detail);

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<uint16_t> indices;


    private:
        uint8_t detail;
        std::unordered_map<uint32_t, uint16_t> newVertices;

        uint16_t getOrCreateMidPoint(uint16_t first, uint16_t second);
    };
}
