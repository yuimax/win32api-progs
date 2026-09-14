#include <string>
#include <vector>
#include <sstream>

class TreeNode : public std::enable_shared_from_this<TreeNode> {
private:
    std::wstring m_value;
    std::weak_ptr<TreeNode> m_parent;
    std::vector<std::shared_ptr<TreeNode>> m_children;

    // 特殊文字の簡易エスケープ（XML用・ワイド文字版）
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

    // 特殊文字の簡易アンエスケープ（XML用・ワイド文字版）
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

    // シリアライズ用のヘルパー（再帰）
    static void serializeInternal(const std::shared_ptr<TreeNode>& node, int depth, std::wostringstream& woss) {
        if (!node) return;

        std::wstring indent(depth * 2, L' ');
        std::wstring escapedVal = escapeXml(node->getValue());

        if (node->getChildren().empty()) {
            woss << indent << L"<Node value=\"" << escapedVal << L"\" />\n";
        } else {
            woss << indent << L"<Node value=\"" << escapedVal << L"\">\n";
            for (const auto& child : node->getChildren()) {
                serializeInternal(child, depth + 1, woss);
            }
            woss << indent << L"</Node>\n";
        }
    }

public:
    // コンストラクタ
    explicit TreeNode(const std::wstring& value) : m_value(value) {}

    // 値の取得・設定
    const std::wstring& getValue() const { return m_value; }
    void setValue(const std::wstring& value) { m_value = value; }

    // 親ノードの取得
    std::shared_ptr<TreeNode> getParent() const { return m_parent.lock(); }

    // 子ノードのコレクションを取得
    const std::vector<std::shared_ptr<TreeNode>>& getChildren() const { return m_children; }

    // 子ノードの追加
    std::shared_ptr<TreeNode> addChild(const std::wstring& value) {
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
    std::wstring serialize() const {
        std::wostringstream woss;
        woss << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        serializeInternal(const_cast<TreeNode*>(this)->shared_from_this(), 0, woss);
        return woss.str();
    }

    // --- XMLデシリアライズ ---
    static std::shared_ptr<TreeNode> deserialize(const std::wstring& data) {
        std::wistringstream wiss(data);
        std::wstring line;

        std::shared_ptr<TreeNode> root = nullptr;
        std::vector<std::shared_ptr<TreeNode>> stack;

        while (std::getline(wiss, line)) {
            // 前後の空白トリム
            size_t start = line.find_first_not_of(L" \t\r\n");
            if (start == std::wstring::npos) continue;
            size_t end = line.find_last_not_of(L" \t\r\n");
            line = line.substr(start, end - start + 1);

            // XML宣言やコメントはスキップ
            if (line.rfind(L"<?", 0) == 0 || line.rfind(L"<!--", 0) == 0) {
                continue;
            }

            // 自己閉じタグの場合: <Node value="..." />
            if (line.rfind(L"<Node", 0) == 0 && line.find(L"/>") != std::wstring::npos) {
                size_t valPos = line.find(L"value=\"");
                if (valPos != std::wstring::npos) {
                    valPos += 7;
                    size_t valEnd = line.find(L"\"", valPos);
                    if (valEnd != std::wstring::npos) {
                        std::wstring val = unescapeXml(line.substr(valPos, valEnd - valPos));
                        auto newNode = std::make_shared<TreeNode>(val);

                        if (!root) {
                            root = newNode;
                        } else if (!stack.empty()) {
                            stack.back()->addChild(newNode);
                        }
                    }
                }
            }
            // 閉じタグの場合: </Node>
            else if (line.rfind(L"</Node>", 0) == 0) {
                if (!stack.empty()) {
                    stack.pop_back();
                }
            }
            // 開始タグの場合: <Node value="...">
            else if (line.rfind(L"<Node", 0) == 0) {
                size_t valPos = line.find(L"value=\"");
                if (valPos != std::wstring::npos) {
                    valPos += 7;
                    size_t valEnd = line.find(L"\"", valPos);
                    if (valEnd != std::wstring::npos) {
                        std::wstring val = unescapeXml(line.substr(valPos, valEnd - valPos));
                        auto newNode = std::make_shared<TreeNode>(val);

                        if (!root) {
                            root = newNode;
                        } else if (!stack.empty()) {
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
