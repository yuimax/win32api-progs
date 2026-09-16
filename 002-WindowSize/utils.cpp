#include "utils.h"

//////////////////////////////////////////////// 001-MainWindow で追加

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

//////////////////////////////////////////////// 002-WindowSize で追加

// 設定ファイルのパス
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
		MySprintf(ConfigFilePath, L"%s%s%s.ini", drive, dir, fname);
	}
};

static Initializer initializer;

// 整数を保存する
static void SaveInt(LPCWSTR section, LPCWSTR key, int value)
{
	WCHAR buffer[100] = { 0 };
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

	SaveInt(L"MainWindow", L"X", rc.left);
	SaveInt(L"MainWindow", L"Y", rc.top);
	SaveInt(L"MainWindow", L"Width", Width(rc));
	SaveInt(L"MainWindow", L"Height", Height(rc));
}

// INIファイルから設定を読み出す
void LoadSettings(HWND hWnd)
{
	RECT rc;
	GetWindowRect(hWnd, &rc);

	int x = LoadInt(L"MainWindow", L"X", rc.left);
	int y = LoadInt(L"MainWindow", L"Y", rc.top);
	int cx = LoadInt(L"MainWindow", L"Width", Width(rc));
	int cy = LoadInt(L"MainWindow", L"Height", Height(rc));

	SetWindowPos(hWnd, NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
}
