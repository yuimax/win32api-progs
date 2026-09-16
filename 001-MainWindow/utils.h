#pragma once

#include <windows.h>
#include <cstdio>
#include <cstdarg>

// sprintf()と同じだがバッファーオーバーフローを回避する
// すべて出力できればTRUEを返し、出力を打ち切った場合はFALSEを返す
template <size_t N>
BOOL MySprintf(WCHAR (&buf)[N], LPCWSTR format, ...) {
    va_list args;
    va_start(args, format);
    int count = _vsnwprintf_s(buf, N, _TRUNCATE, format, args);
    va_end(args);
    return (count >= 0) ? TRUE : FALSE;
}

// ウィンドウにテキストを表示する
// テキストに改行(\n)を含めることができる
extern void MyTextOut(HDC hdc, int x, int y, LPCWSTR text);
