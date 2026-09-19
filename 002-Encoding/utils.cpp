#define NOMINMAX // Windows.h より前に定義する
#include <windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <memory>
#include <algorithm>
#include <string_view>

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

// バイナリのdataがUTF-8かどうかチェックする
// UTF-8の途中で切れていてもsizeバイトまで矛盾がなければtrueとする
static bool IsValidUtf8(const char* data, size_t size) {
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
			return false;
		}

		// 後続バイトの検証
		for (size_t j = 1; j < len; ++j) {
			if (i + j >= size) {
				// 末尾でデータが切れている場合、ここまで矛盾がないので true を返す
				return true;
			}

			c = bytes[i + j];
			if (j == 1) {
				// 2バイト目特有の範囲チェック（Overlong/サロゲート対策）
				if (c < minSecond || c > maxSecond) {
					return false;
				}
			}
			else {
				// 3バイト目、4バイト目は 0x80-0xBF の範囲内か
				if (c < 0x80 || c > 0xBF) {
					return false;
				}
			}
		}

		i += len;
	}

	return true;
}

// 文字列をワイド文字に変換し、std::vector<WCHAR>に格納する
// 元の文字列がUTF-8かShift_JISか自動判定する
// 出力先の末尾に必ずL'\0'が付くのでワイド文字列としても使える
static std::vector<WCHAR> ToWCHAR(const char* str)
{
	// UTF-8でなければShift_JISとみなす(CodePage=932)
	int codepage = IsValidUtf8(str, strlen(str)) ? CP_UTF8 : 932;

	// ワイド文字に変換した場合の文字数を得る（末尾の'\0'を含む）
	int wlen = MultiByteToWideChar(codepage, 0, str, -1, nullptr, 0);

	// バッファを確保し、strをワイド文字に変換して書き込む（末尾のL'\0'を含む）
	auto wbuf = std::vector<WCHAR>(wlen);
	MultiByteToWideChar(codepage, 0, str, -1, &wbuf[0], wlen);

	return wbuf;
}

// ウィンドウにテキストを表示する
// テキストに改行(\n)を含めることができる
// テキストがUTF-8かShift_JISか自動判定する
void MyTextOut(HDC hdc, int x, int y, const char* str)
{
	auto wbuf = ToWCHAR(str);
	MyTextOut(hdc, x, y, &wbuf[0]);
}

// ファイルからすべてのデータを読み込む
// ファイル名に日本語が含まれてもよい
std::vector<char> ReadAllBytes(LPCWSTR filePath)
{
	auto file = std::ifstream(filePath, std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		auto file_size = (size_t)file.tellg();
		if (file_size > 0) {
			auto buf = std::vector<char>(file_size);
			file.seekg(0, std::ios::beg);
			if (file.read(&buf[0], file_size)) {
				return buf;
			}
		}
	}
	return {};
}

// ファイルからすべてのデータを読み込む
// ファイル名に日本語（Shift_JISまたはUTF-8）が含まれてもよい
std::vector<char> ReadAllBytes(const char* filePath)
{
	auto wbuf = ToWCHAR(filePath);
	return ReadAllBytes(&wbuf[0]);
}

// バイナリデータをなんらかのエンコードされたテキストとみなし、UTF-8に変換する
// - データ先頭にUTF-16LEのBOMがあれば、BOMを除いたうえでUTF-8に変換する
// - 最大4096バイトでUTF-8として矛盾がなければUTF-8とみなすが、UTF-8のBOMがあれば削除する
// - UTF-8とUTF-16LEのどちらでもなければ、Shift_JISとみなしてUTF-8に変換する
// どの場合でも文字'\r'をすべて削除し、 末尾に'\0'を追加する
static std::vector<char> ToUtf8(const std::vector<char> data)
{
	if (data.empty()) return {};

	size_t size = data.size();
	std::vector<char> result;

	// UTF-16LE BOM の判定 (0xFF, 0xFE)
	if (size >= 2 && data[0] == 0xFF && data[1] == 0xFE) {
		const WCHAR* wbuf = reinterpret_cast<const WCHAR*>(&data[2]);
		int wlen = ((int)size - 2) / 2;
		int u8len = WideCharToMultiByte(932, 0, wbuf, wlen, nullptr, 0, nullptr, nullptr);
		auto u8buf = std::vector<char>((size_t)u8len);
		WideCharToMultiByte(932, 0, wbuf, wlen, &u8buf[0], u8len, nullptr, nullptr);
		result.assign(u8buf.begin(), u8buf.end());
	}
	// データがUTF-8かどうか判定する（上限4096バイトまで調べる）
	else if (IsValidUtf8(&data[0], std::min<size_t>(size, 4096))) {
		// UTF-8 BOM の判定 (0xEF, 0xBB, 0xBF)
		if (size >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF) {
			result.assign(data.begin() + 3, data.end());
		}
		else {
			result.assign(data.begin(), data.end());
		}
	}
	// UTF-16LEでもUTF-8でもなければShift_JISとみなす
	else {
		// Shift_JISをUtf-16LEに変換する
		int wlen = MultiByteToWideChar(932, 0, &data[0], (int)size, nullptr, 0);
		auto wbuf = std::vector<WCHAR>((size_t)wlen);
		MultiByteToWideChar(932, 0, &data[0], (int)size, &wbuf[0], wlen);

		// UTF-16LEをUTF-8に変換する
		int u8len = WideCharToMultiByte(932, 0, &wbuf[0], wlen, nullptr, 0, nullptr, nullptr);
		auto u8buf = std::vector<char>((size_t)u8len);
		WideCharToMultiByte(932, 0, &wbuf[0], wlen, &u8buf[0], u8len, nullptr, nullptr);

		// 結果をresultに入れる
		result.assign(u8buf.begin(), u8buf.end());
	}

	// 文字列から '\r' を削除する
	result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());

	// 末尾に '\0' を追加する
	result.push_back('\0');

	return result;
}

// ファイルから全テキストを読み込みUTF-8のstd::stringとして返す
// ファイル名に日本語が含まれてもよい
// 元のファイルデータがUTF-16LEやShift_JISならUTF-8に変換する
std::string ReadAllText(LPCWSTR wfilePath)
{
	auto utf8buf = ToUtf8(ReadAllBytes(wfilePath));
	return std::string(utf8buf.data());

	// メモ： std::vector<char> vec から std::string str を作る方法
	// 
	//	vecの最後に必ず'\0'がある場合は、vec.data() のみ指定する
	//	最初の'\0'の直前までの文字がstrに入る
	//		auto str = std::string(vec.data());
	// 
	//	終端が'\0'とは限らない場合は、vec.data()とvec.size()を指定する
	//	すべての文字がstrに入る（途中の'\0'も含む）
	//		auto str = std::string(vec.data(), vec.size());
}

// ファイルから全テキストを読み込みUTF-8のstd::stringとして返す
// ファイル名に日本語（Shift_JISまたはUTF-8）が含まれてもよい
// 元のファイルデータがUTF-16LEやShift_JISならUTF-8に変換する
std::string ReadAllText(const char* filePath)
{
	auto utf8buf = ToUtf8(ReadAllBytes(filePath));
	return std::string(utf8buf.data());
}
