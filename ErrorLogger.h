#pragma once
#include "StringConverter.h"
#include <Windows.h>
#include <comdef.h>

class ErrorLogger
{
public:
	static void Log(std::string message)
	{
		std::string errorMsg = "Error: " + message;
		MessageBoxA(NULL, errorMsg.c_str(), "Error", MB_ICONERROR);
	}
	static void Log(HRESULT hr, std::string message)
	{
		_com_error error(hr);
		std::wstring errorMsg = L"Error: " + StringConverter::StringToWide(message) + L"\n" + error.ErrorMessage();
		MessageBoxW(NULL, errorMsg.c_str(), L"Error", MB_ICONERROR);
	}
	static void Log(HRESULT hr, std::wstring message)
	{
		_com_error error(hr);
		std::wstring errorMsg = L"Error: " + message + L"\n" + error.ErrorMessage();
		MessageBoxW(NULL, errorMsg.c_str(), L"Error", MB_ICONERROR);
	}
};
