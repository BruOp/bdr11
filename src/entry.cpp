#include "pch.h"
#include "entry.h"

using namespace DirectX;

namespace bdr
{
    BaseGame::BaseGame()
    {
        g_renderer.deviceResources->RegisterDeviceNotify(this);
    }

    int BaseGame::run()
    {
        setup();
        MSG msg = {};
        while (WM_QUIT != msg.message) {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else {
                tick();
            }
        }

        shutdown();

        CoUninitialize();

        return (int)msg.wParam;
    }

    void BaseGame::shutdown()
    { }

    void BaseGame::initialize(AppConfig& appConfig)
    {
        OutputDebugString(L"Starting App!\n");

        Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);
        ASSERT_SUCCEEDED(InitializeWinRT);

        HINSTANCE hInstance = GetModuleHandle(0);
        FNRes result = window.initialize(hInstance, WndProc, appConfig.width, appConfig.height);
        ASSERT(result == FNRes::SUCCESS, "Window failed to init");

        window.setUserData(this);
        RECT rc = window.getClientRect();

        g_renderer.setWindow(window, rc.right - rc.left, rc.bottom - rc.top);
    }



    void BaseGame::processMessage(UINT message, WPARAM wParam, LPARAM lParam)
    {
        Keyboard::ProcessMessage(message, wParam, lParam);
        Mouse::ProcessMessage(message, wParam, lParam);
    }

    BDRid BaseGame::createScene()
    {
        BDRid sceneIdx = sceneList.size();
        sceneList.emplace_back();
        return sceneIdx;
    }

    void BaseGame::onActivated() { };
    void BaseGame::onDeactivated() { };
    void BaseGame::onSuspending() { };
    void BaseGame::onResuming() { };

    // Windows procedure
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        PAINTSTRUCT ps;
        HDC hdc;

        static bool s_in_sizemove = false;
        static bool s_in_suspend = false;
        static bool s_minimized = false;
        static bool s_fullscreen = false;
        // TODO: Set s_fullscreen to true if defaulting to fullscreen.

        auto game = reinterpret_cast<bdr::BaseGame*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

        switch (message) {
        case WM_PAINT:
            if (s_in_sizemove && game) {
                game->tick();
            }
            else {
                hdc = BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
            }
            break;

        case WM_MOVE:
            if (game) {
                onWindowMove();
            }
            break;

        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED) {
                if (!s_minimized) {
                    s_minimized = true;
                    if (!s_in_suspend && game)
                        game->onSuspending();
                    s_in_suspend = true;
                }
            }
            else if (s_minimized) {
                s_minimized = false;
                if (s_in_suspend && game)
                    game->onResuming();
                s_in_suspend = false;
            }
            else if (!s_in_sizemove && game) {
                onWindowResize(LOWORD(lParam), HIWORD(lParam));
            }
            break;

        case WM_ENTERSIZEMOVE:
            s_in_sizemove = true;
            break;

        case WM_EXITSIZEMOVE:
            s_in_sizemove = false;
            if (game) {
                RECT rc;
                GetClientRect(hWnd, &rc);
                onWindowResize(rc.right - rc.left, rc.bottom - rc.top);
            }
            break;

        case WM_GETMINMAXINFO:
        {
            auto info = reinterpret_cast<MINMAXINFO*>(lParam);
            info->ptMinTrackSize.x = 320;
            info->ptMinTrackSize.y = 200;
        }
        break;

        case WM_ACTIVATEAPP:
            if (game) {
                if (wParam) {
                    game->onActivated();
                }
                else {
                    game->onDeactivated();
                }
            }
            break;

        case WM_POWERBROADCAST:
            switch (wParam) {
            case PBT_APMQUERYSUSPEND:
                if (!s_in_suspend && game)
                    game->onSuspending();
                s_in_suspend = true;
                return TRUE;

            case PBT_APMRESUMESUSPEND:
                if (!s_minimized) {
                    if (s_in_suspend && game)
                        game->onResuming();
                    s_in_suspend = false;
                }
                return TRUE;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_INPUT:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEHOVER:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            game->processMessage(message, wParam, lParam);
            break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            game->processMessage(message, wParam, lParam);

            if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000) {
                // Implements the classic ALT+ENTER fullscreen toggle
                if (s_fullscreen) {
                    SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                    SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

                    int width = 800;
                    int height = 600;
                    if (game) {
                        width = game->appConfig.width;
                        height = game->appConfig.height;
                    }

                    ShowWindow(hWnd, SW_SHOWNORMAL);

                    SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
                }
                else {
                    SetWindowLongPtr(hWnd, GWL_STYLE, 0);
                    SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

                    SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

                    ShowWindow(hWnd, SW_SHOWMAXIMIZED);
                }

                s_fullscreen = !s_fullscreen;
            }
            break;

        case WM_MENUCHAR:
            // A menu is active and the user presses a key that does not correspond
            // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
            return MAKELRESULT(0, MNC_CLOSE);
        }

        return DefWindowProc(hWnd, message, wParam, lParam);
    }

}

void ExitGame()
{
    PostQuitMessage(0);
}