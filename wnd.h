#pragma once

class wnd 
{
public:
	HWND hWnd; // handle for the window

	/*int SCREEN_WIDTH = 1280;
	int SCREEN_HEIGHT = 720;*/

	void CreateWnd(HINSTANCE hInstance);
	void GetDesktopResolution(int& horizontal, int& vertical);

	wnd()
	{
		// GetDesktopResolution(DESKTOP_SCREEN_WIDTH, DESKTOP_SCREEN_HEIGHT);
	}
};

extern wnd* window;
