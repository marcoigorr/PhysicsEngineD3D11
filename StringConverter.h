#pragma once
#include <string>

class StringConverter
{
public:
	static std::wstring StringToWide(std::string string)
	{
		std::wstring wString(string.begin(), string.end());
		return wString;
	}
};
