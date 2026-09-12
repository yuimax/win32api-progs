#include "utils.h"

// INIファイルのパス
static WCHAR IniFilePath[MAX_PATH];

// INIファイルのメインセクション名
static WCHAR MainSection[] = L"MainWindow";

// WinMain() が実行される前に IniFilePath[] を初期化する
struct Initializer {
	Initializer() {
		WCHAR exePath[MAX_PATH];
		GetModuleFileName(NULL, exePath, MAX_PATH);

		WCHAR drive[_MAX_DRIVE];
		WCHAR dir[_MAX_DIR];
		WCHAR fname[_MAX_FNAME];
		WCHAR ext[_MAX_EXT];
		_wsplitpath_s(exePath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
		swprintf_s(IniFilePath, L"%s%s%s.ini", drive, dir, fname);
	}
};

static Initializer initializer;

// 整数を保存する
static void SaveInt(LPCWSTR section, LPCWSTR key, int value)
{
	WCHAR buffer[50];	// 整数の文字列化に十分な長さを確保
	swprintf_s(buffer, L"%d", value);
	WritePrivateProfileString(section, key, buffer, IniFilePath);
}

// 整数を復元する
static int LoadInt(LPCWSTR section, LPCWSTR key, int defaultValue)
{
	return GetPrivateProfileInt(section, key, defaultValue, IniFilePath);
}

// 設定の保存
void SaveSettings(HWND hWnd)
{
	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	if (!GetWindowPlacement(hWnd, &wp)) return;

	SaveInt(MainSection, L"X", wp.rcNormalPosition.left);
	SaveInt(MainSection, L"Y", wp.rcNormalPosition.top);
	SaveInt(MainSection, L"Width", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
	SaveInt(MainSection, L"Height", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
}

// 設定の復元
void LoadSettings(HWND hWnd)
{
	int x = LoadInt(MainSection, L"X", 0);
	int y = LoadInt(MainSection, L"Y", 0);
	int cx = LoadInt(MainSection, L"Width", 640);
	int cy = LoadInt(MainSection, L"Height", 480);

	SetWindowPos(hWnd, NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
}

// 改行("\n")を含むテキストを表示する
void MyDrawText(HDC hdc, int x, int y, LPCWSTR text)
{
	RECT rc = { x, y, 0, 0 }; // left, top, right, bottom
	DrawText(hdc, text, -1, &rc, DT_NOCLIP);

	// メモ：
	//	第2引数のtextは改行コードが有効（TextOut() だと改行できない）
	//	第3引数は文字数で、ここに-1を指定すると自動計算する
	//	第4引数は表示範囲で、書式が DT_NOCLIP の場合は rc.right と rc.bottom を無視
	//	第5引数は書式で、右詰めやセンタリングなどいろいろ指定できる
}
