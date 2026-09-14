#include <string>
#include <vector>
#include <sstream>

class TreeNode : public std::enable_shared_from_this<TreeNode> {
private:
    std::string m_value;
    std::weak_ptr<TreeNode> m_parent;
    std::vector<std::shared_ptr<TreeNode>> m_children;

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

    // シリアライズ用のヘルパー（再帰）
    static void serializeInternal(const std::shared_ptr<TreeNode>& node, int depth, std::ostringstream& oss) {
        if (!node) return;

        std::string indent(depth * 2, ' ');
        std::string escapedVal = escapeXml(node->getValue());

        if (node->getChildren().empty()) {
            // 子がない場合は自己閉じタグ
            oss << indent << "<Node value=\"" << escapedVal << "\" />\n";
        }
        else {
            // 子がある場合
            oss << indent << "<Node value=\"" << escapedVal << "\">\n";
            for (const auto& child : node->getChildren()) {
                serializeInternal(child, depth + 1, oss);
            }
            oss << indent << "</Node>\n";
        }
    }

public:
    // コンストラクタ
    explicit TreeNode(const std::string& value) : m_value(value) {}

    // 値の取得・設定
    const std::string& getValue() const { return m_value; }
    void setValue(const std::string& value) { m_value = value; }

    // 親ノードの取得
    std::shared_ptr<TreeNode> getParent() const { return m_parent.lock(); }

    // 子ノードのコレクションを取得
    const std::vector<std::shared_ptr<TreeNode>>& getChildren() const { return m_children; }

    // 子ノードの追加
    std::shared_ptr<TreeNode> addChild(const std::string& value) {
        auto child = std::make_shared<TreeNode>(value);
        child->m_parent = shared_from_this();
        m_children.push_back(child);
        return child;
    }

    void addChild(std::shared_ptr<TreeNode> child) {
        if (child) {
            child->m_parent = shared_from_this();
            m_children.push_back(child);
        }
    }

    // --- XMLシリアライズ ---
    std::string serialize() const {
        std::ostringstream oss;
        oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        serializeInternal(const_cast<TreeNode*>(this)->shared_from_this(), 0, oss);
        return oss.str();
    }

    // --- XMLデシリアライズ ---
    static std::shared_ptr<TreeNode> deserialize(const std::string& data) {
        std::istringstream iss(data);
        std::string line;

        std::shared_ptr<TreeNode> root = nullptr;
        std::vector<std::shared_ptr<TreeNode>> stack;

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

            // 自己閉じタグの場合: <Node value="..." />
            if (line.rfind("<Node", 0) == 0 && line.find("/>") != std::string::npos) {
                size_t valPos = line.find("value=\"");
                if (valPos != std::string::npos) {
                    valPos += 7;
                    size_t valEnd = line.find("\"", valPos);
                    if (valEnd != std::string::npos) {
                        std::string val = unescapeXml(line.substr(valPos, valEnd - valPos));
                        auto newNode = std::make_shared<TreeNode>(val);

                        if (!root) {
                            root = newNode;
                        }
                        else if (!stack.empty()) {
                            stack.back()->addChild(newNode);
                        }
                    }
                }
            }
            // 閉じタグの場合: </Node>
            else if (line.rfind("</Node>", 0) == 0) {
                if (!stack.empty()) {
                    stack.pop_back();
                }
            }
            // 開始タグの場合: <Node value="...">
            else if (line.rfind("<Node", 0) == 0) {
                size_t valPos = line.find("value=\"");
                if (valPos != std::string::npos) {
                    valPos += 7;
                    size_t valEnd = line.find("\"", valPos);
                    if (valEnd != std::string::npos) {
                        std::string val = unescapeXml(line.substr(valPos, valEnd - valPos));
                        auto newNode = std::make_shared<TreeNode>(val);

                        if (!root) {
                            root = newNode;
                        }
                        else if (!stack.empty()) {
                            stack.back()->addChild(newNode);
                        }
                        stack.push_back(newNode);
                    }
                }
            }
        }

        return root;
    }
};
