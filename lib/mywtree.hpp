#pragma once
#include "mytree.hpp"  // 元の MyTree クラス
#include <windows.h>   // MultiByteToWideChar / WideCharToMultiByte
#include <string>
#include <memory>

// UTF-8 (std::string) ↔ UTF-16 (std::wstring) 変換ヘルパー（Visual C++ 向け）
namespace MyWTreeDetail {

	inline std::wstring Utf8ToWide(const std::string& utf8)
	{
		if (utf8.empty()) {
			return std::wstring();
		}

		// 必要なバッファサイズを取得
		int len = ::MultiByteToWideChar(
			CP_UTF8, 0,
			utf8.data(), static_cast<int>(utf8.size()),
			nullptr, 0);

		if (len <= 0) {
			return std::wstring();
		}

		std::wstring result(len, L'\0');
		::MultiByteToWideChar(
			CP_UTF8, 0,
			utf8.data(), static_cast<int>(utf8.size()),
			&result[0], len);

		return result;
	}

	inline std::string WideToUtf8(const std::wstring& wide)
	{
		if (wide.empty()) {
			return std::string();
		}

		// 必要なバッファサイズを取得
		int len = ::WideCharToMultiByte(
			CP_UTF8, 0,
			wide.data(), static_cast<int>(wide.size()),
			nullptr, 0,
			nullptr, nullptr);

		if (len <= 0) {
			return std::string();
		}

		std::string result(len, '\0');
		::WideCharToMultiByte(
			CP_UTF8, 0,
			wide.data(), static_cast<int>(wide.size()),
			&result[0], len,
			nullptr, nullptr);

		return result;
	}

} // namespace MyWTreeDetail


/**
 * MyTree を継承したワイド文字列対応版
 *
 * - 既存の MyTree の全機能をそのまま利用可能
 * - ToWString()  : ツリーを UTF-16 の XML ワイド文字列にシリアライズ
 * - FromWString(): UTF-16 の XML ワイド文字列からツリーを構築
 *
 * ワイド文字列は Visual C++ の WCHAR / LPWSTR (UTF-16) を想定。
 * 内部の Key/Value は従来通り UTF-8 の std::string で保持します。
 */
class MyWTree : public MyTree
{
public:
	// コンストラクタ（基底クラスへ転送）
	MyWTree() = default;
	explicit MyWTree(const std::string& key) : MyTree(key) {}
	MyWTree(const std::string& key, const std::string& value) : MyTree(key, value) {}

	// 空のツリーを作成（推奨ファクトリ）
	static std::shared_ptr<MyWTree> Create(const std::string& name)
	{
		return std::make_shared<MyWTree>(name);
	}

	/**
	 * ツリーをワイド文字列の XML にシリアライズする
	 * @return UTF-16 の XML 文字列 (std::wstring)
	 */
	std::wstring ToWString() const
	{
		// 基底クラスの ToString() で UTF-8 XML を生成し、UTF-16 に変換
		return MyWTreeDetail::Utf8ToWide(this->ToString());
	}

	/**
	 * ワイド文字列の XML からツリーを構築する
	 * @param data  UTF-16 の XML 文字列 (const std::wstring& または LPWSTR 相当)
	 * @return 成功時はルートノードの shared_ptr、失敗時は nullptr
	 *
	 * 注意: 返却型は基底の MyTree です（内部ノードも MyTree として生成されるため）。
	 *       MyWTree として扱いたい場合は static_pointer_cast などでキャスト可能ですが、
	 *       追加メンバがないため実質 MyTree と同じです。
	 */
	static std::shared_ptr<MyWTree> FromWString(const std::wstring& data)
	{
		// UTF-16 → UTF-8 に変換してから基底の FromString を呼び出す
		std::string utf8 = MyWTreeDetail::WideToUtf8(data);
		auto tree = MyTree::FromString(utf8);
		return std::static_pointer_cast<MyWTree, MyTree>(tree);
	}

	// LPWSTR / const WCHAR* からも直接呼べるようにオーバーロード
	static std::shared_ptr<MyWTree> FromWString(const WCHAR* data)
	{
		if (!data) {
			return nullptr;
		}
		return FromWString(std::wstring(data));
	}
};
