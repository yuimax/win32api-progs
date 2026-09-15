#pragma once

#include <windows.h>
#include <stdio.h>	// for _vsnwprintf_s()
#include <stdarg.h> // for va_list, va_start, va_end

// sprintf()と同じだがバッファーオーバーフローを回避する
// すべて出力できればTRUEを返し、出力を打ち切った場合はFALSEを返す
template <size_t N>
BOOL MySprintf(WCHAR(&buf)[N], LPCWSTR format, ...) {
    va_list args;
    va_start(args, format);
    int count = _vsnwprintf_s(buf, N, _TRUNCATE, format, args);
    va_end(args);
    return (count >= 0) ? TRUE : FALSE;
}

// RECT の幅を返す
inline int Width(const RECT& rc) { return rc.right - rc.left; }

// RECT の高さを返す
inline int Height(const RECT& rc) { return rc.bottom - rc.top; }

// JSONファイルから設定を読み出す
extern void LoadSettings(HWND hWnd);

// JSONファイルに設定を保存する
extern void SaveSettings(HWND hWnd);

// ウィンドウにテキストを表示する
// テキストに改行(\n)を含めることができる
extern void MyTextOut(HDC hdc, int x, int y, LPCWSTR text);
