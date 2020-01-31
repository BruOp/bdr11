#pragma once
#include "pch.h"

namespace bdr
{
    class BaseGame;

    class Window
    {
    public:
        FNRes initialize(_In_ HINSTANCE hInstance, _In_ WNDPROC WndProc, const int width, const int height);

        inline void setUserData(const BaseGame* game) const
        {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(game));
        };

        inline RECT getClientRect()
        {
            GetClientRect(hwnd, &rect);
            return rect;
        };

        inline operator HWND() { return hwnd; }
    private:
        HWND hwnd;
        RECT rect;
    };
}

