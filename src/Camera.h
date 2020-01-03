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

    //class ICameraController
    //{
    //public:
    //    void update()
    //};

    //class OrbitCameraController
    //{
    //public:
    //    OrbitCameraController(Camera* camera);

    //private:
    //    Camera* camera;
    //};
}

