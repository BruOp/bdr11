#include "pch.h"

#include "View.h"
#include "Camera.h"
#include "Renderer.h"


namespace bdr
{
    void View::setCamera(const Camera* camera)
    {
        type = ViewType::Camera;
        perspectiveProvider.camera = camera;
    }

    void setConstants(Renderer* renderer, const View& view)
    {
        ASSERT(view.type != ViewType::Unknown, "Cannot set constants for unknown view");
        auto* context = renderer->getContext();

        if (view.type == ViewType::Camera) {
            const Camera* camera = view.getCamera();
            ViewConstants viewConstants{
                camera->view.Transpose(),
                camera->projection.Transpose(),
            };
            // Since we've transposed the matrices
            viewConstants.VP = viewConstants.perspectiveTransform * viewConstants.viewTransform;
            viewConstants.cameraPos = camera->view.Translation();
            renderer->viewCB.copyToGPU(context, viewConstants);
            context->VSSetConstantBuffers(0u, 1u, &renderer->viewCB.buffer);
        }
        else if (view.type == ViewType::Light) {
            // TODO;
        }
    }
}