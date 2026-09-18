#include <string>
#include <vector>
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
	static void serialize(const std::shared_ptr<TreeNode>& node, int depth, std::ostringstream& oss) {
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


public:
	// コンストラクタ
	TreeNode() = default;
	explicit TreeNode(const std::string& key) : Key(key) {}
	TreeNode(const std::string& key, const std::string& value) : Key(key), Value(value) {}

	// 親ノードの取得
	std::shared_ptr<TreeNode> getParent() const { return Parent.lock(); }

	// 子ノードのリストを取得
	std::vector<std::shared_ptr<TreeNode>> getChildren() const { return Children; }

	// 子ノードの追加
	std::shared_ptr<TreeNode> addChild(const std::string& key, const std::string& value) {
		auto child = std::make_shared<TreeNode>(key, value);
		child->Parent = shared_from_this();
		this->Children.push_back(child);
		return child;
	}

	std::shared_ptr<TreeNode> addChild(const std::string& key) {
		return addChild(key, "");
	}

	std::shared_ptr<TreeNode> addChild(const std::string& key, int value) {
		return addChild(key, std::to_string(value));
	}

	void addChild(std::shared_ptr<TreeNode> child) {
		if (child) {
			child->Parent = shared_from_this();
			this->Children.push_back(child);
		}
	}

	// 指定した key を持つ最初の子ノードを取得
	// 条件を満たす子ノードがなければ nullptr を返す
	std::shared_ptr<TreeNode> getChild(const std::string& key) const {
		for (const auto& child : this->Children) {
			if (child->Key == key) {
				return child;
			}
		}
		return nullptr;
	}

	// 指定した key を持つ最初の子ノードの値を std::string として取得
	// 条件を満たす子ノードがなければ defaultValue を返す
	std::string getString(const std::string& key, const std::string& defaltValue) const {
		auto child = getChild(key);
		return child ? child->Value : defaltValue;
	}

	// 指定した key を持つ最初の子ノードの値を std::string として取得
	// 条件を満たす子ノードがなければ空文字列 "" を返す
	std::string getString(const std::string& key) const {
		return getString(key, "");
	}

	// 指定した key を持つ最初の子ノードの値を int として取得
	// 条件を満たす子ノードがなければ defaultValue を返す
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
	std::string ToString() const {
		std::ostringstream oss;
		serialize(const_cast<TreeNode*>(this)->shared_from_this(), 0, oss);
		return oss.str();
	}

	// XML文字列からツリーを構築する
	// TreeNodeに変換できない場合はnullptrを返す
	static std::shared_ptr<TreeNode> FromString(const std::string& data) {
		std::istringstream iss(data);
		std::string line;

		std::shared_ptr<TreeNode> root = nullptr;
		std::vector<std::shared_ptr<TreeNode>> stack;

		while (std::getline(iss, line)) {
			// 前後の空白をトリム
			size_t start = line.find_first_not_of(" \t\r\n");
			if (start == std::string::npos) continue;
			size_t end = line.find_last_not_of(" \t\r\n");
			line = line.substr(start, end - start + 1);

			// XML宣言やコメントはスキップ
			if (line.rfind("<?", 0) == 0 || line.rfind("<!--", 0) == 0) {
				continue;
			}

			// 自己閉じタグの場合: <node ... />
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
			// 閉じタグの場合: </node>
			else if (line.rfind("</node>", 0) == 0) {
				if (!stack.empty()) {
					stack.pop_back();
				}
			}
			// 開始タグの場合: <node ... >
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

// TreeNode に Encoding を追加したクラス
// ToString()メソッドをオーバーライドしてXMLヘッダを付加するように変更
class MyTree : public TreeNode {
private:
	std::string Encoding = "UTF-8";

public:
    MyTree() = default;

	explicit MyTree(const std::string& name)
		: TreeNode(name) {}

	MyTree(const std::string& name, const std::string& encoding)
		: TreeNode(name), Encoding(encoding) {}

    // ツリー全体をヘッダ付きのXML文字列に変換
    std::string ToString() const {
		std::string xmlHeader = "<?xml version=\"1.0\" encoding=\"" + Encoding + "\"?>\n";
		return xmlHeader + ((TreeNode*)this)->ToString();
    }
};
