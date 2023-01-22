#include "Engine.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    Engine engine;

    if (engine.Initialize(hInstance, "PhysicsEngine", "WindowClass1", 1920, 1080))
    {
        while (engine.ProcessMessages() == true)
        {
            engine.RenderFrame();
            engine.Update();
        }
    }

    engine.CleanD3D();

    return 0;
}