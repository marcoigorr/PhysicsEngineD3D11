#include "pch.h"

#include "wnd.h"
#include "dx11.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    // Fullscreen transparent window creation
    if (!window->CreateWnd(hInstance, "PhysicsEngine", "WindowClass1", 1600, 1200))
        return 0;    

    // Initialize DirectX11
    d3d11->InitD3D(window->_hWnd);

    // Display created window
    ShowWindow(window->_hWnd, nCmdShow);

    // Main loop
    while (window->ProcessMessages() == true)
    {
        d3d11->RenderFrame();
    }

    // Clean up DirectXand COM
    d3d11->CleanD3D();

    return 0;
}