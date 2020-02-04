#pragma once
#include "pch.h"
#include "Core/bdrMath.h"


namespace bdr
{
    struct Camera
    {
        glm::mat4 view;
        glm::mat4 invView;
        glm::mat4 projection;
    };

    class ICameraController
    {
    public:
        virtual void update(const DirectX::Keyboard::State& kbState, const DirectX::Mouse::State& mouseState, const float deltaTime) = 0;
    };

    class OrbitCameraController : ICameraController
    {
    public:
        inline void setCamera(Camera* newCamera) { camera = newCamera; };

        void update(const DirectX::Keyboard::State& kbState, const DirectX::Mouse::State& mouseState, const float deltaTime) override final;

        float yawSensitivity = 0.75f;
        float pitchSensitivity = 0.5f;
        float zoomSensitivity = 0.1f;
        float pitch = math::HALF_PI;
        float yaw = math::HALF_PI;
        float radius = 1.0f;
        glm::vec3 origin;
    private:
        Camera* camera = nullptr;
        DirectX::Mouse::ButtonStateTracker tracker;
    };

    void setCameraView(Camera& camera, const glm::mat4& view);
}

