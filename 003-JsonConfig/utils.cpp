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
		MySprintf(ConfigFilePath, L"%s%s%s.json", drive, dir, fname);
	}
};

static Initializer initializer;

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
	RECT rc;
	GetWindowRect(hWnd, &rc);
	
	int x = rc.left;
	int y = rc.top;
	int cx = Width(rc);
	int cy = Height(rc);

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
