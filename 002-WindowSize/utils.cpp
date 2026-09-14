#include "utils.h"

// 設定ファイルのパス
static WCHAR ConfigFilePath[MAX_PATH];

// 設定ファイルのメインセクション名
static WCHAR MainSection[] = L"MainWindow";

// WinMain() が実行される前に ConfigFilePath[] を初期化する
struct Initializer {
	Initializer() {
		WCHAR exePath[MAX_PATH];
		GetModuleFileName(NULL, exePath, MAX_PATH);

		WCHAR drive[_MAX_DRIVE];
		WCHAR dir[_MAX_DIR];
		WCHAR fname[_MAX_FNAME];
		WCHAR ext[_MAX_EXT];
		_wsplitpath_s(exePath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
		MySprintf(ConfigFilePath, L"%s%s%s.ini", drive, dir, fname);
	}
};

static Initializer initializer;

// 整数を保存する
static void SaveInt(LPCWSTR section, LPCWSTR key, int value)
{
	WCHAR buffer[50] = { 0 };	// 整数の文字列化に十分な長さ
	MySprintf(buffer, L"%d", value);
	WritePrivateProfileString(section, key, buffer, ConfigFilePath);
}

// 整数を復元する
static int LoadInt(LPCWSTR section, LPCWSTR key, int defaultValue)
{
	return GetPrivateProfileInt(section, key, defaultValue, ConfigFilePath);
}

// INIファイルに設定を保存する
void SaveSettings(HWND hWnd)
{
	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	GetWindowPlacement(hWnd, &wp);

	RECT& rc = wp.rcNormalPosition;

	SaveInt(MainSection, L"X", rc.left);
	SaveInt(MainSection, L"Y", rc.top);
	SaveInt(MainSection, L"Width", Width(rc));
	SaveInt(MainSection, L"Height", Height(rc));
}

// INIファイルから設定を読み出す
void LoadSettings(HWND hWnd)
{
	RECT rc;
	GetWindowRect(hWnd, &rc);

	int x = LoadInt(MainSection, L"X", rc.left);
	int y = LoadInt(MainSection, L"Y", rc.top);
	int cx = LoadInt(MainSection, L"Width", Width(rc));
	int cy = LoadInt(MainSection, L"Height", Height(rc));

	SetWindowPos(hWnd, NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
}

// ウィンドウにテキストを表示する
// テキストに改行(\n)を含めることができる
void MyTextOut(HDC hdc, int x, int y, LPCWSTR text)
{
	RECT rc = { x, y, 0, 0 }; // left, top, right, bottom
	DrawText(hdc, text, -1, &rc, DT_NOCLIP | DT_NOPREFIX);

	// メモ： DrawText()について
	//	第2引数のtext内では改行コード(\n)が有効
	//	またtext内の文字'&'は「次の文字を下線で修飾する」という意味になる
	//	'&'を普通の文字として表示するには、書式に DT_NOPREFIX を含める
	//	第3引数は文字数で、-1を指定すると自動計算する
	//	第4引数は表示範囲で、書式に DT_NOCLIP がある場合は rc.right と rc.bottom を無視
	//	第5引数は書式で、右詰めやセンタリングなどいろいろ
}
