#include "utils.h"
using json = nlohmann::json;

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
		_wsplitpath_s(exePath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
		swprintf_s(ConfigFilePath, L"%s%s%s.json", drive, dir, fname);
	}
};

static Initializer initializer;

// 整数を保存する
static void SaveInt(LPCWSTR section, LPCWSTR key, int value)
{
	WCHAR buffer[50];	// 整数の文字列化に十分な長さを確保
	swprintf_s(buffer, L"%d", value);
	WritePrivateProfileString(section, key, buffer, ConfigFilePath);
}

// 整数を復元する
static int LoadInt(LPCWSTR section, LPCWSTR key, int defaultValue)
{
	return GetPrivateProfileInt(section, key, defaultValue, ConfigFilePath);
}

// 設定の保存
void SaveSettings(HWND hWnd)
{
	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	GetWindowPlacement(hWnd, &wp);

	json j;
	j["X"] = wp.rcNormalPosition.left;
	j["Y"] = wp.rcNormalPosition.top;
	j["Width"] = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	j["Height"] = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;

	std::ofstream file(ConfigFilePath);
	if (!file.is_open()) {
		MessageBox(hWnd, L"JSONファイルに書き込めません", L"エラー", MB_OK);
		return;
	}

	file << j.dump(4); // 4タブインデントで整形して保存
}

// 設定の復元
void LoadSettings(HWND hWnd)
{
	int x = 0;
	int y = 0;
	int cx = 640;
	int cy = 480;

	std::ifstream file(ConfigFilePath);
	if (file.is_open()) {
		json j;
		try {
			file >> j;
		}
		catch (...) {
			MessageBox(hWnd, L"JSONファイルが不正です", L"エラー", MB_OK);
			return;
		}
		x = j.value("X", x);
		y = j.value("Y", y);
		cx = j.value("Width", cx);
		cy = j.value("Height", cy);
	}

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
