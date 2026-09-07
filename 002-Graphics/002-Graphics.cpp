// 002-Graphics.cpp

#include "framework.h"

HINSTANCE hInst;
WCHAR szTitle[] = L"win32api 002-Graphics";
WCHAR szWindowClass[] = L"002-Graphics Class";

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

		// 背景の塗りつぶし
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		// 図形の枠線のみ描画、NULL_BRUSHを使用して図形塗りつぶしなし
		{
			HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);	// 塗りつぶしなしのブラシ
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, (HGDIOBJ)nullBrush);

			HPEN greenPen = CreatePen(PS_SOLID, 3, RGB(128, 224, 64)); // 緑色のペン
			HPEN bluePen = CreatePen(PS_SOLID, 3, RGB(64, 128, 224)); // 青色のペン

			HPEN oldPen = (HPEN)SelectObject(hdc, (HGDIOBJ)greenPen);
			Rectangle(hdc, 50, 50, 200, 150);	// hdc, X, Y, Width, Height

			SelectObject(hdc, (HGDIOBJ)bluePen);
			Ellipse(hdc, 100, 100, 250, 200);	// hdc, X, Y, Width, Height

			// ペンを元に戻す
			SelectObject(hdc, oldPen);

			// CreatePenで作成したペンは使用後に削除する
			DeleteObject(greenPen);
			DeleteObject(bluePen);

			// ブラシを元に戻す
			SelectObject(hdc, oldBrush);

			// GetStockObjectで取得したブラシは削除しない
			// DeleteObject(nullBrush); // これは不要
		}

		// 面の塗りつぶし、NULL_PENを使用して枠線なし
		{
			HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);	// 枠線なしのペン
			HPEN oldPen = (HPEN)SelectObject(hdc, (HGDIOBJ)nullPen);

			// CreateSolidBrushでブラシを作成する
			HBRUSH greenBrush = CreateSolidBrush(RGB(128, 224, 64)); // 緑色のブラシ
			HBRUSH blueBrush = CreateSolidBrush(RGB(64, 128, 224)); // 青色のブラシ

			// 緑色のブラシで四角形を描く
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, (HGDIOBJ)greenBrush);
			Rectangle(hdc, 300, 50, 450, 150);	// hdc, X, Y, Width, Height

			// 青色のブラシで楕円を描く
			SelectObject(hdc, (HGDIOBJ)blueBrush);
			Ellipse(hdc, 350, 100, 500, 200);	// hdc, X, Y, Width, Height

			// ブラシを元に戻す
			SelectObject(hdc, oldBrush);

			// CreateSolidBrushで作成したブラシは使用後に削除する
			DeleteObject(greenBrush);
			DeleteObject(blueBrush);

			// ペンを元に戻す
			SelectObject(hdc, oldPen);

			// GetStockObjectで取得したペンは削除しない
			// DeleteObject(nullPen); // これは不要
		}

		// テキストの描画
		{
			// テキストを透過モードで描画する
			// DrawText, TextOut, ExtTextOut などのテキスト描画関数は、デフォルトでは不透明モード
			SetBkMode(hdc, TRANSPARENT);

			// フォントの作成と選択
			HFONT hFont = CreateFont(
				48,								// 行の高さ（48ピクセル）
				0,								// 幅（0で自動調整）
				0,								// エスケープメント角度
				0,								// ベースライン角度
				FW_NORMAL,						// 太さ (FW_NORMAL, FW_BOLD など)
				FALSE,							// 斜体 (TRUE / FALSE)
				FALSE,							// 下線
				FALSE,							// 打消線
				SHIFTJIS_CHARSET,				// 文字セット（日本語環境）
				OUT_DEFAULT_PRECIS,				// 出力精度
				CLIP_DEFAULT_PRECIS,			// クリッピング精度
				DEFAULT_QUALITY,				// 描画品質
				DEFAULT_PITCH | FF_DONTCARE,	// ピッチとファミリー
				L"Noto Sans JP"					// フォント名
			);
			HFONT oldfont = (HFONT)SelectObject(hdc, hFont);

			// DrawTextの描画領域を指定する
			// DT_NOCLIPの場合 right と bottom は無視される
			RECT textRect = { 70, 100, 0, 0 };	// left, top, right, bottom

			// DrawTextでテキストを描画する
			// DrawTextの第3引数に-1を指定すると、文字列の終端まで描画される
			DrawText(
				hdc,
				L"Hell, world\nこんにちは世界\n",
				-1,
				&textRect,
				DT_LEFT | DT_TOP | DT_NOCLIP
			);

			// 使用後にフォントを削除する
			SelectObject(hdc, oldfont);
			DeleteObject(hFont);
		}

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

