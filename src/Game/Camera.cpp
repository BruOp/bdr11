#include "pch.h"
#include "Camera.h"


using namespace DirectX;

using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;

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
        tracker.Update(mouseState);


        if (tracker.leftButton == ButtonState::PRESSED || tracker.rightButton == ButtonState::PRESSED) {
            Mouse::Get().SetMode(Mouse::MODE_RELATIVE);
        }
        else if (tracker.leftButton == ButtonState::RELEASED || tracker.rightButton == ButtonState::RELEASED) {
            Mouse::Get().SetMode(Mouse::MODE_ABSOLUTE);
        }

        if (mouseState.positionMode == Mouse::MODE_RELATIVE) {
            if (mouseState.leftButton) {
                yaw -= deltaTime * yawSensitivity * mouseState.x;
                pitch += deltaTime * pitchSensitivity * mouseState.y;
            }
            if (mouseState.rightButton) {
                glm::vec3 right = math::getRight(camera->view);
                origin += deltaTime * mouseState.x * right;
                origin += deltaTime * mouseState.y * glm::cross(right, math::up);
            }
        }


        radius += deltaTime * zoomSensitivity * mouseState.scrollWheelValue;
        constexpr float radiusLimit = 0.1f;
        radius = std::max(radius, radiusLimit);

        constexpr float pitchLimit = 0.1f;
        pitch = std::min(std::max(pitch, pitchLimit), math::PI - pitchLimit);
        if (yaw > math::PI) {
            yaw -= math::TWO_PI;
        }
        else if (yaw < -math::PI) {
            yaw += math::TWO_PI;
        }

        glm::vec3 posOnSphere = {
            sinf(pitch) * cosf(yaw),
            cosf(pitch),
            sinf(pitch) * sinf(yaw),
        };
        posOnSphere *= radius;
        setCameraView(*camera, glm::lookAt(posOnSphere + origin, origin, math::up));
    }

    void setCameraView(Camera& camera, const glm::mat4& view)
    {
        camera.view = view;
        camera.invView = glm::inverse(camera.view);
    }
}
