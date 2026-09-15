#include <string>
#include <vector>
#include <sstream>

// 階層構造でデータを管理するクラス
class MyTree : public std::enable_shared_from_this<MyTree> {
private:
	std::string Key;
	std::string Value;
	std::weak_ptr<MyTree> Parent;
	std::vector<std::shared_ptr<MyTree>> Children;

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
	static void serializeInternal(const std::shared_ptr<MyTree>& node, int depth, std::ostringstream& oss) {
		if (!node) return;

		std::string indent(depth * 2, ' ');
		std::string escapedKey = escapeXml(node->Key);
		std::string escapedVal = escapeXml(node->Value);

		if (node->Children.empty()) {
			oss << indent << "<node key=\"" << escapedKey << "\" value=\"" << escapedVal << "\" />\n";
		}
		else {
			oss << indent << "<node key=\"" << escapedKey << "\" value=\"" << escapedVal << "\">\n";
			for (const auto& child : node->Children) {
				serializeInternal(child, depth + 1, oss);
			}
			oss << indent << "</node>\n";
		}
	}

public:
	// コンストラクタ
	MyTree() = default;
	explicit MyTree(const std::string& key) : Key(key) {}
	MyTree(const std::string& key, const std::string& value) : Key(key), Value(value) {}

	// 親ノードの取得
	std::shared_ptr<MyTree> getParent() const { return Parent.lock(); }

	// 子ノードのリストを取得
	std::vector<std::shared_ptr<MyTree>> getChildren() const { return Children; }

	// 子ノードの追加
	std::shared_ptr<MyTree> addChild(const std::string& key, const std::string& value) {
		auto child = std::make_shared<MyTree>(key, value);
		child->Parent = shared_from_this();
		this->Children.push_back(child);
		return child;
	}

	std::shared_ptr<MyTree> addChild(const std::string& key) {
		return addChild(key, "");
	}

	std::shared_ptr<MyTree> addChild(const std::string& key, int value) {
		return addChild(key, std::to_string(value));
	}

	void addChild(std::shared_ptr<MyTree> child) {
		if (child) {
			child->Parent = shared_from_this();
			this->Children.push_back(child);
		}
	}

	// 指定した key を持つ最初の子ノードを取得
	std::shared_ptr<MyTree> getChild(const std::string& key) const {
		for (const auto& child : Children) {
			if (child->Key == key) {
				return child;
			}
		}
		return nullptr;
	}

	// 指定したkeyを持つ最初の子ノードの値をstd::stringとして取得
	std::string getString(const std::string& key) const {
		auto child = getChild(key);
		return child ? child->Value : "";
	}

	// 指定したkeyを持つ最初の子ノードの値をintとして取得
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

	// 空のツリーを作る
	static std::shared_ptr<MyTree> Create(const std::string& name) {
		return std::make_shared<MyTree>(name);
	}

	// ツリーの構造をXML文字列に変換する
	std::string ToString() const {
		std::ostringstream oss;
		oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		serializeInternal(const_cast<MyTree*>(this)->shared_from_this(), 0, oss);
		return oss.str();
	}

	// XML文字列からツリーを構築する
	static std::shared_ptr<MyTree> FromString(const std::string& data) {
		std::istringstream iss(data);
		std::string line;

		std::shared_ptr<MyTree> root = nullptr;
		std::vector<std::shared_ptr<MyTree>> stack;

		while (std::getline(iss, line)) {
			// 前後の空白トリム
			size_t start = line.find_first_not_of(" \t\r\n");
			if (start == std::string::npos) continue;
			size_t end = line.find_last_not_of(" \t\r\n");
			line = line.substr(start, end - start + 1);

			// XML宣言やコメントはスキップ
			if (line.rfind("<?", 0) == 0 || line.rfind("<!--", 0) == 0) {
				continue;
			}

			// 自己閉じタグの場合: <node key="..." value="..." />
			if (line.rfind("<node", 0) == 0 && line.find("/>") != std::string::npos) {
				std::string key = getAttributeValue(line, "key");
				std::string val = getAttributeValue(line, "value");
				auto newNode = std::make_shared<MyTree>(key, val);

				if (!root) {
					root = newNode;
				}
				else if (!stack.empty()) {
					stack.back()->addChild(newNode);
				}
			}
			// 閉じタグの場合: </node>
			else if (line.rfind("</node>", 0) == 0) {
				if (!stack.empty()) {
					stack.pop_back();
				}
			}
			// 開始タグの場合: <node key="..." value="...">
			else if (line.rfind("<node", 0) == 0) {
				std::string key = getAttributeValue(line, "key");
				std::string val = getAttributeValue(line, "value");
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
