#include "utils.h"

// EXEファイルと同じディレクトリで、ファイル名の拡張子を ".ini" に変えたパスを得る
static std::wstring GetIniFilePath() {
	WCHAR exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);

	WCHAR drive[_MAX_DRIVE];
	WCHAR dir[_MAX_DIR];
	WCHAR fname[_MAX_FNAME];
	WCHAR ext[_MAX_EXT];
	_wsplitpath_s(exePath,
		drive, _MAX_DRIVE,
		dir, _MAX_DIR,
		fname, _MAX_FNAME,
		ext, _MAX_EXT);

	WCHAR iniFilePath[MAX_PATH];
	swprintf_s(iniFilePath, L"%s%s%s.ini", drive, dir, fname);

	return std::wstring(iniFilePath);
}

// INIファイルのパス
static std::wstring MyIniFilePath = GetIniFilePath();

// 整数を保存する
static void SaveInt(LPCWSTR section, LPCWSTR key, int value) {
	WritePrivateProfileStringW(
		section,
		key,
		std::to_wstring(value).c_str(),
		MyIniFilePath.c_str()
	);
}

// 整数を復元する
static int LoadInt(LPCWSTR section, LPCWSTR key, int defaultValue) {
	return GetPrivateProfileIntW(
		section,
		key,
		defaultValue,
		MyIniFilePath.c_str()
	);
}

// 設定ファイルのセクション名
static const LPCWSTR MainSection = L"MainWindow";

// 設定の保存
void SaveSettings(HWND hWnd) {
	RECT rc;
	GetWindowRect(hWnd, &rc);

	SaveInt(MainSection, L"X", rc.left);
	SaveInt(MainSection, L"Y", rc.top);
	SaveInt(MainSection, L"Width", rc.right - rc.left);
	SaveInt(MainSection, L"Height", rc.bottom - rc.top);
}

// 設定の復元
void LoadSettings(HWND hWnd) {
	int x = LoadInt(MainSection, L"X", 0);
	int y = LoadInt(MainSection, L"Y", 0);
	int cx = LoadInt(MainSection, L"Width", 640);
	int cy = LoadInt(MainSection, L"Height", 480);

	SetWindowPos(hWnd, NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
}
