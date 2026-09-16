#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <utility>

// 階層構造でデータを管理するクラス
class MyTree : public std::enable_shared_from_this<MyTree> {
private:
	std::wstring Key;
	std::wstring Value;
	std::weak_ptr<MyTree> Parent;
	std::vector<std::shared_ptr<MyTree>> Children;

	// 特殊文字の簡易エスケープ（XML用）
	static std::wstring escapeXml(const std::wstring& str) {
		std::wstring result;
		for (wchar_t c : str) {
			switch (c) {
			case L'&':  result += L"&amp;"; break;
			case L'<':  result += L"&lt;"; break;
			case L'>':  result += L"&gt;"; break;
			case L'\"': result += L"&quot;"; break;
			case L'\'': result += L"&apos;"; break;
			default:   result += c; break;
			}
		}
		return result;
	}

	// 特殊文字の簡易アンエスケープ（XML用）
	static std::wstring unescapeXml(const std::wstring& str) {
		std::wstring result = str;
		static const std::pair<std::wstring, std::wstring> entities[] = {
			{L"&amp;",  L"&"},
			{L"&lt;",   L"<"},
			{L"&gt;",   L">"},
			{L"&quot;", L"\""},
			{L"&apos;", L"'"}
		};
		for (const auto& e : entities) {
			size_t pos = 0;
			while ((pos = result.find(e.first, pos)) != std::wstring::npos) {
				result.replace(pos, e.first.length(), e.second);
				pos += e.second.length();
			}
		}
		return result;
	}

	// タグ文字列から指定した属性の値を取得するヘルパー
	static std::wstring getAttributeValue(const std::wstring& line, const std::wstring& attrName) {
		std::wstring target = attrName + L"=\"";
		size_t pos = line.find(target);
		if (pos == std::wstring::npos) return L"";
		pos += target.length();
		size_t endPos = line.find(L"\"", pos);
		if (endPos == std::wstring::npos) return L"";
		return unescapeXml(line.substr(pos, endPos - pos));
	}

	// シリアライズ用のヘルパー（再帰）
	static void serializeInternal(const std::shared_ptr<MyTree>& node, int depth, std::wostringstream& oss) {
		if (!node) return;

		std::wstring indent(depth * 2, L' ');
		std::wstring escapedKey = escapeXml(node->Key);
		std::wstring escapedVal = escapeXml(node->Value);

		if (node->Children.empty()) {
			oss << indent << L"<node key=\"" << escapedKey << L"\" value=\"" << escapedVal << L"\" />\n";
		}
		else {
			oss << indent << L"<node key=\"" << escapedKey << L"\" value=\"" << escapedVal << L"\">\n";
			for (const auto& child : node->Children) {
				serializeInternal(child, depth + 1, oss);
			}
			oss << indent << L"</node>\n";
		}
	}

public:
	// コンストラクタ
	MyTree() = default;
	explicit MyTree(const std::wstring& key) : Key(key) {}
	MyTree(const std::wstring& key, const std::wstring& value) : Key(key), Value(value) {}

	// 親ノードの取得
	std::shared_ptr<MyTree> getParent() const { return Parent.lock(); }

	// 子ノードのリストを取得
	std::vector<std::shared_ptr<MyTree>> getChildren() const { return Children; }

	// 子ノードの追加
	std::shared_ptr<MyTree> addChild(const std::wstring& key, const std::wstring& value) {
		auto child = std::make_shared<MyTree>(key, value);
		child->Parent = shared_from_this();
		this->Children.push_back(child);
		return child;
	}

	std::shared_ptr<MyTree> addChild(const std::wstring& key) {
		return addChild(key, L"");
	}

	std::shared_ptr<MyTree> addChild(const std::wstring& key, int value) {
		return addChild(key, std::to_wstring(value));
	}

	void addChild(std::shared_ptr<MyTree> child) {
		if (child) {
			child->Parent = shared_from_this();
			this->Children.push_back(child);
		}
	}

	// 指定した key を持つ最初の子ノードを取得
	// 条件を満たす子ノードがなければ nullptr を返す
	std::shared_ptr<MyTree> getChild(const std::wstring& key) const {
		for (const auto& child : this->Children) {
			if (child->Key == key) {
				return child;
			}
		}
		return nullptr;
	}

	// 指定した key を持つ最初の子ノードの値を std::wstring として取得
	// 条件を満たす子ノードがなければ空文字列 L"" を返す
	std::wstring getString(const std::wstring& key) const {
		auto child = getChild(key);
		return child ? child->Value : L"";
	}

	// 指定した key を持つ最初の子ノードの値を int として取得
	// 条件を満たす子ノードがなければ defaultValue を返す
	int getInt(const std::wstring& key, int defaultValue) const {
		auto child = getChild(key);
		if (child) {
			try {
				return std::stoi(child->Value);
			}
			catch (...) {
				/* FALLTHROUGH */
			}
		}
		return defaultValue;
	}

	// 空のツリーを作る
	static std::shared_ptr<MyTree> Create(const std::wstring& name) {
		return std::make_shared<MyTree>(name);
	}

	// ツリーの構造を XML 文字列に変換する
	std::wstring ToString() const {
		std::wostringstream oss;
		oss << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		serializeInternal(const_cast<MyTree*>(this)->shared_from_this(), 0, oss);
		return oss.str();
	}

	// XML 文字列からツリーを構築する
	static std::shared_ptr<MyTree> FromString(const std::wstring& data) {
		std::wistringstream iss(data);
		std::wstring line;

		std::shared_ptr<MyTree> root = nullptr;
		std::vector<std::shared_ptr<MyTree>> stack;

		while (std::getline(iss, line)) {
			// 前後の空白トリム
			size_t start = line.find_first_not_of(L" \t\r\n");
			if (start == std::wstring::npos) continue;
			size_t end = line.find_last_not_of(L" \t\r\n");
			line = line.substr(start, end - start + 1);

			// XML宣言やコメントはスキップ
			if (line.rfind(L"<?", 0) == 0 || line.rfind(L"<!--", 0) == 0) {
				continue;
			}

			// 自己閉じタグの場合: <node key="..." value="..." />
			if (line.rfind(L"<node", 0) == 0 && line.find(L"/>") != std::wstring::npos) {
				std::wstring key = getAttributeValue(line, L"key");
				std::wstring val = getAttributeValue(line, L"value");
				auto newNode = std::make_shared<MyTree>(key, val);

				if (!root) {
					root = newNode;
				}
				else if (!stack.empty()) {
					stack.back()->addChild(newNode);
				}
			}
			// 閉じタグの場合: </node>
			else if (line.rfind(L"</node>", 0) == 0) {
				if (!stack.empty()) {
					stack.pop_back();
				}
			}
			// 開始タグの場合: <node key="..." value="...">
			else if (line.rfind(L"<node", 0) == 0) {
				std::wstring key = getAttributeValue(line, L"key");
				std::wstring val = getAttributeValue(line, L"value");
				auto newNode = std::make_shared<MyTree>(key, val);

				if (!root) {
					root = newNode;
				}
				else if (!stack.empty()) {
					stack.back()->addChild(newNode);
				}
				stack.push_back(newNode);
			}
		}

		return root;
	}
};
