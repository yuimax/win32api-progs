#pragma once

// ウィンドウにワイド文字列のテキストを表示する
// テキストに改行(\n)を含めることができる
extern void MyTextOut(HDC hdc, int x, int y, LPCWSTR str);
