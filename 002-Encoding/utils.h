#pragma once

// ウィンドウにワイド文字のテキストを表示する
// テキストに改行(\n)を含めることができる
extern void MyTextOut(HDC hdc, int x, int y, LPCWSTR wstr);

// ウィンドウにテキストを表示する
// テキストに改行(\n)を含めることができる
// テキストがUTF-8かShift_JISか自動判定する
extern void MyTextOut(HDC hdc, int x, int y, const char* str);
