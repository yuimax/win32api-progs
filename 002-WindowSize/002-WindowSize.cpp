// 002-WindowSize.cpp

#include "framework.h"

// ウィンドウの初期サイズ
constexpr int INIT_WINDOW_WIDTH = 640;
constexpr int INIT_WINDOW_HEIGHT = 480;

// ★ウィンドウの最小サイズ・最大サイズ
constexpr int MIN_WINDOW_WIDTH = 400;
constexpr int MIN_WINDOW_HEIGHT = 300;
constexpr int MAX_WINDOW_WIDTH = 1000;
constexpr int MAX_WINDOW_HEIGHT = 750;

// グローバル変数
HINSTANCE hInst;
WCHAR WindowTitle[] = L"win32api 002-WindowSize";
WCHAR WindowClassName[] = L"WinClass 002-WindowSize";

// プロトタイプ宣言
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// エントリポイント
int APIENTRY wWinMain(
	_In_     HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_     LPWSTR lpCmdLine,
	_In_     int nCmdShow)
{
	// 引数未使用の警告を回避
	(void)hPrevInstance;
	(void)lpCmdLine;

	// ウィンドウクラスを登録する
	MyRegisterClass(hInstance);

	// ウィンドウを作成する
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	// hInstanceをグローバル変数に保存しておく
	hInst = hInstance;

	// メッセージループ
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

// メインウィンドウのウィンドウクラスを登録
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) }; // 先頭要素のみ指定、残りを0で埋める

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = WindowClassName;

	return RegisterClassEx(&wcex);
}

// メインウィンドウの作成
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd = CreateWindowEx(
		0,						// dwExStyle
		WindowClassName,		// lpClassName
		WindowTitle,			// lpWindowName
		WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,	// ★ WS_MAXMIZEBOX を無効化
		CW_USEDEFAULT,			// X
		CW_USEDEFAULT,			// Y
		INIT_WINDOW_WIDTH,		// nWidth
		INIT_WINDOW_HEIGHT,		// nHeight
		nullptr,				// hWndParent
		nullptr,				// hMenu
		hInstance,				// hInstance
		nullptr					// lpParam
	);

	if (!hWnd)
	{
		return FALSE;
	}

	LoadSettings(hWnd);	// ★設定の復元を追加

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

////////////////////////////////////////// メインウィンドウのメッセージ処理

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {

	case WM_GETMINMAXINFO:
	{
		// ★ウィンドウの最小最大サイズの確認が必要なときここに来る

		MINMAXINFO* pMinMaxInfo = (MINMAXINFO*)lParam;

		// 最小サイズの制限（不用な場合は次の2行をコメントアウト）
		pMinMaxInfo->ptMinTrackSize.x = MIN_WINDOW_WIDTH;
		pMinMaxInfo->ptMinTrackSize.y = MIN_WINDOW_HEIGHT;

		// 最大サイズの制限（不用な場合は次の2行をコメントアウト）
		pMinMaxInfo->ptMaxTrackSize.x = MAX_WINDOW_WIDTH;
		pMinMaxInfo->ptMaxTrackSize.y = MAX_WINDOW_HEIGHT;

		break;
	}

	case WM_PAINT:
	{
		// 画面を更新すべきタイミングでこのメッセージが来る

		// 前準備
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		// 背景を塗りつぶす
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		// ★ウィンドウのサイズを表示
		RECT rc;
		GetWindowRect(hWnd, &rc);
		WCHAR text[100];
		MySprintf(text, L"Window Size = (%d, %d)", Width(rc), Height(rc));
		MyTextOut(hdc, 8, 8, text);

		// 後始末
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_SETCURSOR:
	{
		// マウスカーソルを変えるべきタイミングでこのメッセージが来る

		// マウスカーソルの下に何があるかチェック
		WORD hitTest = LOWORD(lParam);

		// クライアント領域の場合は、標準の矢印カーソルにする
		if (hitTest == HTCLIENT)
		{
			HCURSOR hCursor = LoadCursor(0, IDC_ARROW);
			SetCursor(hCursor);
			return TRUE;	// 自分でカーソルを設定した場合はTRUEを返す
		}

		// それ以外（枠線上など）の場合は、デフォルト処理にまかせる
		break;
	}

	case WM_DESTROY:
	{
		// ウィンドウが破棄されたとき、このメッセージが来る

		// ★ウィンドウサイズを保存する
		SaveSettings(hWnd);

		// メッセージループを終了する
		PostQuitMessage(0);
		break;
	}

	} // end of switch (message)

	return DefWindowProc(hWnd, message, wParam, lParam);
}
