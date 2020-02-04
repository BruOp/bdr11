#include "pch.h"

#include "View.h"
#include "Camera.h"
#include "Graphics/Renderer.h"


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
                camera->view,
                camera->projection,
            };
            // Since we've transposed the matrices
            viewConstants.VP = viewConstants.perspectiveTransform * viewConstants.viewTransform;
            viewConstants.cameraPos = math::getTranslation(camera->invView);
            renderer->viewCB.copyToGPU(context, viewConstants);
            context->VSSetConstantBuffers(0u, 1u, &renderer->viewCB.buffer);
            context->PSSetConstantBuffers(0u, 1u, &renderer->viewCB.buffer);
        }
        else if (view.type == ViewType::Light) {
            // TODO;
        }
    }
}