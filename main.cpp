#include "pch.h"

#include "wnd.h"
#include "dx11.h"

// Forward declare message handler from imgui_impl_win32.cpp
// extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    /* if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true; */

    switch (message)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    } break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Fullscreen transparent window creation
    window->CreateWnd(hInstance);

    // Initialize DirectX11
    d3d11->InitD3D(window->hWnd);

    // Display created window
    ShowWindow(window->hWnd, nCmdShow);

    MSG msg;

    // Main loop
    while (!(GetAsyncKeyState(VK_HOME)))
    {
        // Check to see if any messages are waiting in the queue
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                break;
        }

        d3d11->RenderFrame();
    }

    // Clean up DirectXand COM
    d3d11->CleanD3D();

    // Return this part of the WM_QUIT message to Windows
    return msg.wParam;
}