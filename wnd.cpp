#include "pch.h"
#include "wnd.h"

// Forward declaration of WndProc in main.cpp
extern LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void wnd::CreateWnd(HINSTANCE hInstance)
{
    WNDCLASSEX wc;

    // Clearing window class for use
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    // Filling needed information
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    // wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = L"WindowClass";

    // Register window class
    RegisterClassEx(&wc);
      
    // Using AdjustWindowRect to set an accurate size of the drawing area
    RECT wr = { 0,0,SCREEN_WIDTH, SCREEN_HEIGHT };
    AdjustWindowRect(&wr, WS_OVERLAPPED, FALSE);

    // Create the window and use the result as the handle
    hWnd = CreateWindowEx(NULL,
        L"WindowClass",
        L"PhysicsEngine",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,    // window flags
        0,  // the starting x and y positions should be 0
        0,
        wr.right - wr.left,  // width of window
        wr.bottom - wr.top,  // height of window
        NULL,
        NULL,
        hInstance,
        NULL);

    // Set the opacity and transparency color key
    // SetLayeredWindowAttributes(window->hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
}

void wnd::GetDesktopResolution(int& horizontal, int& vertical)
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

wnd* window = new wnd();