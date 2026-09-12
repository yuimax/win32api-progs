// 002-WindowSize.cpp

#include "framework.h"

// ウィンドウの初期サイズ
constexpr int INIT_WINDOW_WIDTH = 640;
constexpr int INIT_WINDOW_HEIGHT = 480;

// ★ウィンドウの最小サイズ・最大サイズ
constexpr int MIN_WINDOW_WIDTH = 400;
constexpr int MIN_WINDOW_HEIGHT = 300;
constexpr int MAX_WINDOW_WIDTH = 1024;
constexpr int MAX_WINDOW_HEIGHT = 768;

// グローバル変数
HINSTANCE hInst;
WCHAR szTitle[] = L"win32api 002-WindowSize";
WCHAR szWindowClass[] = L"WinClass 002-WindowSize";

// プロトタイプ宣言
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

// x64用エントリポイント
int APIENTRY wWinMain(
	_In_     HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_     LPWSTR lpCmdLine,
	_In_     int nCmdShow)
{
	hInst = hInstance;		// あとで使うのでグローバル変数に格納
	(void)hPrevInstance;	// 引数未使用の警告を回避
	(void)lpCmdLine;		// 引数未使用の警告を回避

	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

// メインウィンドウのウィンドウクラスを登録
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wcex.hCursor = nullptr; // カーソルを自分で設定するので、nullptrにする
	wcex.hbrBackground = nullptr; // 背景を自分で描画するので、nullptrにする
	wcex.lpszMenuName = nullptr; // メニューは使用しないので、nullptrにする
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = nullptr; // 小さいアイコンにも hIcon を兼用するので、nullptrにする

	return RegisterClassExW(&wcex);
}

// メインウィンドウの作成
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd = CreateWindowExW(
		0,                   // dwExStyle
		szWindowClass,       // lpClassName
		szTitle,             // lpWindowName
		WS_OVERLAPPEDWINDOW, // dwStyle
		CW_USEDEFAULT,       // X
		CW_USEDEFAULT,       // Y
		INIT_WINDOW_WIDTH,   // nWidth
		INIT_WINDOW_HEIGHT,  // nHeight
		nullptr,             // hWndParent
		nullptr,             // hMenu
		hInstance,           // hInstance
		nullptr              // lpParam
	);

	if (!hWnd)
	{
		return FALSE;
	}

	LoadSettings(hWnd);	// ★ウィンドウの位置とサイズを復元

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

////////////////////////////////////////// メインウィンドウの挙動

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {

	case WM_GETMINMAXINFO:
	{
		MINMAXINFO* pMinMaxInfo = (MINMAXINFO*)lParam;

		// 最小サイズの制限
		pMinMaxInfo->ptMinTrackSize.x = MIN_WINDOW_WIDTH;
		pMinMaxInfo->ptMinTrackSize.y = MIN_WINDOW_HEIGHT;

		// 最大サイズの制限
		pMinMaxInfo->ptMaxTrackSize.x = MAX_WINDOW_WIDTH;
		pMinMaxInfo->ptMaxTrackSize.y = MAX_WINDOW_HEIGHT;

		break;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_SETCURSOR:
	{
		WORD hitTest = LOWORD(lParam);
		if (hitTest == HTCLIENT)
		{
			HCURSOR hCursor = LoadCursor(0, IDC_ARROW);
			SetCursor(hCursor);
			return TRUE;	// 自分でカーソルを設定した場合はTRUEを返す
		}
		break;
	}

	case WM_DESTROY:
		SaveSettings(hWnd);	// ★設定を保存する

		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
