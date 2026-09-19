#include <windows.h>
#include <string>
#include <sstream>

// 階層構造でデータを管理するクラス
class TreeNode : public std::enable_shared_from_this<TreeNode> {
private:
	std::string Key;
	std::string Value;
	std::weak_ptr<TreeNode> Parent;
	std::vector<std::shared_ptr<TreeNode>> Children;

	// 特殊文字の簡易エスケープ（XML用）
	static std::string escapeXml(const std::string& str) {
		std::string result;
		for (char c : str) {
			switch (c) {
			case '&':  result += "&amp;"; break;
			case '<':  result += "&lt;"; break;
			case '>':  result += "&gt;"; break;
			case '\"': result += "&quot;"; break;
			case '\'': result += "&apos;"; break;
			default:   result += c; break;
			}
		}
		return result;
	}

	// 特殊文字の簡易アンエスケープ（XML用）
	static std::string unescapeXml(const std::string& str) {
		std::string result = str;
		static const std::pair<std::string, std::string> entities[] = {
			{"&amp;",  "&"},
			{"&lt;",   "<"},
			{"&gt;",   ">"},
			{"&quot;", "\""},
			{"&apos;", "'"}
		};
		for (const auto& e : entities) {
			size_t pos = 0;
			while ((pos = result.find(e.first, pos)) != std::string::npos) {
				result.replace(pos, e.first.length(), e.second);
				pos += e.second.length();
			}
		}
		return result;
	}

	// タグ文字列から指定した属性の値を取得するヘルパー
	static std::string getAttributeValue(const std::string& line, const std::string& attrName) {
		std::string target = attrName + "=\"";
		size_t pos = line.find(target);
		if (pos == std::string::npos) return "";
		pos += target.length();
		size_t endPos = line.find("\"", pos);
		if (endPos == std::string::npos) return "";
		return unescapeXml(line.substr(pos, endPos - pos));
	}

	// シリアライズ用のヘルパー（再帰）
	static void serialize(const std::shared_ptr<const TreeNode>& node, int depth, std::ostringstream& oss) {
		if (!node) return;

		std::string indent(depth * 2, ' ');
		std::string escapedKey = escapeXml(node->Key);
		std::string escapedVal = escapeXml(node->Value);
		std::string attribKey = " name=\"" + escapedKey + "\"";
		std::string attribVal = escapedVal.empty() ? "" : " value=\"" + escapedVal + "\"";

		if (node->Children.empty()) {
			oss << indent << "<node" << attribKey << attribVal << " />\n";
		}
		else {
			oss << indent << "<node" << attribKey << attribVal << ">\n";
			for (const auto& child : node->Children) {
				serialize(child, depth + 1, oss);
			}
			oss << indent << "</node>\n";
		}
	}

	// Encodingが不明な文字列をUTF-8に統一する
	static std::string Utf8(const std::string& str)
	{
		// UTF-8なら何もしない
		if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.c_str(), -1, nullptr, 0) > 0) {
			return str;
		}

		// UTF-8でなければCP932（Shift_JIS）とみなす
		const int cp932 = 932;

		// CP932からワイド文字に変換する
		int wlen = MultiByteToWideChar(cp932, 0, str.c_str(), -1, nullptr, 0);
		std::vector<WCHAR> wbuf(wlen);
		MultiByteToWideChar(cp932, 0, str.c_str(), -1, &wbuf[0], wlen);

		// ワイド文字列からUTF-8に変換する
		int utf8len = WideCharToMultiByte(CP_UTF8, 0, &wbuf[0], wlen, nullptr, 0, nullptr, nullptr);
		std::vector<char> utf8buf(utf8len);
		WideCharToMultiByte(CP_UTF8, 0, &wbuf[0], wlen, &utf8buf[0], utf8len, nullptr, nullptr);

		// std::stringとして返す
		return std::string(&utf8buf[0], utf8buf.size() - 1);
	}

