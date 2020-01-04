#pragma once
#include "pch.h"
#include "bdrMath.h"


namespace bdr
{
    struct Camera
    {
        DirectX::SimpleMath::Matrix view;
        DirectX::SimpleMath::Matrix projection;
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
        float pitch = DirectX::XM_PIDIV2;
        float yaw = DirectX::XM_PIDIV2;
        float radius = 1.0f;
        DirectX::SimpleMath::Vector3 origin;
    private:
        Camera* camera = nullptr;
        DirectX::Mouse::ButtonStateTracker tracker;
    };
}

