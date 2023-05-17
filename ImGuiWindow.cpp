#include "ImGuiWindow.h"

bool ImGuiWindow::Initialize(HWND hWnd, ID3D11Device* dev, ID3D11DeviceContext* devcon)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.LogFilename = nullptr;
    io.WantSaveIniSettings = true;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowMinSize = ImVec2(375, 200);
    style.WindowTitleAlign = ImVec2(0.50f, 0.50f); // Title
    style.WindowPadding = ImVec2(7, 7);
    // style.WindowRounding = 9.0f;
    style.FrameRounding = 1;
    // style.FramePadding = ImVec2(17.0f, 4.0f);
    // style.TabRounding = 9.0f;
    // style.GrabRounding = 3;
    // style.GrabMinSize = 15.0f;
    // style.ScrollbarSize = 17.0f;
    // style.ItemSpacing = ImVec2(7, 4);
    // style.ItemInnerSpacing = ImVec2(10, 5);
    style.AntiAliasedLines = true;

    if (!ImGui_ImplWin32_Init(hWnd))
    {
        ErrorLogger::Log("ImGui_ImplWin32_Init, failed to initialize ImGui.");
        return false;
    }

    if (!ImGui_ImplDX11_Init(dev, devcon))
    {
        ErrorLogger::Log("ImGui_ImplDX11_Init, failed to initialize ImGui.");
        return false;
    }

    return true;
}

void ImGuiWindow::ShutDown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiWindow::BeginRender()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiWindow::EndRender()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
