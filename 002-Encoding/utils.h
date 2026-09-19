#pragma once
#include <vector>
#include <string>

// ウィンドウにワイド文字のテキストを表示する
// テキストに改行(\n)を含めることができる
extern void MyTextOut(HDC hdc, int x, int y, LPCWSTR wstr);

// ウィンドウにテキストを表示する
// テキストに改行(\n)を含めることができる
// テキストがUTF-8かShift_JISか自動判定する
extern void MyTextOut(HDC hdc, int x, int y, const char* str);

// ファイルからすべてのデータを読み込む
// ファイル名に日本語が含まれてもよい
extern std::vector<char> ReadAllBytes(LPCWSTR filePath);

// ファイルからすべてのデータを読み込む
// ファイル名に日本語（Shift_JISまたはUTF-8）が含まれてもよい
extern std::vector<char> ReadAllBytes(const char* filePath);

// ファイルからすべてのテキストを読み込みUTF-8のstd::stringとして返す
// ファイル名に日本語が含まれてもよい
// ファイルデータがUTF-16LEやShift_JISならUTF-8に変換する
extern std::string ReadAllText(LPCWSTR filePath);

// ファイルからすべてのテキストを読み込みUTF-8のstd::stringとして返す
// ファイル名に日本語（Shift_JISまたはUTF-8）が含まれてもよい
// ファイルデータがUTF-16LEやShift_JISならUTF-8に変換する
extern std::string ReadAllText(const char* filePath);
