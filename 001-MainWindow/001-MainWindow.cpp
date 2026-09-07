// 001-MainWindow.cpp

#include "framework.h"

HINSTANCE hInst;
WCHAR szTitle[] = L"win32api 001-MainWindow";
WCHAR szWindowClass[] = L"001-MainWindow Class";

ATOM             MyRegisterClass(HINSTANCE hInstance);
BOOL             InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(
	_In_     HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_     LPWSTR lpCmdLine,
	_In_     int nCmdShow)
{
	(void)hPrevInstance;
	(void)lpCmdLine;

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

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

	HWND hWnd = CreateWindowExW(
		0,                   // dwExStyle
		szWindowClass,       // lpClassName
		szTitle,             // lpWindowName
		WS_OVERLAPPEDWINDOW, // dwStyle
		CW_USEDEFAULT,       // X
		CW_USEDEFAULT,       // Y
		600,                 // nWidth
		400,                 // nHeight
		nullptr,             // hWndParent
		nullptr,             // hMenu
		hInstance,           // hInstance
		nullptr              // lpParam
	);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {

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
			return TRUE;	// 自分でカーソルを設定したら、TRUEを返す
		}
		break;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

