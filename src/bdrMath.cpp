#include "pch.h"
#include "bdrMath.h"


using namespace DirectX::SimpleMath;

namespace bdr
{
    Matrix getMatrixFromTransform(const Transform& transform)
    {
        Matrix localTransform = Matrix::Identity;
        if (transform.mask & TransformType::Scale) {
            localTransform *= Matrix::CreateScale(transform.scale);
        }
        if (transform.mask & TransformType::Rotation) {
            localTransform = Matrix::Transform(localTransform, transform.rotation);
        }
        if (transform.mask & TransformType::Translation) {
            localTransform *= Matrix::CreateTranslation(transform.translation);
        }
        return localTransform;
    }
}