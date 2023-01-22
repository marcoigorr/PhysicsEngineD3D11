#include "WindowContainer.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_NCCREATE:
    {
        OutputDebugStringA("[+] The window was created! ");
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

bool RenderWindow::CreateWnd(HINSTANCE hInstance, std::string title, std::string wclass, int width, int height)
{
    _hInstance = hInstance;
    _width = width;
    _height = height;
    _sWindowTitle = title;
    _wWindowTitle = StringConverter::StringToWide(title);
    _sWindowClass = wclass;
    _wWindowClass = StringConverter::StringToWide(wclass);

    this->RegisterWindowClass();
      
    int centerScreenX = GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2;
    int centerScreenY = GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2;

    // Using AdjustWindowRect to set an accurate size of the drawing area
    RECT wr;
    wr.left = centerScreenX;
    wr.top = centerScreenY;
    wr.right = wr.left + width;
    wr.bottom = wr.top + height;
    AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

    // Create the window and use the result as the handle
    _hWnd = CreateWindowEx(NULL,
        _wWindowClass.c_str(),
        _wWindowTitle.c_str(),
        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,  // window flags
        wr.left,  // the starting x position
        wr.top,  // the starting y positions
        wr.right - wr.left,  // width of window
        wr.bottom - wr.top,  // height of window
        NULL,
        NULL,
        hInstance,
        NULL);

    if (_hWnd == NULL)
    {
        ErrorLogger::Log(GetLastError(), "CreateWindowEx Failed for window " + _sWindowTitle);
        return false;
    }

    // Show the window and set focus
    ShowWindow(_hWnd, SW_SHOW);
    SetForegroundWindow(_hWnd);
    SetFocus(_hWnd);
    
    return true;
}

RenderWindow::~RenderWindow()
{
    if (_hWnd != NULL)
    {
        UnregisterClass(_wWindowClass.c_str(), _hInstance);
        DestroyWindow(_hWnd);
    }
}

void RenderWindow::RegisterWindowClass(void)
{
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = _hInstance;
    wc.hIcon = NULL;
    wc.hIconSm = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = _wWindowClass.c_str();
    wc.cbSize = sizeof(WNDCLASSEX);           

    RegisterClassEx(&wc);
}

bool RenderWindow::ProcessMessages()
{
    // Handle windows messages
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    while (PeekMessage(&msg, _hWnd, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Check if window was closed
    if (msg.message == WM_NULL)
    {
        if (!IsWindow(_hWnd))
        {
            _hWnd = NULL; // message processing loop takes care of destroying this window
            UnregisterClass(_wWindowClass.c_str(), _hInstance);
            return false;
        }
    }

    return true;
}

HWND RenderWindow::GetHWND() const
{
    return _hWnd;
}
