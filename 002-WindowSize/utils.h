#pragma once

#include <windows.h>
#include <stdio.h>	// for swprintf_s()
#include <fstream>
#include "../include/json.hpp"

extern void LoadSettings(HWND hWnd);
extern void SaveSettings(HWND hWnd);
extern void MyDrawText(HDC hdc, int x, int y, LPCWSTR text);