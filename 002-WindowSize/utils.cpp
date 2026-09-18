#include <windows.h>
#include <vector>

// 文字列がUTF-8かどうか判定する
static BOOL IsValidUtf8(const char* str)
{
	return MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str, -1, nullptr, 0) > 0;
}

// 文字列をstd::vector<WCHAR>に変換する
static std::vector<WCHAR> ToWideChar(const char* str)
{
	// 元の文字列がUTF-8でなければcp932(Shift_JIS)とみなす
	int codepage = IsValidUtf8(str) ? CP_UTF8 : 932;

	// ワイド文字に変換した場合の文字数を得る（末尾の'\0'を含む）
	int wlen = MultiByteToWideChar(codepage, 0, str, -1, nullptr, 0);

	// バッファを確保し、strをワイド文字に変換して書き込む（末尾の'\0'を含む）
	std::vector<WCHAR> wbuf(wlen);
	MultiByteToWideChar(codepage, 0, str, -1, &wbuf[0], wlen);

	return wbuf;
}

// ウィンドウにテキストを表示する
// テキストに改行(\n)を含めることができる
void MyTextOut(HDC hdc, int x, int y, const char* str)
{
	// テキストをワイド文字に変換する
	auto wbuf = ToWideChar(str);

	// Win32APIのDrawText()を呼び出し、テキストを画面に表示する
	RECT rc = { x, y, 0, 0 }; // left, top, right, bottom
	DrawText(hdc, &wbuf[0], (int)wbuf.size() - 1, &rc, DT_NOCLIP | DT_NOPREFIX);

	// メモ： DrawText()について
	//	第2引数は表示するテキストで、ワイド文字列（WCHARへのポインタ）を指定する
	//	テキスト内では改行コード(\n)が有効
	//	テキスト内の文字'&'は「次の文字を下線で修飾する」という意味になる
	//	'&'を普通の文字として表示するには、書式に DT_NOPREFIX を含める
	//	第3引数は末尾の'\0'を除く文字数で、-1を指定すると自動計算する
	//	第4引数は表示範囲で、書式に DT_NOCLIP がある場合は rc.right と rc.bottom を無視
	//	第5引数は書式で、右詰めやセンタリングなどいろいろ

	// メモ： DrawTextA() について
	//	DrawTextA() という別のWin32APIを使うと、
	//	WCHAR変換は不用で、テキストを char* str のままAPIに渡せる
	//	ただし日本語データはShift_JISに統一する必要がある
	//	Shift_JISに統一するより、WCHARに変換するほうが簡単である
}

//////////////////////////////////////////////// 002-WindowSize で追加

#include <string>
#include <fstream>
#include <iterator>
#include "utils.h"
#include "../lib/mytree.hpp"

using StreamIt = std::istreambuf_iterator<char>;

// 設定ファイルのパス
//	メモ：パスに日本語が含まれる可能性もあるので WCHAR[] 型が推奨される。
//	char[] 型を使う場合、ファイル名はShift_JISでなければならない。UTF-8は使えない。
static WCHAR ConfigFilePath[MAX_PATH];

// WinMain() が実行される前に ConfigFilePath[] を初期化する
struct Initializer {
	Initializer() {
		WCHAR exePath[MAX_PATH];
		GetModuleFileName(NULL, exePath, MAX_PATH);
		WCHAR drive[_MAX_DRIVE];
		WCHAR dir[_MAX_DIR];
		WCHAR fname[_MAX_FNAME];
		WCHAR ext[_MAX_EXT];
		_wsplitpath_s(exePath, drive, dir, fname, ext);
		MyWSprintf(ConfigFilePath, L"%s%s%s.xml", drive, dir, fname);
	}
};

// グローバルな変数は WinMain() に先行して初期化される
static Initializer init;

// 最も近いモニターのワークエリアを取得し、
// 四角形(x, y, x+cx, y+cy)がその中に収まるように(x, y)を書き換える
static void MoveToSafeLocation(int& x, int& y, int cx, int cy)
{
	RECT rc{ x, y, x + cx, y + cy };
	HMONITOR hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
	if (hMonitor) {
		MONITORINFO mi = { sizeof(MONITORINFO) };
		if (GetMonitorInfo(hMonitor, &mi)) {
			RECT area = mi.rcWork;
			x = min(max(x, area.left), area.right - cx);
			y = min(max(y, area.top), area.bottom - cy);
		}
	}
}

// 設定の保存
void SaveSettings(HWND hWnd)
{
	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	GetWindowPlacement(hWnd, &wp);
	RECT& rc = wp.rcNormalPosition;

	auto config = MyTree::Create("config");
	auto section = config->addChild("MainWindow");
	section->addChild("X", rc.left);
	section->addChild("Y", rc.top);
	section->addChild("Width", Width(rc));
	section->addChild("Height", Height(rc));
	section->addChild("テストキー", "テスト値");

	std::ofstream file(ConfigFilePath);	// VC++ではパスにワイド文字列が使える
	if (file.is_open()) {
		file << config->ToString();
	}
	else {
		MessageBox(hWnd, L"設定ファイルに書き込めません", L"Error", MB_OK);
	}
}

// 設定の復元
void LoadSettings(HWND hWnd)
{
	RECT rc;
	GetWindowRect(hWnd, &rc);

	int x = rc.left;
	int y = rc.top;
	int cx = Width(rc);
	int cy = Height(rc);

	std::ifstream file(ConfigFilePath);	// VC++ではパスにワイド文字列が使える
	if (file.is_open()) {
		auto xmlstr = std::string{ StreamIt(file), StreamIt() };
		auto config = TreeNode::FromString(xmlstr);
		if (config) {
			auto section = config->getChild("MainWindow");
			if (section) {
				// 保存データがあれば取り出す
				x = section->getInt("X", x);
				y = section->getInt("Y", y);
				cx = section->getInt("Width", cx);
				cy = section->getInt("Height", cy);
				// ウィンドウが画面内に収まるように位置を調整
				MoveToSafeLocation(x, y, cx, cy);
			}
		}
	}

	SetWindowPos(hWnd, NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
}
