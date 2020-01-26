#include "pch.h"
#include "Window.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

namespace bdr
{
    FNRes Window::initialize(_In_ HINSTANCE hInstance, _In_ WNDPROC WndProc, const int width, const int height)
    {
        // Register class
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIconW(hInstance, L"IDI_ICON");
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszClassName = L"bdrWindowClass";
        wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");
        if (!RegisterClassExW(&wcex))
            return FNRes::FAILURE;

        // Create window
        rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        HWND hwnd = CreateWindowExW(0, L"bdrWindowClass", L"BDR Example", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance,
            nullptr);
        // TODO: Change to CreateWindowExW(WS_EX_TOPMOST, L"bdrWindowClass", L"bdr_v2", WS_POPUP,
        // to default to fullscreen.

        if (!hwnd)
            return FNRes::FAILURE;

        ShowWindow(hwnd, SW_SHOW);
        // TODO: Change nCmdShow to SW_SHOWMAXIMIZED to default to fullscreen.
        return FNRes::SUCCESS;
    }
}
