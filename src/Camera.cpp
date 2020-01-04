#include "pch.h"
#include "Camera.h"


using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace bdr
{
    enum MoveDir : uint8_t
    {
        LEFT = 1 << 0,
        RIGHT = 1 << 1,
        UP = 1 << 2,
        DOWN = 1 << 3,
        FORWARD = 1 << 4,
        BACKWARD = 1 << 5
    };

    void OrbitCameraController::update(const DirectX::Keyboard::State&, const DirectX::Mouse::State& mouseState, const float deltaTime)
    {
        ASSERT(camera != nullptr);
        if (mouseState.positionMode != Mouse::MODE_RELATIVE) {
            return;
        }

        if (mouseState.leftButton) {
            yaw -= deltaTime * yawSensitivity * mouseState.x;
            pitch += deltaTime * pitchSensitivity * mouseState.y;
        }

        if (mouseState.rightButton) {
            origin.x += deltaTime * mouseState.x;
            origin.z += deltaTime * mouseState.y;
        }

        radius += deltaTime * zoomSensitivity * mouseState.scrollWheelValue;
        constexpr float pitchLimit = 0.1f;
        constexpr float radiusLimit = 0.1f;

        pitch = std::min(std::max(pitch, pitchLimit), XM_PI - pitchLimit);
        if (yaw > XM_PI) {
            yaw -= XM_2PI;
        }
        else if (yaw < -XM_PI) {
            yaw += XM_2PI;
        }
        radius = std::max(radius, radiusLimit);

        Vector3 posOnSphere = {
            sinf(pitch) * cosf(yaw),
            cosf(pitch),
            sinf(pitch) * sinf(yaw),
        };
        posOnSphere *= radius;
        camera->view = Matrix::CreateLookAt(posOnSphere + origin, origin, Vector3::UnitY);;
    }
}
