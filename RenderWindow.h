#pragma once
#include "ErrorLogger.h"

class WindowContainer;

class RenderWindow
{
public:
	~RenderWindow();

	bool CreateWnd(HINSTANCE hInstance, std::string sTitle, std::string sClass, int width, int height);
	void RegisterWindowClass(void);
	bool ProcessMessages();

	HWND GetHWND() const;

	HWND _hWnd = NULL; // handle for the window
	HINSTANCE _hInstance = NULL; // handle to application instance
	std::string _sWindowTitle = "";
	std::wstring _wWindowTitle = L"";
	std::string _sWindowClass = "";
	std::wstring _wWindowClass = L"";
	int _width = 0;
	int _height = 0;
};
