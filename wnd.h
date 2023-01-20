#pragma once

class wnd
{
public:
	wnd();
	~wnd();

	bool CreateWnd(HINSTANCE hInstance, std::string sTitle, std::string sClass, int width, int height);
	bool ProcessMessages();
	void GetDesktopResolution(int& horizontal, int& vertical);

	HWND _hWnd = NULL; // handle for the window
	HINSTANCE _hInstance = NULL; // handle to application instance
	std::string _sWindowTitle = "";
	std::wstring _wWindowTitle = L"";
	std::string _sWindowClass = "";
	std::wstring _wWindowClass = L"";
	int _width = 0;
	int _height = 0;
	int _NATIVE_WIDTH = 0;
	int _NATIVE_HEIGHT = 0;
};

extern wnd* window;
