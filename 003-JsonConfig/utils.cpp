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

//////////////////////////////////////////////// 003-JsonConfig で追加

#include <fstream>
#include "../lib/json.hpp"

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
		_wsplitpath_s(exePath, drive, dir, fname, ext);
		MySprintf(ConfigFilePath, L"%s%s%s.json", drive, dir, fname);
	}
};

static Initializer initializer;

// 設定の保存
void SaveSettings(HWND hWnd)
{
	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	GetWindowPlacement(hWnd, &wp);

	RECT& rc = wp.rcNormalPosition;
	json config, section;
	section["X"] = rc.left;
	section["Y"] = rc.top;
	section["Width"] = Width(rc);
	section["Height"] = Height(rc);
	config["MainWindow"] = section;

	std::ofstream file(ConfigFilePath);
	if (!file.is_open()) {
		MessageBox(hWnd, L"JSONファイルに書き込めません", L"エラー", MB_OK);
		return;
	}

	file << config.dump(4); // 4タブインデントで整形して保存
}

// 最も近いモニターのワークエリアを取得し、
// 四角形(x, y, x+cx, y+cy)がワークエリアに収まるように調整した位置(x, y)を返す
static POINT GetSafeLocation(int x, int y, int cx, int cy)
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
	return POINT{ x, y };
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
		json config = json::parse(file, nullptr, false);
		if (config.is_discarded()) {
			MessageBox(hWnd, L"JSONファイルが不正です", L"エラー", MB_OK);
			return;
		}
		json section = config["MainWindow"];
		if (section != nullptr) {
			// 保存データがあれば取り出す
			x = section.value("X", x);
			y = section.value("Y", y);
			cx = section.value("Width", cx);
			cy = section.value("Height", cy);
			// ウィンドウが画面内に収まるように位置を調整
			POINT pt = GetSafeLocation(x, y, cx, cy);
			x = pt.x;
			y = pt.y;
		}
	}

	SetWindowPos(hWnd, NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
}
