#include <windows.h>
#include <vector>
#include <string.h>	// for strlen()

// ウィンドウにワイド文字のテキストを表示する
// テキストに改行(\n)を含めることができる
void MyTextOut(HDC hdc, int x, int y, LPCWSTR wstr)
{
	RECT rc = { x, y, 0, 0 }; // left, top, right, bottom
	DrawText(hdc, wstr, -1, &rc, DT_NOCLIP | DT_NOPREFIX);

	// メモ： DrawText() について
	//	第2引数は表示するテキストで、ワイド文字列を指定する
	//	テキスト内では改行コード(\n)が有効
	//	テキスト内の文字'&'は「次の文字を下線で修飾する」という意味になる
	//	'&'を普通の文字として表示するには、書式に DT_NOPREFIX を含める
	//	第3引数は末尾の'\0'を除く文字数で、-1を指定すると自動計算する
	//	第4引数は表示範囲で、書式に DT_NOCLIP がある場合は rc.right と rc.bottom を無視
	//	第5引数は書式で、右詰めやセンタリングなどいろいろ
}

// 成功フラグ付き戻り値
template <typename T>
struct result {
	bool success;
	T value;
};

// バイナリのdataがUTF-8かどうかチェックする
// UTF-8判定結果(bool)と、UTF-8のバイト数(size_t)を返す
// UTF-8の途中で切れていても、そこまで矛盾がなければ判定結果はtrueとする
static result<size_t> IsValidUtf8(const char* data, size_t size) {
	auto bytes = (const unsigned char*)data;
	size_t i = 0;
	while (i < size) {
		int c = bytes[i];
		if (c <= 0x7F) {
			i += 1;
			continue;
		}

		size_t len;
		int minSecond = 0x80;
		int maxSecond = 0xBF;
		if (c >= 0xC2 && c <= 0xDF) {	// 2バイト文字 (0xC2 - 0xDF)
			len = 2;
		}
		else if (c >= 0xE0 && c <= 0xEF) {	// 3バイト文字 (0xE0 - 0xEF)
			len = 3;
			if (c == 0xE0) {
				minSecond = 0xA0; // Overlong 対策 (Unicode < U+0800 の排除)
			}
			else if (c == 0xED) {
				maxSecond = 0x9F; // サロゲートペア領域 (U+D800 - U+DFFF) の排除
			}
		}
		else if (c >= 0xF0 && c <= 0xF4) {	// 4バイト文字 (0xF0 - 0xF4)
			len = 4;
			if (c == 0xF0) {
				minSecond = 0x90; // Overlong 対策 (Unicode < U+10000 の排除)
			}
			else if (c == 0xF4) {
				maxSecond = 0x8F; // Unicode上限 U+10FFFF 超過の排除
			}
		}
		else {	// 先頭バイトとして不正な値 (0x80-0xC1, 0xF5-0xFF)
			return { false, i };
		}

		// 後続バイトの検証
		for (size_t j = 1; j < len; ++j) {
			if (i + j >= size) {
				// 末尾でデータが切れている場合、ここまで矛盾がないので true を返す
				return { true, i };
			}

			c = bytes[i + j];
			if (j == 1) {
				// 2バイト目特有の範囲チェック（Overlong/サロゲート対策）
				if (c < minSecond || c > maxSecond) {
					return { false, i };
				}
			}
			else {
				// 3バイト目、4バイト目は 0x80-0xBF の範囲内か
				if (c < 0x80 || c > 0xBF) {
					return { false, i };
				}
			}
		}

		i += len;
	}

	return { true, i };
}

// 文字列をワイド文字に変換し、std::vector<WCHAR>に格納する
// 元の文字列がUTF-8かShift_JISか自動判定する
// 出力先の末尾に必ずL'\0'が付くのでワイド文字列としても使える
static std::vector<WCHAR> ToWCHAR(const char* str)
{
	size_t size = strlen(str);	// 末尾の'\0'を含まない長さ
	int codepage = 0;

	// UTF-8でなければShift_JISとみなす
	auto u8check = IsValidUtf8(str, size);
	if (u8check.success) {
		size = u8check.value;
		codepage = CP_UTF8;
	}
	else {
		codepage = 932;	// Shift_JISのコードページ
	}

	// ワイド文字の文字数を得る（末尾の'\0'を含む）
	int slen = static_cast<int>(size) + 1; // 末尾の'\0'を含む長さ
	int wlen = MultiByteToWideChar(codepage, 0, str, slen, nullptr, 0);

	// strをワイド文字に変換する（末尾のL'\0'を含む）
	auto wbuf = std::vector<WCHAR>(wlen);
	MultiByteToWideChar(codepage, 0, str, slen, wbuf.data(), wlen);

	return wbuf;
}

// ウィンドウにテキストを表示する
// テキストに改行(\n)を含めることができる
// テキストがUTF-8かShift_JISか自動判定する
void MyTextOut(HDC hdc, int x, int y, const char* str)
{
	auto wbuf = ToWCHAR(str);
	MyTextOut(hdc, x, y, wbuf.data());
}