public:
	// コンストラクタ / デストラクタ
	TreeNode() = default;
	explicit TreeNode(const std::string& key) : Key(Utf8(key)) {}
	TreeNode(const std::string& key, const std::string& value) : Key(Utf8(key)), Value(Utf8(value)) {}
	virtual ~TreeNode() = default; // 派生クラスのために仮想デストラクタを追加

	// アクセサ
	const std::string& getKey() const { return Key; }
	const std::string& getValue() const { return Value; }

	// 親ノードの取得
	std::shared_ptr<TreeNode> getParent() const { return Parent.lock(); }

	// 子ノードのリストを取得
	std::vector<std::shared_ptr<TreeNode>> getChildren() const { return Children; }

	// 子ノードの追加
	void addChild(std::shared_ptr<TreeNode> child) {
		if (child) {
			child->Parent = shared_from_this();
			this->Children.push_back(child);
		}
	}

	std::shared_ptr<TreeNode> addChild(const std::string& key, const std::string& value) {
		auto child = std::make_shared<TreeNode>(key, value);
		addChild(child);
		return child;
	}

	std::shared_ptr<TreeNode> addChild(const std::string& key) {
		return addChild(key, "");
	}

	std::shared_ptr<TreeNode> addChild(const std::string& key, int value) {
		return addChild(key, std::to_string(value));
	}

	// 指定した key を持つ最初の子ノードを取得
	std::shared_ptr<TreeNode> getChild(const std::string& key) const {
		const std::string& utf8key = Utf8(key);
		for (const auto& child : this->Children) {
			if (child->Key == utf8key) {
				return child;
			}
		}
		return nullptr;
	}

	// 指定した key を持つ最初の子ノードの値を std::string として取得
	std::string getString(const std::string& key, const std::string& defaultValue) const {
		auto child = getChild(key);
		return child ? child->Value : Utf8(defaultValue);
	}

	// 指定した key を持つ最初の子ノードの値を int として取得
	int getInt(const std::string& key, int defaultValue) const {
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

	// 空のノードを作る
	static std::shared_ptr<TreeNode> Create(const std::string& name) {
		return std::make_shared<TreeNode>(name);
	}

	// ツリー構造をXML文字列に変換する
	virtual std::string ToString() const {
		std::ostringstream oss;
		serialize(shared_from_this(), 0, oss);
		return oss.str();
	}

	// XML文字列からツリーを構築する
	static std::shared_ptr<TreeNode> FromString(const std::string& data) {
		std::istringstream iss(Utf8(data));
		std::string line;

		std::shared_ptr<TreeNode> root = nullptr;
		std::vector<std::shared_ptr<TreeNode>> stack;

		while (std::getline(iss, line)) {
			size_t start = line.find_first_not_of(" \t\r\n");
			if (start == std::string::npos) continue;
			size_t end = line.find_last_not_of(" \t\r\n");
			line = line.substr(start, end - start + 1);

			if (line.rfind("<?", 0) == 0 || line.rfind("<!--", 0) == 0) {
				continue;
			}

			if (line.rfind("<node", 0) == 0 && line.find("/>") != std::string::npos) {
				std::string key = getAttributeValue(line, "name");
				std::string val = getAttributeValue(line, "value");
				auto newNode = std::make_shared<TreeNode>(key, val);

				if (!root) {
					root = newNode;
				}
				else if (!stack.empty()) {
					stack.back()->addChild(newNode);
				}
			}
			else if (line.rfind("</node>", 0) == 0) {
				if (!stack.empty()) {
					stack.pop_back();
				}
			}
			else if (line.rfind("<node", 0) == 0) {
				std::string key = getAttributeValue(line, "name");
				std::string val = getAttributeValue(line, "value");
				auto newNode = std::make_shared<TreeNode>(key, val);

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

// TreeNode を継承したツリークラス
class MyTree : public TreeNode {
private:
	MyTree() = default;

public:
	// ツリー全体をヘッダ付きのXML文字列に変換
	std::string ToString() const override {
		return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			+ TreeNode::ToString();
	}
};