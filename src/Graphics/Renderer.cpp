#include "pch.h"

#include <string>
#include <fstream>

#include "Renderer.h"
#include "GPUBuffer.h"
#include "d3dcompiler.h"


namespace bdr
{
}

void onWindowResize(bdr::Renderer& renderer, int width, int height)
{
    renderer.hasWindowSizeChanged(width, height);
}

void onWindowMove(bdr::Renderer& renderer)
{
    RECT r = renderer.getOutputSize();
    renderer.hasWindowSizeChanged(r.right, r.bottom);
}
