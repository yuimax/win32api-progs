#include <windows.h>
#include <vector>

// 文字列がUTF-8かどうか判定する
static BOOL IsValidUtf8(const char* str)
{
	return MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str, -1, nullptr, 0) > 0;
}

// 文字列をstd::vector<WCHAR>に変換する
static std::vector<WCHAR> ToWideChar(const char* str)
{
	// 元の文字列がUTF-8でなければcp932(Shift_JIS)とみなす
	int codepage = IsValidUtf8(str) ? CP_UTF8 : 932;

	// ワイド文字に変換した場合の文字数を得る（末尾の'\0'を含む）
	int wlen = MultiByteToWideChar(codepage, 0, str, -1, nullptr, 0);

	// バッファを確保し、strをワイド文字に変換して書き込む（末尾の'\0'を含む）
	std::vector<WCHAR> wbuf(wlen);
	MultiByteToWideChar(codepage, 0, str, -1, &wbuf[0], wlen);

	return wbuf;
}

// ウィンドウにテキストを表示する
// テキストに改行(\n)を含めることができる
void MyTextOut(HDC hdc, int x, int y, const char* str)
{
	// テキストをワイド文字に変換する
	auto wbuf = ToWideChar(str);

	// Win32APIのDrawText()を呼び出し、テキストを画面に表示する
	RECT rc = { x, y, 0, 0 }; // left, top, right, bottom
	DrawText(hdc, &wbuf[0], (int)wbuf.size() - 1, &rc, DT_NOCLIP | DT_NOPREFIX);

	// メモ： DrawText()について
	//	第2引数は表示するテキストで、ワイド文字列（WCHARへのポインタ）を指定する
	//	テキスト内では改行コード(\n)が有効
	//	テキスト内の文字'&'は「次の文字を下線で修飾する」という意味になる
	//	'&'を普通の文字として表示するには、書式に DT_NOPREFIX を含める
	//	第3引数は末尾の'\0'を除く文字数で、-1を指定すると自動計算する
	//	第4引数は表示範囲で、書式に DT_NOCLIP がある場合は rc.right と rc.bottom を無視
	//	第5引数は書式で、右詰めやセンタリングなどいろいろ
}
