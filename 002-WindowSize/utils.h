#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

// ウィンドウにテキストを表示する
// テキストに改行(\n)を含めることができる
extern void MyTextOut(HDC hdc, int x, int y, const char* str);

// sprintf()と同じだがバッファーオーバーフローを回避する
// すべて出力できればTRUEを返し、出力を打ち切った場合はFALSEを返す
template <size_t N>
BOOL MySprintf(char(&buf)[N], const char* format, ...) {
	va_list args;
	va_start(args, format);
	int count = _vsnprintf_s(buf, N, format, args);
	va_end(args);
	return (count >= 0) ? TRUE : FALSE;
}

// wsprintf()と同じだがバッファーオーバーフローを回避する
// すべて出力できればTRUEを返し、出力を打ち切った場合はFALSEを返す
template <size_t N>
BOOL MyWSprintf(WCHAR(&buf)[N], const WCHAR* format, ...) {
	va_list args;
	va_start(args, format);
	int count = _vsnwprintf_s(buf, N, format, args);
	va_end(args);
	return (count >= 0) ? TRUE : FALSE;
}

// RECT の幅を返す
inline int Width(const RECT& rc) { return rc.right - rc.left; }

// RECT の高さを返す
inline int Height(const RECT& rc) { return rc.bottom - rc.top; }

// Configファイルから設定を読み出す
extern void LoadSettings(HWND hWnd);

// Configファイルに設定を保存する
extern void SaveSettings(HWND hWnd);
