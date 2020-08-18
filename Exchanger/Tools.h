#pragma once

// STL
#include <algorithm>
#include <string> 
#include <iostream>
#include <iomanip> 
#include <fstream>
#include <ctime>
#include <map>
#include <stdlib.h>
// OpenGL
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring& wstr)
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an ANSI string to a wide Unicode String
std::wstring ansi2unicode(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}