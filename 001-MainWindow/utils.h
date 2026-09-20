#pragma once

// ウィンドウにワイド文字のテキストを表示する
// テキストに改行(\n)を含めることができる
extern void MyTextOut(HDC hdc, int x, int y, const WCHAR* wstr);
