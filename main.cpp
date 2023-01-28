#include "Engine.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to call CoInitialize.");
        return -1;
    }
    
    Engine engine;

    if (engine.Initialize(hInstance, "PhysicsEngine", "WindowClass1", 1200, 1000))
    {
        while (engine.ProcessMessages() == true)
        {
            engine.RenderFrame();
            engine.Update();
        }
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    engine.CleanD3D();

    return 0;
}