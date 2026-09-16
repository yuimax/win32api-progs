#include "utils.h"

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
