#pragma once

#include <windows.h>
#include <string>

extern void LoadSettings(HWND hWnd);
extern void SaveSettings(HWND hWnd);
extern void MyDrawText(HDC hdc, int x, int y, LPCWSTR text);