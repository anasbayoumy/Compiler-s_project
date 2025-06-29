#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStyleFactory>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QProcess>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QPixmap>
#include <QTemporaryFile>
#include <QDir>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <iostream>
#include <regex>
#include <unordered_set>
#include <stack>
#include <map>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <filesystem>
#include <memory>
#include <sstream>
#include <queue>
#include <stdexcept>
#include <windows.h>
using namespace std;

// Node structure for the parse tree
struct ParseNode {
    string type;
    string value;
    vector<shared_ptr<ParseNode>> children;

    ParseNode(string t, string v = "") : type(t), value(v) {}
};

// Token structure
struct Token {
    string type;
    string value;
    int line;

    Token(string t, string v, int l) : type(t), value(v), line(l) {}
};

class PythonSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    enum Theme { Dark, Light };

    PythonSyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent), currentTheme(Dark) {
        updateHighlightingRules();
    }

    void setTheme(Theme theme) {
        if (currentTheme != theme) {
            currentTheme = theme;
            updateHighlightingRules();
            rehighlight(); // Reapply highlighting with new colors
        }
    }

private:
    void updateHighlightingRules() {
        highlightingRules.clear();
        HighlightingRule rule;

        // Colors adjusted for theme
        QColor defColor = (currentTheme == Dark) ? QColor("#ff55ff") : QColor("#ff00ff"); // Magenta, slightly darker for light theme
        QColor keywordColor = (currentTheme == Dark) ? QColor("#ffff55") : QColor("#ccaa00"); // Yellow, darker for light theme
        QColor builtinColor = (currentTheme == Dark) ? QColor("#ffaa00") : QColor("#cc8800"); // Orange, darker for light theme
        QColor numberColor = (currentTheme == Dark) ? QColor("#55ffff") : QColor("#0088cc"); // Cyan, darker for light theme
        QColor stringColor = (currentTheme == Dark) ? QColor("#ff3300") : QColor("#cc2200"); // Orange, darker for light theme
        QColor commentColor = (currentTheme == Dark) ? QColor("#00ff00") : QColor("#008800"); // Green, darker for light theme
        QColor symbolColor = (currentTheme == Dark) ? QColor("#aaaaaa") : QColor("#666666"); // Gray, darker for light theme
        QColor identifierColor = (currentTheme == Dark) ? QColor(Qt::white) : QColor(Qt::black); // White in dark, black in light
        QColor boolColor = (currentTheme == Dark) ? QColor("#5555ff") : QColor("#3333cc"); // Blue, darker for light theme
        QColor functionColor = (currentTheme == Dark) ? QColor("#ffff55") : QColor("#ccaa00"); // Yellow, darker for light theme
        QColor classColor = (currentTheme == Dark) ? QColor("#5555ff") : QColor("#3333cc"); // Blue, darker for light theme

        // 'def' keyword
        QTextCharFormat defFormat;
        defFormat.setForeground(defColor);
        defFormat.setFontWeight(QFont::Bold);
        rule.pattern = QRegularExpression("\\bdef\\b");
        rule.format = defFormat;
        highlightingRules.append(rule);

        // 'class' keyword
        QTextCharFormat keywordFormat;
        keywordFormat.setForeground(keywordColor);
        keywordFormat.setFontWeight(QFont::Bold);
        rule.pattern = QRegularExpression("\\bclass\\b");
        rule.format = keywordFormat;
        highlightingRules.append(rule);

        // Built-in functions
        QTextCharFormat builtinFormat;
        builtinFormat.setForeground(builtinColor);
        QStringList builtinPatterns;
        for (const auto& builtin : builtins) {
            builtinPatterns << QString("\\b%1\\b").arg(QString::fromStdString(builtin));
        }
        for (const QString &pattern : builtinPatterns) {
            rule.pattern = QRegularExpression(pattern);
            rule.format = builtinFormat;
            highlightingRules.append(rule);
        }

        // Other keywords (excluding 'def' and 'class')
        QStringList keywordPatterns;
        for (const auto& kw : keywords) {
            if (kw != "def" && kw != "class") {
                keywordPatterns << QString("\\b%1\\b").arg(QString::fromStdString(kw));
            }
        }
        for (const QString &pattern : keywordPatterns) {
            rule.pattern = QRegularExpression(pattern);
            rule.format = keywordFormat;
            highlightingRules.append(rule);
        }

        // Numbers
        QTextCharFormat numberFormat;
        numberFormat.setForeground(numberColor);
        rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
        rule.format = numberFormat;
        highlightingRules.append(rule);

        // Strings
        QTextCharFormat stringFormat;
        stringFormat.setForeground(stringColor);
        rule.pattern = QRegularExpression("\"[^\"]*\"|'[^']*'");
        rule.format = stringFormat;
        highlightingRules.append(rule);

        // Triple-quoted strings
        rule.pattern = QRegularExpression("(\"\"\".*?\"\"\")|('''.*?''')", QRegularExpression::DotMatchesEverythingOption);
        rule.format = stringFormat;
        highlightingRules.append(rule);

        // Comments
        QTextCharFormat commentFormat;
        commentFormat.setForeground(commentColor);
        rule.pattern = QRegularExpression("#[^\n]*");
        rule.format = commentFormat;
        highlightingRules.append(rule);

        // Symbols
        QTextCharFormat symbolFormat;
        symbolFormat.setForeground(symbolColor);
        rule.pattern = QRegularExpression("[\\(\\)\\{\\}\\[\\]\\=\\+\\-\\*\\/\\%\\:\\,\\.]");
        rule.format = symbolFormat;
        highlightingRules.append(rule);

        // Identifiers (last to avoid overriding)
        QTextCharFormat identifierFormat;
        identifierFormat.setForeground(identifierColor);
        rule.pattern = QRegularExpression("[A-Za-z_][A-Za-z0-9_]*");
        rule.format = identifierFormat;
        highlightingRules.append(rule);
    }

protected:
    void highlightBlock(const QString &text) override {
        // Apply all rules first
        for (const HighlightingRule &rule : highlightingRules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }

        // Re-process identifiers for function, class names, and specific keywords
        QRegularExpression identifierPattern("[A-Za-z_][A-Za-z0-9_]*");
        QRegularExpressionMatchIterator idIterator = identifierPattern.globalMatch(text);
        QStringList words = text.split(QRegularExpression("\\s+"));

        QColor boolColor = (currentTheme == Dark) ? QColor("#5555ff") : QColor("#3333cc");
        QColor functionColor = (currentTheme == Dark) ? QColor("#ffff55") : QColor("#ccaa00");
        QColor classColor = (currentTheme == Dark) ? QColor("#5555ff") : QColor("#3333cc");
        QColor defColor = (currentTheme == Dark) ? QColor("#ff55ff") : QColor("#ff00ff");

        while (idIterator.hasNext()) {
            QRegularExpressionMatch match = idIterator.next();
            int start = match.capturedStart();
            int length = match.capturedLength();
            QString id = match.captured();

            // Skip if already formatted as a keyword, built-in, number, or string
            QTextCharFormat currentFormat = format(start);
            QColor identifierColor = (currentTheme == Dark) ? Qt::white : Qt::black;
            if (currentFormat.foreground() != identifierColor) {
                continue;
            }

            // Find the word’s position in the split text to check context
            int wordIndex = -1;
            for (int i = 0; i < words.size(); ++i) {
                if (words[i] == id) {
                    wordIndex = i;
                    break;
                }
            }

            if (wordIndex >= 0) {
                QString word = words[wordIndex];
                QString prevWord = (wordIndex > 0) ? words[wordIndex - 1] : "";
                QString nextChar = (start + length < text.length()) ? text.mid(start + length, 1) : "";
                QString nextWord = (wordIndex + 1 < words.size()) ? words[wordIndex + 1] : "";

                // Color specific control flow keywords in magenta
                if (word == "def" || word == "if" || word == "elif" || word == "else" ||
                    word == "while" || word == "break" || word == "continue") {
                    QTextCharFormat keywordFormat;
                    keywordFormat.setForeground(defColor);
                    setFormat(start, length, keywordFormat);
                }
                // Color boolean literals in dark blue
                else if (word == "True" || word == "False" || word == "None") {
                    QTextCharFormat boolFormat;
                    boolFormat.setForeground(boolColor);
                    setFormat(start, length, boolFormat);
                }
                // Color function names (after 'def') in yellow
                else if (prevWord == "def") {
                    QTextCharFormat functionFormat;
                    functionFormat.setForeground(functionColor);
                    setFormat(start, length, functionFormat);
                }
                // Color class names (after 'class') in blue
                else if (prevWord == "class") {
                    QTextCharFormat classFormat;
                    classFormat.setForeground(classColor);
                    setFormat(start, length, classFormat);
                }
                // Color words followed by an open bracket (directly or after space) in yellow
                else if (nextChar == "(" || (nextWord == "(")) {
                    QTextCharFormat bracketFollowFormat;
                    bracketFollowFormat.setForeground(functionColor);
                    setFormat(start, length, bracketFollowFormat);
                }
            }
        }

        // Handle multi-line triple-quoted strings
        setCurrentBlockState(0);
        int startIndex = 0;
        if (previousBlockState() != 1) {
            startIndex = text.indexOf(QRegularExpression("(\"\"\"|''')"));
        }

        while (startIndex >= 0) {
            QRegularExpression endExpr(previousBlockState() == 1 ? QRegularExpression("(\"\"\"|''')") : QRegularExpression("(\"\"\"|''')"));
            QRegularExpressionMatch endMatch = endExpr.match(text, startIndex + 3);
            int endIndex = endMatch.capturedStart();
            int length;

            if (endIndex == -1) {
                setCurrentBlockState(1);
                length = text.length() - startIndex;
            } else {
                length = endIndex - startIndex + endMatch.capturedLength();
            }

            QTextCharFormat stringFormat;
            stringFormat.setForeground((currentTheme == Dark) ? QColor("#ff3300") : QColor("#cc2200"));
            setFormat(startIndex, length, stringFormat);
            startIndex = text.indexOf(QRegularExpression("(\"\"\"|''')"), startIndex + length);
        }
    }

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
    Theme currentTheme;

    static const unordered_set<string> keywords;
    static const unordered_set<string> builtins;
};

const unordered_set<string> PythonSyntaxHighlighter::keywords = {
    "False", "await", "else", "import", "pass", "None", "break", "except",
    "in", "raise", "True", "class", "finally", "is", "return", "and", "continue",
    "for", "lambda", "try", "as", "def", "from", "nonlocal", "while", "assert",
    "del", "global", "not", "with", "async", "elif", "if", "or", "yield"
};

const unordered_set<string> PythonSyntaxHighlighter::builtins = {
    "print", "len", "range", "int", "str", "float", "bool", "list", "dict", "set",
    "tuple", "abs", "max", "min", "sum", "open", "input", "type", "dir", "help"
};

class Parser {
private:
    vector<Token> tokens;
    size_t current = 0;
    int nodeCounter = 0;

    // Helper function to check if we've reached the end
    bool isAtEnd() {
        return current >= tokens.size();
    }

    // Helper function to peek at current token
    Token peek() {
        if (isAtEnd()) return Token("EOF", "", -1);
        return tokens[current];
    }

    // Helper function to peek ahead n positions
    Token peekAhead(size_t n = 1) {
        if (current + n >= tokens.size()) return Token("EOF", "", -1);
        return tokens[current + n];
    }

    // Helper function to check if current token matches expected type
    bool check(const string& type) {
        return !isAtEnd() && tokens[current].type == type;
    }

    // Helper function to advance and return previous token
    Token advance() {
        if (!isAtEnd()) current++;
        return tokens[current - 1];
    }

    // Helper function to match and consume token if it matches
    bool match(const vector<string>& types) {
        for (auto& t : types) {
            if (check(t)) {
                advance();
                return true;
            }
        }
        return false;
    }

    // Helper function to consume token of expected type
    Token consume(const string& type, const string& msg) {
        if (check(type)) return advance();
        throw runtime_error(msg + " (found '" + peek().type + ":" + peek().value + "' instead)");
    }

    shared_ptr<ParseNode> consumeNode(const string& expectedToken, const string& errorMessage) {
        if (!check(expectedToken)) {
            throw runtime_error(errorMessage);
        }
        auto token = advance();  // consume the token
        return make_shared<ParseNode>(expectedToken);  // create a node for the token
    }

    // Parse a file input
    shared_ptr<ParseNode> program() {
        auto node = make_shared<ParseNode>("program");
        node->children.push_back(stmt_list());
        if (!isAtEnd()) {
            Token t = consume("ENDMARKER", "Expected ENDMARKER");
            node->children.push_back(make_shared<ParseNode>("ENDMARKER", t.value));
        }
        return node;
    }

    // Parse a statement list
    shared_ptr<ParseNode> stmt_list() {
        auto node = make_shared<ParseNode>("stmt_list");
        while (!isAtEnd() && !check("ENDMARKER")) {
            if (check("NEWLINE")) { advance(); continue; }
            node->children.push_back(stmt());
            if (check("NEWLINE")) advance();
        }
        return node;
    }

    // Parse a statement
    shared_ptr<ParseNode> stmt() {
        if (isSimpleStmt()) return simple_stmts();
        return block_stmt();
    }

    // Check for simple statement
    bool isSimpleStmt() {
        if (check("NAME") || check("pass") || check("break") || check("continue") || check("return") || check("import") || check("from")) return true;
        return false;
    }

    // Parse simple statements
    shared_ptr<ParseNode> simple_stmts() {
        auto node = make_shared<ParseNode>("simple_stmts");
        node->children.push_back(small_stmt());
        while (match({";"})) {
            if (check("NEWLINE") || isAtEnd()) break;
            node->children.push_back(small_stmt());
        }
        return node;
    }

    // Parse a small statement
    shared_ptr<ParseNode> small_stmt() {
        if (check("NAME") && current + 1 < tokens.size() &&
            (tokens[current+1].type == "=" ||
             tokens[current+1].type == "+=" ||
             tokens[current+1].type == "-=" ||
             tokens[current+1].type == "*=" ||
             tokens[current+1].type == "/=" ||
             tokens[current+1].type == "%=" ||
             tokens[current+1].type == "//="))
            return assignment();

        if (check("pass") || check("break") || check("continue") || check("return"))
            return control_flow();

        if (check("import") || check("from"))
            return declaration();

        if (check("NAME"))
            return invocation();

        throw runtime_error("Unknown small statement type: " + peek().type);
    }

    // Parse a control flow statement
    shared_ptr<ParseNode> control_flow() {
        if (match({"pass"})) return make_shared<ParseNode>("pass_stmt");
        if (match({"break"})) return make_shared<ParseNode>("break_stmt");
        if (match({"continue"})) return make_shared<ParseNode>("continue_stmt");
        if (match({"return"})) {
            auto node = make_shared<ParseNode>("return_stmt");
            if (!check("NEWLINE") && !check(";")) node->children.push_back(expr());
            return node;
        }
        throw runtime_error("Unknown control flow statement");
    }

    // Parse a declaration statement
    shared_ptr<ParseNode> declaration() {
        if (match({"import"})) return import_decl();
        if (match({"from"})) return import_decl();
        throw runtime_error("Unknown declaration statement");
    }

    // Parse an import declaration
    shared_ptr<ParseNode> import_decl() {
        auto node = make_shared<ParseNode>("import_decl");
        if (tokens[current-1].type == "import") {
            node->children.push_back(module_ref());
            if (match({"as"})) {
                node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected NAME after 'as'").value));
            }
            while (match({","})) {
                node->children.push_back(module_ref());
                if (match({"as"})) {
                    node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected NAME after 'as'").value));
                }
            }
        } else if (tokens[current-1].type == "from") {
            node->children.push_back(module_ref());
            consume("import", "Expected 'import'");
            if (check("NAME")) {
                node->children.push_back(make_shared<ParseNode>("NAME", advance().value));
                if (match({"as"})) {
                    node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected NAME after 'as'").value));
                }
            } else if (match({"*"})) {
                node->children.push_back(make_shared<ParseNode>("*"));
            }
        }
        return node;
    }

    // Parse a module reference
    shared_ptr<ParseNode> module_ref() {
        auto node = make_shared<ParseNode>("module_ref");
        node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected module name").value));
        while (match({"."})) {
            node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected name after '.'").value));
        }
        return node;
    }

    // Parse an assignment statement
    shared_ptr<ParseNode> assignment() {
        auto node = make_shared<ParseNode>("assignment");
        node->children.push_back(targets());  // x
        node->children.push_back(assign_op());  // =
        node->children.push_back(exprs());  // a / b (with exprs as parent)
        return node;
    }

    // Parse targets
    shared_ptr<ParseNode> targets() {
        auto node = make_shared<ParseNode>("targets");
        node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected target name").value));
        while (match({","})) {
            node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected name after ','").value));
        }
        return node;
    }

    // Parse an assignment operator
    shared_ptr<ParseNode> assign_op() {
        if (match({"=", "+=", "-=", "*=", "/=", "%=", "//="})) {
            return make_shared<ParseNode>("assign_op", tokens[current-1].type);
        }
        throw runtime_error("Expected assignment operator");
    }

    // Parse expressions
    shared_ptr<ParseNode> exprs() {
        auto node = make_shared<ParseNode>("exprs");
        auto expr_node = expr();
        // Flatten the expression tree into exprs children
        if (expr_node->value.empty() && !expr_node->children.empty()) {
            // Operator expression (like a / b)
            for (auto& child : expr_node->children) {
                node->children.push_back(child);
            }
        } else {
            // Simple value (like NUMBER: 2)
            node->children.push_back(expr_node);
        }
        return node;
    }

    // Parse an invocation
    shared_ptr<ParseNode> invocation() {
        auto node = make_shared<ParseNode>("invocation");
        node->children.push_back(callable());
        node->children.push_back(consumeNode("(", "Expected '(' after function name"));  // now '(' is a node
        if (!check(")")) {
            node->children.push_back(arguments());  // optional arguments
        }
        node->children.push_back(consumeNode(")", "Expected ')' to close function call"));  // ')' node
        return node;
    }

    // Parse a callable
    shared_ptr<ParseNode> callable() {
        if (check("NAME")) {
            return make_shared<ParseNode>("NAME", advance().value);
        }
        return module_ref();
    }

    // Parse arguments
    shared_ptr<ParseNode> arguments() {
        auto node = make_shared<ParseNode>("arguments");
        node->children.push_back(expr());
        while (match({","})) {
            if (check(")")) break; // Handle trailing comma
            node->children.push_back(expr());
        }
        return node;
    }

    // Parse a block statement
    shared_ptr<ParseNode> block_stmt() {
        if (check("if")) return conditional();
        if (check("while")) return loop();
        if (check("for")) return loop();
        if (check("def")) return definition();
        if (check("class")) return definition();
        throw runtime_error("Unknown block statement type: " + peek().type);
    }

    // Parse a conditional statement
    shared_ptr<ParseNode> conditional() {
        auto node = make_shared<ParseNode>("conditional");
        node->children.push_back(if_chain());
        if (match({"else"})) {
            consume(":", "Expected ':' after else");
            node->children.push_back(suite());
        }
        return node;
    }

    // Parse an if chain
    shared_ptr<ParseNode> if_chain() {
        auto node = make_shared<ParseNode>("if_chain");
        consume("if", "Expected 'if'");
        node->children.push_back(comparison_expr());
        consume(":", "Expected ':' after condition");
        node->children.push_back(suite());
        while (match({"elif"})) {
            node->children.push_back(comparison_expr());
            consume(":", "Expected ':' after elif condition");
            node->children.push_back(suite());
        }
        return node;
    }

    // Special comparison expression handler for if statements
    shared_ptr<ParseNode> comparison_expr() {
        auto left = expr();
        cout << "In comparison_expr. Current token: " << (current < tokens.size() ? tokens[current].type : "EOF") << endl;
        if (current < tokens.size()) {
            string opType = tokens[current].type;
            cout << "Checking operator: " << opType << endl;
            if (opType == "==" || opType == "<" || opType == ">" ||
                opType == ">=" || opType == "<=" || opType == "!=" || opType == "=") {
                advance();
                auto op = make_shared<ParseNode>(opType);
                op->children.push_back(left);
                op->children.push_back(expr());
                return op;
            }
        }
        return left;
    }

    // Parse a loop statement
    shared_ptr<ParseNode> loop() {
        if (match({"while"})) {
            auto node = make_shared<ParseNode>("while_loop");
            node->children.push_back(expr());
            consume(":", "Expected ':' after while condition");
            node->children.push_back(suite());
            return node;
        }
        if (match({"for"})) {
            auto node = make_shared<ParseNode>("for_loop");
            node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected loop variable").value));
            consume("in", "Expected 'in' after loop variable");
            node->children.push_back(expr());
            consume(":", "Expected ':' after for loop iterable");
            node->children.push_back(suite());
            return node;
        }
        throw runtime_error("Unknown loop type");
    }

    // Parse a definition statement
    shared_ptr<ParseNode> definition() {
        if (match({"def"})) return func_def();
        if (match({"class"})) return class_def();
        throw runtime_error("Unknown definition type");
    }

    // Parse a function definition
    shared_ptr<ParseNode> func_def() {
        auto node = make_shared<ParseNode>("func_def");
        node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected function name").value));
        node->children.push_back(params());
        consume(":", "Expected ':' after function parameters");
        node->children.push_back(suite());
        return node;
    }

    // Parse function parameters
    shared_ptr<ParseNode> params() {
        consume("(", "Expected '(' after function name");
        auto node = make_shared<ParseNode>("params");
        if (!check(")")) {
            node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected parameter name").value));
            while (match({","})) {
                if (check(")")) break; // Handle trailing comma
                node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected parameter name").value));
            }
        }
        consume(")", "Expected ')' to close parameter list");
        return node;
    }

    // Parse a class definition
    shared_ptr<ParseNode> class_def() {
        auto node = make_shared<ParseNode>("class_def");
        node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected class name").value));
        if (match({"("})) {
            node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected parent class name").value));
            consume(")", "Expected ')' to close parent class list");
        }
        consume(":", "Expected ':' after class definition");
        node->children.push_back(suite());
        return node;
    }

    // Parse a suite (indented block)
    shared_ptr<ParseNode> suite() {
        auto node = make_shared<ParseNode>("suite");
        cout << "In suite. Current token: " << peek().type << " '" << peek().value << "'" << endl;
        if (check("INDENT")) {
            advance();
            while (!check("DEDENT") && !isAtEnd()) {
                node->children.push_back(stmt());
            }
            if (check("DEDENT")) advance();
            return node;
        }
        if (check("NEWLINE")) {
            advance();
            if (check("INDENT")) {
                advance();
                while (!check("DEDENT") && !isAtEnd()) {
                    node->children.push_back(stmt());
                }
                if (check("DEDENT")) advance();
                return node;
            }
        }
        return simple_stmts();
    }

    // Parse an expression
    shared_ptr<ParseNode> expr() {
        return logical_or();
    }

    // Parse a logical OR expression
    shared_ptr<ParseNode> logical_or() {
        auto node = logical_and();
        while (match({"or"})) {
            auto op = make_shared<ParseNode>("or");
            op->children.push_back(node);
            op->children.push_back(logical_and());
            node = op;
        }
        return node;
    }

    // Parse a logical AND expression
    shared_ptr<ParseNode> logical_and() {
        auto node = logical_not();
        while (match({"and"})) {
            auto op = make_shared<ParseNode>("and");
            op->children.push_back(node);
            op->children.push_back(logical_not());
            node = op;
        }
        return node;
    }

    // Parse a logical NOT expression
    shared_ptr<ParseNode> logical_not() {
        if (match({"not"})) {
            auto node = make_shared<ParseNode>("not");
            node->children.push_back(logical_not());
            return node;
        }
        return comparison();
    }

    // Parse a comparison
    shared_ptr<ParseNode> comparison() {
        auto node = arithmetic();
        while (match({"<", ">", "==", ">=", "<=", "!="})) {
            auto op = make_shared<ParseNode>(tokens[current-1].type);
            op->children.push_back(node);
            op->children.push_back(arithmetic());
            node = op;
        }
        return node;
    }

    // Parse an arithmetic expression
    shared_ptr<ParseNode> arithmetic() {
        auto node = term();
        while (match({"+", "-"})) {
            auto op_node = make_shared<ParseNode>("");
            op_node->children.push_back(node);
            op_node->children.push_back(make_shared<ParseNode>(tokens[current-1].type));
            op_node->children.push_back(term());
            node = op_node;
        }
        return node;
    }

    // Parse a term
    shared_ptr<ParseNode> term() {
        auto node = factor();
        while (match({"*", "/", "//", "%"})) {
            auto op_node = make_shared<ParseNode>("");
            op_node->children.push_back(node);
            op_node->children.push_back(make_shared<ParseNode>(tokens[current-1].type));
            op_node->children.push_back(factor());
            node = op_node;
        }
        return node;
    }

    // Parse a factor
    shared_ptr<ParseNode> factor() {
        if (match({"+", "-", "~"})) {
            auto node = make_shared<ParseNode>(tokens[current-1].type);
            node->children.push_back(factor());
            return node;
        }
        return primary();
    }

    // Parse a primary expression
    shared_ptr<ParseNode> primary() {
        if (check("NUMBER")) return make_shared<ParseNode>("NUMBER", advance().value);
        if (check("BOOL")) return make_shared<ParseNode>("BOOL", advance().value);
        if (check("STRING")) return make_shared<ParseNode>("STRING", advance().value);
        if (check("None") || check("True") || check("False")) return make_shared<ParseNode>(advance().type);
        if (check("NAME")) return make_shared<ParseNode>("NAME", advance().value);
        if (check("(")) return grouped();
        if (check("[")) return list_();
        if (check("{")) return dict_();
        throw runtime_error("Unknown primary expression type");
    }

    // Parse a grouped expression
    shared_ptr<ParseNode> grouped() {
        consume("(", "Expected '('");
        auto node = make_shared<ParseNode>("grouped");
        if (!check(")")) node->children.push_back(expr_list());
        consume(")", "Expected ')'");
        return node;
    }

    // Parse a list expression
    shared_ptr<ParseNode> list_() {
        consume("[", "Expected '['");
        auto node = make_shared<ParseNode>("list");
        if (!check("]")) node->children.push_back(expr_list());
        consume("]", "Expected ']'");
        return node;
    }

    // Parse a dictionary expression
    shared_ptr<ParseNode> dict_() {
        consume("{", "Expected '{'");
        auto node = make_shared<ParseNode>("dict");
        if (!check("}")) node->children.push_back(key_values());
        consume("}", "Expected '}'");
        return node;
    }

    // Parse an expression list
    shared_ptr<ParseNode> expr_list() {
        auto node = make_shared<ParseNode>("expr_list");
        node->children.push_back(expr());
        while (match({","})) {
            if (check(")") || check("]")) break; // Handle trailing comma
            node->children.push_back(expr());
        }
        return node;
    }

    // Parse key-value pairs
    shared_ptr<ParseNode> key_values() {
        auto node = make_shared<ParseNode>("key_values");
        node->children.push_back(expr());
        consume(":", "Expected ':' after dictionary key");
        node->children.push_back(expr());
        while (match({","})) {
            if (check("}")) break; // Handle trailing comma
            node->children.push_back(expr());
            consume(":", "Expected ':' after dictionary key");
            node->children.push_back(expr());
        }
        return node;
    }

public:
    // Generate DOT representation of the parse tree
    void generateDOT(shared_ptr<ParseNode> node, ostream& dotFile, string parent = "") {
        if (!node) return;
        string nodeId = "node" + to_string(nodeCounter++);

        // Determine if the node is a leaf (no children)
        bool isLeaf = node->children.empty();

        // Enhanced node styling, with different color for leaf nodes
        dotFile << nodeId << " [" << "label=\"" << node->type;
        if (!node->value.empty()) dotFile << "\\n" << node->value;
        dotFile << "\", " << "shape=box, " << "style=\"rounded,filled\", " << "fillcolor=\"" << (isLeaf ? "#90EE90" : "#f0f0f0") << "\", "
                << "fontname=\"Arial\", " << "fontsize=11, " << "penwidth=1.0" << "];\n";

        if (!parent.empty()) {
            dotFile << parent << " -> " << nodeId << " [penwidth=1.0, arrowsize=0.8];\n";
        }

        for (auto& child : node->children) {
            generateDOT(child, dotFile, nodeId);
        }
    }

    bool create_Tree(const string& dot_file_name, const string& image_file_name) {
        // Command to convert DOT to high-resolution PNG using Graphviz
        string command = "dot -Tpng -Gdpi=300 " + dot_file_name + " -o " + image_file_name;

        int result = system(command.c_str());

        if (result == 0) {
            cout << "High-resolution image generated successfully: " << image_file_name << endl;
            return true;
        } else {
            cerr << "Failed to generate image. Is Graphviz installed?" << endl;
            return false;
        }
    }

    string processDotLabels(const string& dotContent) {
        istringstream iss(dotContent);
        ostringstream oss;
        string line;

        while (getline(iss, line)) {
            size_t label_pos = line.find("label=\"\"");
            if (label_pos != string::npos) {
                line.replace(label_pos, 8, "label=\"arith_expr\"");
            }
            oss << line << "\n";
        }

        return oss.str();
    }

    void loadTokens(const string& filename) {
        ifstream file(filename);
        string line;
        regex lineRegex(R"(\[(\d+)\])");
        regex tokenRegex(R"(<([^;]+);\s*([^>]*)>)");
        smatch matches;

        if (!file.is_open()) {
            throw runtime_error("Could not open tokens file: " + filename);
        }

        cout << "Successfully opened token file: " + filename << endl;

        while (getline(file, line)) {
            if (line.empty()) continue;

            if (regex_search(line, matches, lineRegex)) {
                int lineNum = stoi(matches[1]);
                string::const_iterator searchStart = line.cbegin();  // Start iterator

                while (regex_search(searchStart, line.cend(), matches, tokenRegex)) {
                    string type = matches[1];
                    string value = matches[2];

                    cout << "Token: " << type << " = " << value << " (line " << lineNum << ")" << endl;

                    if (type == "id") type = "NAME";
                    else if (type == "int" || type == "float") type = "NUMBER";
                    else if (type == "string") type = "STRING";
                    else if (type == "bool") type = "BOOL";
                    else if (type == "keyword") type = value;
                    else if (type == "symbol") {
                        value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));
                        value.erase(value.find_last_not_of(" \t\n\r\f\v") + 1);

                        if (value == ">") type = ">";
                        else if (value == "<") type = "<";
                        else if (value == "==") type = "==";
                        else if (value == ">=") type = ">=";
                        else if (value == "<=") type = "<=";
                        else if (value == "!=") type = "!=";
                        else if (value == "=") type = "=";
                        else if (value == "greater") type = ">";
                        else type = value;
                    }
                    else if (type == "Function") type = "NAME";
                    else if (type == "indent") type = "INDENT";
                    else if (type == "dedent") type = "DEDENT";
                    else if (type == "newline") type = "NEWLINE";

                    if (type == " " || type.empty()) {
                        searchStart = matches.suffix().first;
                        continue;
                    }

                    tokens.push_back(Token(type, value, lineNum));
                    searchStart = matches.suffix().first;
                }
            }
        }

        if (tokens.empty()) {
            throw runtime_error("No tokens found in file");
        }

        cout << "Total tokens loaded: " << tokens.size() << endl;
    }

    shared_ptr<ParseNode> parse() {
        try {
            current = 0;
            auto root = program();
            if (!root) {
                throw runtime_error("Failed to parse program: root node is null");
            }
            return root;
        } catch (const exception& e) {
            cerr << "Parse error: " << e.what() << endl;
            if (current < tokens.size()) {
                cerr << "Current token: type=" << tokens[current].type
                     << ", value='" << tokens[current].value
                     << "', line=" << tokens[current].line << endl;
            } else {
                cerr << "Reached end of tokens" << endl;
            }
            return nullptr;
        }
    }

    void createDirectoryIfNotExists(const string& path) {
        size_t pos = 0;
        do {
            pos = path.find_first_of("\\/", pos + 1);
            string subdir = path.substr(0, pos);
            if (!subdir.empty()) {
                CreateDirectoryA(subdir.c_str(), NULL);
            }
        } while (pos != string::npos);
    }

    void generateDOTFile(shared_ptr<ParseNode> root, const string& filename) {
        size_t lastSlash = filename.find_last_of("\\/");
        if (lastSlash != string::npos) {
            string dir = filename.substr(0, lastSlash);
            createDirectoryIfNotExists(dir);
        }

        ofstream dotFile(filename);
        if (!dotFile.is_open()) {
            throw runtime_error("Could not create DOT file: " + filename);
        }

        dotFile << "digraph ParseTree {\n"
                << "node [shape=box];\n"
                << "graph [\n"
                << "  rankdir=TB,\n"
                << "  bgcolor=\"#ffffff\",\n"
                << "  fontname=\"Arial\",\n"
                << "  fontsize=12,\n"
                << "  labeljust=l,\n"
                << "  labelloc=t,\n"
                << "  splines=ortho\n"
                << "];\n"
                << "edge [\n"
                << "  arrowsize=0.8,\n"
                << "  color=\"#666666\",\n"
                << "  fontname=\"Arial\",\n"
                << "  fontsize=10\n"
                << "];\n";

        nodeCounter = 0;
        generateDOT(root, dotFile);
        dotFile << "}\n";
        dotFile.close();

        cout << "DOT file generated: " << filename << endl;
    }
};

string output[500];
int outputIndex = 0;

unordered_set<string> keywords = {
    "False", "await", "else", "import", "pass", "None", "break", "except",
    "in", "raise", "True", "class", "finally", "is", "return", "and", "continue",
    "for", "lambda", "try", "as", "def", "from", "nonlocal", "while", "assert",
    "del", "global", "not", "with", "async", "elif", "if", "or", "yield"
};

bool isIdentifier(const string& word) {
    return regex_match(word, regex("[A-Za-z_][A-Za-z0-9_]*"));
}

bool isNumber(const string& word) {
    return regex_match(word, regex("\\d+(\\.\\d+)?"));
}

void storeOutput(const string& msg) {
    if (outputIndex < 500)
        if(msg.size() > 0)
            output[outputIndex++] = msg;
}

void processToken(const string& token, int lineNumber) {
    if (keywords.count(token))
        storeOutput("Line " + to_string(lineNumber) + " - Keyword: " + token);
    else if (isNumber(token))
        storeOutput("Line " + to_string(lineNumber) + " - Number: " + token);
    else if (regex_match(token, regex("\\d+[A-Za-z_]+[A-Za-z0-9_]*")))
        storeOutput("Line " + to_string(lineNumber) + " - Error Invalid Identifier: " + token);
    else if (isIdentifier(token))
        storeOutput("Line " + to_string(lineNumber) + " - Identifier: " + token);
    else
        storeOutput("Line " + to_string(lineNumber) + " - Unknown: " + token);
}

bool isCommentLine(const string& line) {
    for (char ch : line) {
        if (isspace(ch)) continue;
        return ch == '#';
    }
    return false;
}

bool handleTripleQuote(ifstream& file, string& currentLine, size_t& i, int& lineNumber, char quoteType) {
    string triple = string(3, quoteType);
    string str = triple;
    i += 2;

    string line;
    while (getline(file, line)) {
        ++lineNumber;
        str += "\n" + line;
        size_t pos = line.find(triple);
        if (pos != string::npos) {
            str += triple;
            storeOutput("Line " + to_string(lineNumber) + " - String: " + str);
            return true;
        }
    }
    storeOutput("Line " + to_string(lineNumber) + " - Syntax Error: Unterminated multi-line string");
    return false;
}

void handleIndentation(const string& line, int lineNumber, stack<int>& indentLevels) {
    int spaces = 0;
    for (char c : line) {
        if (c == ' ') spaces++;
        else break;
    }
    if (spaces % 4 != 0) {
        storeOutput("Line " + to_string(lineNumber) + " - Indentation Error: Not a multiple of 4");
        return;
    }
    int currentIndent = indentLevels.top();
    if (spaces > currentIndent) {
        indentLevels.push(spaces);
        storeOutput("Line " + to_string(lineNumber) + " - INDENT");
    } else if (spaces < currentIndent) {
        while (spaces < indentLevels.top()) {
            indentLevels.pop();
            storeOutput("Line " + to_string(lineNumber) + " - DEDENT");
        }
    }
}

void analyzeLine(string& line, int lineNumber, stack<char>& brackets, ifstream& file) {
    string word;
    for (size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];
        if ((ch == '"' || ch == '\'') && i + 2 < line.size() &&
            line[i + 1] == ch && line[i + 2] == ch) {
            if (!word.empty()) {
                processToken(word, lineNumber);
                word.clear();
            }
            break;
        }
        if (ch == '"' || ch == '\'') {
            if (!word.empty()) {
                processToken(word, lineNumber);
                word.clear();
            }
            char quote = ch;
            string strToken;
            strToken.push_back(ch);
            size_t start = i;
            ++i;
            bool terminated = false;
            while (i < line.size()) {
                char c = line[i];
                strToken.push_back(c);
                if (c == quote && line[i - 1] != '\\') {
                    terminated = true;
                    break;
                }
                ++i;
            }
            if (terminated) {
                storeOutput("Line " + to_string(lineNumber) + " - String: " + strToken);
            } else {
                storeOutput("Line " + to_string(lineNumber) + " - Syntax Error: Unterminated string: " + strToken);
            }
            continue;
        }
        if (isalnum(ch) || ch == '_') {
            word.push_back(ch);
        } else {
            if (!word.empty()) {
                processToken(word, lineNumber);
                word.clear();
            }
            if (ch == '(' || ch == '{' || ch == '[') {
                brackets.push(ch);
                storeOutput("Line " + to_string(lineNumber) + " - Symbol (opening bracket): " + ch);
            } else if (ch == ')' || ch == '}' || ch == ']') {
                if (brackets.empty() ||
                    (ch == ')' && brackets.top() != '(') ||
                    (ch == '}' && brackets.top() != '{') ||
                    (ch == ']' && brackets.top() != '[')) {
                    storeOutput("Line " + to_string(lineNumber) + " - Syntax Error: Mismatched bracket '" + string(1, ch));
                } else {
                    brackets.pop();
                    storeOutput("Line " + to_string(lineNumber) + " - Symbol (closing bracket): " + string(1, ch));
                }
            } else if (!isspace(ch) && !isalnum(ch) && ch != '_') {
                storeOutput("Line " + to_string(lineNumber) + " - Symbol: " + string(1, ch));
            }
        }
    }
    if (!word.empty())
        processToken(word, lineNumber);
    bool hasCloser = false;
    for (char ch : line) {
        if (ch == ')' || ch == '}' || ch == ']') {
            hasCloser = true;
            break;
        }
    }
    if (!hasCloser && !brackets.empty()) {
        storeOutput("Line " + to_string(lineNumber) + " - Syntax Error: Unmatched opening bracket(s)");
        while (!brackets.empty()) brackets.pop();
    }
}

void printOutput() {
    for (int i = 0; i < outputIndex; ++i) {
        cout << output[i] << '\n';
    }
}

string to_lower(string s) {
    for (char& c : s) c = tolower(c);
    return s;
}

struct SymbolInfo {
    string name;
    string type = "N/A";
    string value = "N/A";
};

map<string, SymbolInfo> symbolTable;

vector<string> sanitize_tokens_vector(const vector<string>& token_lines) {
    vector<string> sanitized_tokens;

    regex malformed_double_assign(R"(<symbol;\s*=>\s*>\s*<symbol;\s*=>\s*>)");
    regex float_parts(R"(<number;\s*(\d+)\s*> <symbol;\s*\.{1}\s*> <number;\s*(\d+)\s*>)");
    regex func_call(R"(<id;\s*([^>]+)\s*>\s*<symbol;\s*\(\s*>)");
    regex number_token(R"(<number;\s*(\d+)\s*>)");
    regex math_expr(R"(<(id|number|float|int);\s*([^>]+)\s*> <symbol;\s*([+\-*/])\s*> <(id|number|float|int);\s*([^>]+)\s*>)");

    for (string line : token_lines) {
        line = regex_replace(line, malformed_double_assign, "<symbol; ==>");

        // Handle math expressions
        smatch math_match;
        while (regex_search(line, math_match, math_expr)) {
            string left_type = math_match[1], left_val = math_match[2];
            string op = math_match[3];
            string right_type = math_match[4], right_val = math_match[5];

            if (left_type == "id" && symbolTable.count(left_val))
                left_val = symbolTable[left_val].value, left_type = symbolTable[left_val].type;
            if (right_type == "id" && symbolTable.count(right_val))
                right_val = symbolTable[right_val].value, right_type = symbolTable[right_val].type;

            double left = (left_type == "float") ? stod(left_val) : stoi(left_val);
            double right = (right_type == "float") ? stod(right_val) : stoi(right_val);
            double result = 0;
            string result_type = "int";

            if (op == "+") result = left + right;
            else if (op == "-") result = left - right;
            else if (op == "*") result = left * right;
            else if (op == "/") {
                if (right == 0) break;
                result = left / right;
                result_type = "float";
            }

            if (result_type == "int" && result != int(result))
                result_type = "float";

            string result_token = (result_type == "int")
                                      ? "<int; " + to_string(int(result)) + ">"
                                      : "<float; " + to_string(result) + ">";

            line.replace(math_match.position(0), math_match.length(0), result_token);
        }

        // Handle floats like 2 . 4 → <float; 2.4>
        smatch float_match;
        while (regex_search(line, float_match, float_parts)) {
            string fullFloat = float_match[1].str() + "." + float_match[2].str();
            line.replace(float_match.position(0), float_match.length(0), "<float; " + fullFloat + ">");
        }

        // Replace numbers with ints
        smatch number_match;
        while (regex_search(line, number_match, number_token)) {
            string intVal = number_match[1].str();
            line.replace(number_match.position(0), number_match.length(0), "<int; " + intVal + ">");
        }

        // Replace function names
        smatch func_match;
        while (regex_search(line, func_match, func_call)) {
            string funcName = func_match[1].str();
            string replacement = "<Function; " + funcName + ">";
            size_t pos = line.find("<id; " + funcName + ">", func_match.position(0));
            if (pos != string::npos)
                line.replace(pos, 8 + funcName.size() + 1, replacement);
        }

        // List detection (like: <id; pp> <symbol; => <symbol; [> <int; 1> <symbol; ,> ... <symbol; ]>)
        smatch list_match;
        regex list_expr(R"(<id;\s*(\w+)\s*>\s*<symbol;\s*=>\s*<symbol;\s*\[>\s*((?:<(int|float);\s*[^>]+>\s*(?:<symbol;\s*,>\s*)?)*)<symbol;\s*\]>)");

        // Save line number prefix like "[1] "
        string line_number_prefix = "";
        size_t bracket_pos = line.find(']');
        if (bracket_pos != string::npos && line[0] == '[') {
            line_number_prefix = line.substr(0, bracket_pos + 1) + " ";
            line = line.substr(bracket_pos + 2); // Skip "] "
        }

        if (regex_search(line, list_match, list_expr)) {
            string var = list_match[1];
            string inner = list_match[2];

            // Extract all numbers
            regex item(R"(<(int|float);\s*([^>]+)\s*>)");
            smatch m;
            string items = "[";
            string temp = inner;
            bool first = true;
            while (regex_search(temp, m, item)) {
                if (!first) items += ",";
                items += m[2].str();
                temp = m.suffix();
                first = false;
            }
            items += "]";

            line = "<id; " + var + "> <symbol; => <list; " + items + ">";
            symbolTable[var] = SymbolInfo{var, "list", items};
        }

        // Reattach line number
        line = line_number_prefix + line;

        // Assignment handling for int/float
        smatch assign_match;
        if (regex_search(line, assign_match, regex(R"(<id;\s*(\w+)\s*>\s*<symbol;\s*=>\s*<(int|float);\s*([\d\.]+)\s*>)"))) {
            string var = assign_match[1];
            string type = assign_match[2];
            string val = assign_match[3];
            symbolTable[var] = SymbolInfo{var, type, val};
        }

        // Replace " with '
        size_t pos;
        while ((pos = line.find('"')) != string::npos)
            line.replace(pos, 1, "'");

        // Fix malformed >ymbol
        string from1 = ">ymbol;";
        string to1 = "> <symbol;";
        size_t pos1 = line.find(from1);
        if (pos1 != string::npos)
            line.replace(pos1, from1.length(), to1);

        // Handle bools
        line = regex_replace(
            line,
            regex(R"(<id;\s*(\w+)\s*>\s*<symbol;\s*=>\s*<keyword;\s*(True|False)\s*>)"),
            "<id; $1> <symbol; => <bool; $2>"
            );

        sanitized_tokens.push_back(line);
    }

    // Add dedent if needed
    if (!sanitized_tokens.empty() && sanitized_tokens.back().find("<indent; indent>") != string::npos) {
        string last_line = "[" + to_string(sanitized_tokens.size() + 2) + "] <dedent; dedent>";
        sanitized_tokens.push_back(last_line);
    }

    return sanitized_tokens;
}

vector<string> parse_token_lines(const string arr[], int size) {
    vector<string> result;
    int current_line = -1;
    string current_tokens = "";

    map<string, string> token_map = {
        {"Keyword", "keyword"},
        {"Identifier", "id"},
        {"Symbol", "symbol"},
        {"Symbol (opening bracket)", "symbol"},
        {"Symbol (closing bracket)", "symbol"},
        {"Number", "number"},
        {"String", "string"},
        {"INDENT", "indent"},
        {"DEDENT", "dedent"},
        {"Syntax Error", "error"},
        {"Error Invalid Identifier", "error"}
    };

    regex full_regex(R"(Line (\d+) - ([^:]+): (.+))");
    regex short_regex(R"(Line (\d+) - (.+))");

    string line;
    for (int i = 0; i < size; ++i) {
        line = arr[i];
        if (line.empty()) continue;

        smatch match;
        int line_num = -1;
        string token_type, token_value;

        if (regex_match(line, match, full_regex)) {
            line_num = stoi(match[1]);
            token_type = match[2];
            token_value = match[3];
        } else if (regex_match(line, match, short_regex)) {
            line_num = stoi(match[1]);
            token_type = match[2];
            token_value = "";
        } else {
            continue;
        }

        if (current_line != line_num) {
            if (current_line != -1) {
                result.push_back("[" + to_string(current_line) + "] " + current_tokens);
            }
            current_line = line_num;
            current_tokens = "";
        }

        if (token_type == "INDENT" || token_type == "DEDENT") {
            current_tokens += "<" + to_lower(token_type) + "; " + to_lower(token_type) + "> ";
        } else if (token_type.find("Error") != string::npos) {
            current_tokens += "<error; " + token_value + "> ";
        } else {
            string category = token_map.count(token_type) ? token_map[token_type] : "unknown";
            current_tokens += "<" + category + "; " + token_value + "> ";
        }
    }

    if (!current_tokens.empty()) {
        result.push_back("[" + to_string(current_line) + "] " + current_tokens);
    }

    // Post-processing
    for (string& line : result) {
        // Replace double quotes with single quotes
        size_t pos2;
        while ((pos2 = line.find('"')) != string::npos) {
            line.replace(pos2, 1, "'");
        }

        // Convert boolean assignments
        line = regex_replace(
            line,
            regex(R"(<id;\s*(\w+)\s*>\s*<symbol;\s*=>\s*<keyword;\s*(True|False)\s*>)"),
            "<id; $1> <symbol; => <bool; $2>"
            );

        // Convert numbers to int or float
        line = regex_replace(
            line,
            regex(R"(<number;\s*([+-]?\d+)\s*>)"),
            "<int; $1>"
            );
        line = regex_replace(
            line,
            regex(R"(<number;\s*([+-]?\d*\.\d+)\s*>)"),
            "<float; $1>"
            );
    }

    return result;
}

void build_and_draw_symbol_table(const vector<string>& token_lines) {
    map<string, SymbolInfo> symbol_map;
    regex id_pattern(R"(<id;\s*([^>]+)\s*>)");
    regex assign_pattern(R"(<symbol;\s*=>\s*>)");
    regex value_pattern(R"(<(float|int|string|id|bool|list);\s*([^>]+)\s*>)");
    regex direct_assignment_pattern(R"(<id;\s*(\w+)\s*>\s*<symbol;\s*=>\s*<(int|float|string|bool|list);\s*([^>]+)\s*>)");

    for (const string& line : token_lines) {
        if (line.find("<Function;") != string::npos) continue;

        smatch assign_match;
        if (regex_search(line, assign_match, direct_assignment_pattern)) {
            string id = assign_match[1];
            string type = assign_match[2];
            string value = assign_match[3];
            symbol_map[id] = SymbolInfo{id, type, value};
        }

        sregex_iterator it(line.begin(), line.end(), id_pattern);
        sregex_iterator end;

        while (it != end) {
            string id = (*it)[1];
            if (symbol_map.count(id) == 0) {
                symbol_map[id] = SymbolInfo{id};
            }
            size_t id_pos = it->position();
            size_t eq_pos = line.find("<symbol; =>", id_pos);
            if (eq_pos != string::npos) {
                smatch match;
                string rest = line.substr(eq_pos);
                if (regex_search(rest, match, value_pattern)) {
                    symbol_map[id].type = match[1];
                    symbol_map[id].value = match[2];
                }
            }
            ++it;
        }
    }
    symbolTable = symbol_map;
}

ifstream flatten_multiline_file(ifstream& inputFile) {
    vector<string> result;
    bool in_multiline = false;
    string multiline_delim;
    string combined;

    string line;
    while (getline(inputFile, line)) {
        if (!in_multiline) {
            size_t pos_triple_double = line.find("\"\"\"");
            size_t pos_triple_single = line.find("'''");
            if (pos_triple_double != string::npos || pos_triple_single != string::npos) {
                in_multiline = true;
                multiline_delim = (pos_triple_double != string::npos) ? "\"\"\"" : "'''";
                combined = line;
                size_t end_pos = line.find(multiline_delim, (pos_triple_double != string::npos ? pos_triple_double + 3 : pos_triple_single + 3));
                if (end_pos != string::npos && end_pos > (pos_triple_double != string::npos ? pos_triple_double + 2 : pos_triple_single + 2)) {
                    in_multiline = false;
                    while (combined.find(multiline_delim) != string::npos)
                        combined.replace(combined.find(multiline_delim), 3, "\"");
                    result.push_back(combined);
                }
            } else {
                result.push_back(line);
            }
        } else {
            combined += " " + line;
            if (line.find(multiline_delim) != string::npos) {
                in_multiline = false;
                while (combined.find(multiline_delim) != string::npos)
                    combined.replace(combined.find(multiline_delim), 3, "\"");
                result.push_back(combined);
            }
        }
    }

    ofstream tempOut("temp_flattened.py");
    for (const string& l : result) {
        tempOut << l << "\n";
    }
    tempOut.close();

    return ifstream("temp_flattened.py");
}

void saveTokensToFile(const vector<string>& tokens) {
    ofstream outFile("A:\\MICHAEL\\Projects\\Design of Compilers\\Fidj\\CompilersQtGUI\\Tokens1.txt");
    if (!outFile) {
        cerr << "Error: Could not open Tokens.txt for writing.\n";
        return;
    }
    cout << "Saving " << tokens.size() << " tokens to Tokens.txt\n";
    for (const string& token : tokens) {
        cout << "Writing token: " << token << endl;
        outFile << token << '\n';
    }
    outFile.close();
}

class LexerAnalyzerWindow : public QMainWindow {
    Q_OBJECT

public:
    LexerAnalyzerWindow(QWidget *parent = nullptr) : QMainWindow(parent), isDarkTheme(true) {
        setWindowTitle("C++ Lexer Analyzer");
        resize(800, 600);

        qApp->setStyle(QStyleFactory::create("Fusion"));
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        qApp->setPalette(darkPalette);

        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *openButton = new QPushButton("OPEN FILE", this);
        QPushButton *saveButton = new QPushButton("SAVE", this);
        QPushButton *clearButton = new QPushButton("CLEAR", this);
        QPushButton *analyzeButton = new QPushButton("ANALYZE CODE", this);
        analyzeButton->setStyleSheet("background-color: #4282da; color: #ffffff; padding: 5px;");
        themeButton = new QPushButton("SWITCH TO LIGHT THEME", this);
        themeButton->setObjectName("themeButton");
        themeButton->setStyleSheet("background-color: #4282da; color: #ffffff; padding: 5px;");
        buttonLayout->addWidget(openButton);
        buttonLayout->addWidget(saveButton);
        buttonLayout->addWidget(clearButton);
        buttonLayout->addWidget(analyzeButton);
        buttonLayout->addStretch();
        buttonLayout->addWidget(themeButton);

        QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
        splitter->setStyleSheet("QSplitter::handle { background-color: #444444; }");

        QWidget *editorWidget = new QWidget(this);
        QVBoxLayout *editorLayout = new QVBoxLayout(editorWidget);
        QLabel *editorLabel = new QLabel("CODE EDITOR", this);
        editorLabel->setStyleSheet("font-weight: bold; color: #ffffff; font-size: 15px");
        codeEditor = new QTextEdit(this);
        codeEditor->setPlaceholderText("Enter Python code here...");
        codeEditor->setObjectName("codeEditor");
        codeEditor->setStyleSheet("background-color: #2d2d2d; color: #ffffff; border: 1px solid #555555; font-family: 'Courier New'; font-size: 14px; letter-spacing: 3.5px;");
        // Apply syntax highlighter
        highlighter = new PythonSyntaxHighlighter(codeEditor->document());
        editorLayout->addWidget(editorLabel);
        editorLayout->addWidget(codeEditor);

        QWidget *resultsWidget = new QWidget(this);
        QVBoxLayout *resultsLayout = new QVBoxLayout(resultsWidget);
        QLabel *resultsLabel = new QLabel("ANALYSIS RESULTS", this);
        resultsLabel->setStyleSheet("font-weight: bold; color: #ffffff;");
        tabWidget = new QTabWidget(this);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setStyleSheet("QTabWidget::pane { border: 1px solid #555555; background-color: #2d2d2d; } "
                                 "QTabBar::tab { background-color: #3c3c3c; color: #ffffff; padding: 5px; } "
                                 "QTabBar::tab:selected { background-color: #4282da; }");
        tokensTab = new QWidget();
        tokensText = new QTextEdit(tokensTab);
        tokensText->setReadOnly(true);
        tokensText->setPlaceholderText("Lexical tokens will appear here...");
        tokensText->setObjectName("tokensText");
        tokensText->setStyleSheet("background-color: #2d2d2d; color: #ffffff; border: none; font-family: 'Courier New';");
        QVBoxLayout *tokensLayout = new QVBoxLayout(tokensTab);
        tokensLayout->addWidget(tokensText);
        identifiersTab = new QWidget();
        identifiersText = new QTextEdit(identifiersTab);
        identifiersText->setReadOnly(true);
        identifiersText->setPlaceholderText("Identifier table will appear here...");
        identifiersText->setObjectName("identifiersText");
        identifiersText->setStyleSheet("background-color: #2d2d2d; color: #ffffff; border: none; font-family: 'Courier New';");
        QVBoxLayout *identifiersLayout = new QVBoxLayout(identifiersTab);
        identifiersLayout->addWidget(identifiersText);
        treeTab = new QWidget();
        treeView = new QGraphicsView(treeTab);
        treeScene = new QGraphicsScene(treeView);
        treeView->setScene(treeScene);
        treeView->setRenderHint(QPainter::Antialiasing);
        treeView->setObjectName("treeView");
        treeView->setStyleSheet("background-color: #2d2d2d; color: #ffffff; border: none; font-family: 'Courier New';");
        QVBoxLayout *treeLayout = new QVBoxLayout(treeTab);
        treeLayout->addWidget(treeView);

        QHBoxLayout* zoomLayout = new QHBoxLayout();
        QPushButton* zoomInButton = new QPushButton("+", treeTab);
        QPushButton* zoomOutButton = new QPushButton("-", treeTab);
        QPushButton* resetZoomButton = new QPushButton("Reset", treeTab);

        zoomInButton->setFixedSize(30, 30);
        zoomOutButton->setFixedSize(30, 30);
        resetZoomButton->setFixedSize(60, 30);

        connect(zoomInButton, &QPushButton::clicked, this, &LexerAnalyzerWindow::zoomIn);
        connect(zoomOutButton, &QPushButton::clicked, this, &LexerAnalyzerWindow::zoomOut);
        connect(resetZoomButton, &QPushButton::clicked, this, &LexerAnalyzerWindow::resetZoom);

        zoomLayout->addWidget(zoomInButton);
        zoomLayout->addWidget(zoomOutButton);
        zoomLayout->addWidget(resetZoomButton);
        zoomLayout->addStretch();

        treeLayout->addLayout(zoomLayout);

        tabWidget->addTab(tokensTab, "TOKENS");
        tabWidget->addTab(identifiersTab, "IDENTIFIERS");
        tabWidget->addTab(treeTab, "TREE");
        resultsLayout->addWidget(resultsLabel);
        resultsLayout->addWidget(tabWidget);

        splitter->addWidget(editorWidget);
        splitter->addWidget(resultsWidget);
        splitter->setSizes(QList<int>() << 400 << 400);

        QHBoxLayout *bottomLayout = new QHBoxLayout();
        bottomLayout->addStretch();

        mainLayout->addLayout(buttonLayout);
        mainLayout->addWidget(splitter);
        mainLayout->addLayout(bottomLayout);

        statusBar()->showMessage("Ready");
        statusBar()->setObjectName("statusBar");
        if(isDarkTheme)
            statusBar()->setStyleSheet("background-color: #3c3c3c; color: #ffffff;");
        else
            statusBar()->setStyleSheet("background-color: #ffffff; color: #3c3c3c;");
        connect(openButton, &QPushButton::clicked, this, &LexerAnalyzerWindow::openFile);
        connect(saveButton, &QPushButton::clicked, this, &LexerAnalyzerWindow::saveFile);
        connect(clearButton, &QPushButton::clicked, this, &LexerAnalyzerWindow::clearText);
        connect(analyzeButton, &QPushButton::clicked, this, &LexerAnalyzerWindow::analyzeCode);
        connect(themeButton, &QPushButton::clicked, this, &LexerAnalyzerWindow::toggleTheme);

        applyTheme();
    }

private slots:
    void zoomIn() {
        treeView->scale(1.2, 1.2);
    }

    void zoomOut() {
        treeView->scale(0.8, 0.8);
    }

    void resetZoom() {
        treeView->resetTransform();
        if (!treeScene->items().isEmpty()) {
            treeView->fitInView(treeScene->itemsBoundingRect(), Qt::KeepAspectRatio);
            treeView->scale(0.9, 0.9);
        }
    }

    void renderDotGraph(const QString& dotContent) {
        treeScene->clear();

        QProcess checkProcess;
        checkProcess.start("dot", QStringList() << "-V");
        checkProcess.waitForFinished();

        if (checkProcess.exitCode() != 0) {
            QGraphicsTextItem* textItem = treeScene->addText(
                "Graphviz (dot) not found.\n"
                "Please install Graphviz to visualize parse trees.\n"
                "Download from: https://graphviz.org/download/");
            textItem->setDefaultTextColor(Qt::white);
            return;
        }

        QProcess dotProcess;
        dotProcess.start("dot", QStringList() << "-Tpng");

        if (!dotProcess.waitForStarted(3000)) {
            treeScene->addText("Failed to start dot process")->setDefaultTextColor(Qt::white);
            return;
        }

        dotProcess.write(dotContent.toUtf8());
        dotProcess.closeWriteChannel();

        if (!dotProcess.waitForFinished(5000)) {
            treeScene->addText("Dot process timed out")->setDefaultTextColor(Qt::white);
            return;
        }

        QByteArray pngData = dotProcess.readAllStandardOutput();
        QPixmap pixmap;
        if (pixmap.loadFromData(pngData)) {
            treeScene->addPixmap(pixmap);
            treeView->fitInView(treeScene->itemsBoundingRect(), Qt::KeepAspectRatio);
        } else {
            treeScene->addText("Failed to render tree image")->setDefaultTextColor(Qt::white);
        }
    }

    void openFile() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Text Files (*.py *.txt)"));
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                codeEditor->setText(file.readAll());
                file.close();
            }
        }
    }

    void saveFile() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text Files (*.py *.txt)"));
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.write(codeEditor->toPlainText().toUtf8());
                file.close();
            }
        }
    }

    void clearText() {
        codeEditor->clear();
        tokensText->clear();
        identifiersText->clear();
        outputIndex = 0;
        symbolTable.clear();
    }

    void analyzeCode() {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        treeScene->clear();
        statusBar()->clearMessage();
        outputIndex = 0;
        tokensText->clear();
        identifiersText->clear();
        symbolTable.clear();

        try {
            QString customTempDir = QDir::tempPath() + "/ParserTemp/";
            if (!QDir().mkpath(customTempDir)) {
                throw runtime_error("Failed to create temporary directory");
            }

            string code = codeEditor->toPlainText().toStdString();
            QString inputFilePath = customTempDir + "input.py";
            {
                ofstream tempFile(inputFilePath.toStdString());
                if (!tempFile) {
                    throw runtime_error("Failed to create temporary input file");
                }
                tempFile << code;
            }

            ifstream file(inputFilePath.toStdString());
            if (!file) {
                throw runtime_error("Failed to open temporary input file");
            }

            ifstream flattened = flatten_multiline_file(file);
            string line;
            int lineNumber = 0;
            stack<char> brackets;
            stack<int> indentLevels;
            indentLevels.push(0);

            while (getline(flattened, line)) {
                ++lineNumber;
                if (isCommentLine(line)) continue;
                if (line.size() > 0) {
                    storeOutput("\n");
                    handleIndentation(line, lineNumber, indentLevels);
                    analyzeLine(line, lineNumber, brackets, flattened);
                }
            }

            vector<string> tokens = parse_token_lines(output, outputIndex);
            vector<string> sanitized_tokens = sanitize_tokens_vector(tokens);
            saveTokensToFile(sanitized_tokens); // Use sanitized tokens for file and further processing

            for (const string& line : tokens) { // Changed to use non-sanitized 'tokens'
                size_t openBracket = line.find('[');
                size_t closeBracket = line.find(']');
                string numberStr = line.substr(openBracket + 1, closeBracket - openBracket - 1);
                if (line.find("<error;") != string::npos) {
                    QMessageBox::information(nullptr, "Error",
                                             QString("\n Error at line %1: PROGRAM TERMINATED.").arg(numberStr));
                    return;
                }
            }

            QString tokensFilePath = "A:\\MICHAEL\\Projects\\Design of Compilers\\Fidj\\CompilersQtGUI\\Tokens.txt";

            for (const string& token : tokens) { // Changed to use non-sanitized 'tokens'
                tokensText->append(QString::fromStdString(token));
            }

            build_and_draw_symbol_table(sanitized_tokens); // Use sanitized tokens for symbol table
            string table_html = "<pre><table border='1' style='border-collapse: collapse; font-family: \"Courier New\";'>";
            table_html += "<tr style='background-color: " + string(isDarkTheme ? "#444444" : "#cccccc") + ";'>";
            table_html += "<th style='padding: 5px; width: 60px; color: " + string(isDarkTheme ? "#ffffff" : "#000000") + ";'>Index</th>";
            table_html += "<th style='padding: 5px; width: 100px; color: " + string(isDarkTheme ? "#ffffff" : "#000000") + ";'>ID</th>";
            table_html += "<th style='padding: 5px; width: 80px; color: " + string(isDarkTheme ? "#ffffff" : "#000000") + ";'>Type</th>";
            table_html += "<th style='padding: 5px; width: 100px; color: " + string(isDarkTheme ? "#ffffff" : "#000000") + ";'>Value</th>";
            table_html += "</tr>";

            int index = 1;
            for (const auto& [name, info] : symbolTable) {
                string type_color;
                if (info.type == "int") type_color = (isDarkTheme ? "#ffff55" : "#ccaa00");
                else if (info.type == "float") type_color = (isDarkTheme ? "#55ffff" : "#0088cc");
                else if (info.type == "string") type_color = (isDarkTheme ? "#ff55ff" : "#cc2200");
                else type_color = (isDarkTheme ? "#aaaaaa" : "#666666");

                table_html += "<tr style='background-color: " + string(isDarkTheme ? (index % 2 == 0 ? "#333333" : "#3c3c3c") : (index % 2 == 0 ? "#e6e6e6" : "#f0f0f0")) + ";'>";
                table_html += "<td style='padding: 5px; text-align: center; color: " + string(isDarkTheme ? "#ffffff" : "#000000") + ";'>" + to_string(index) + "</td>";
                table_html += "<td style='padding: 5px; color: " + string(isDarkTheme ? "#ffffff" : "#000000") + ";'>" + info.name + "</td>";
                table_html += "<td style='padding: 5px; color: " + type_color + ";'>" + (info.type.empty() ? "N/A" : info.type) + "</td>";
                table_html += "<td style='padding: 5px; color: " + string(isDarkTheme ? "#ffffff" : "#000000") + ";'>" + (info.type.empty() ? "N/A" : info.value) + "</td></tr>";
                index++;
            }
            table_html += "</table></pre>";
            identifiersText->setHtml(QString::fromStdString(table_html));

            Parser parser;
            parser.loadTokens(tokensFilePath.toStdString());
            shared_ptr<ParseNode> parseTree = parser.parse();

            if (parseTree) {
                QString dotFilePath = customTempDir + "parse_tree.dot";
                QString pngFilePath = customTempDir + "parse_tree.png";

                parser.generateDOTFile(parseTree, dotFilePath.toStdString());
                statusBar()->showMessage("Generated DOT file", 2000);

                QFile inFile(dotFilePath);
                QString content;
                if (inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    content = QString(inFile.readAll());
                    inFile.close();
                }
                QString processed = QString::fromStdString(parser.processDotLabels(content.toStdString()));
                QFile outFile(dotFilePath);
                if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    outFile.write(processed.toUtf8());
                    outFile.close();
                }

                string errorMsg;
                if (!parser.create_Tree(dotFilePath.toStdString(), pngFilePath.toStdString())) {
                    throw runtime_error("Failed to generate parse tree image");
                }

                QPixmap pixmap(pngFilePath);
                if (!pixmap.isNull()) {
                    treeScene->clear();
                    QGraphicsPixmapItem* pixmapItem = treeScene->addPixmap(pixmap);
                    treeScene->setBackgroundBrush(QBrush(Qt::white));
                    treeView->setSceneRect(pixmapItem->boundingRect());
                    treeView->fitInView(pixmapItem, Qt::KeepAspectRatio);
                    treeView->scale(0.9, 0.9);
                    statusBar()->showMessage("Tree visualization loaded", 2000);
                } else {
                    throw runtime_error("Failed to load generated image");
                }
            }

        } catch (const exception& e) {
            statusBar()->showMessage(QString::fromStdString("Error: " + string(e.what())), 5000);

            if (string(e.what()).find("Graphviz") != string::npos) {
                QGraphicsTextItem* errorItem = treeScene->addText(
                    QString::fromStdString("Graphviz Error:\n" + string(e.what()) +
                                           "\n\nPlease install Graphviz from\nhttps://graphviz.org/download/"));
                errorItem->setDefaultTextColor(isDarkTheme ? Qt::white : Qt::black);
            }
        }
        QApplication::restoreOverrideCursor();
    }

    void toggleTheme() {
        isDarkTheme = !isDarkTheme;
        applyTheme();
        highlighter->setTheme(isDarkTheme ? PythonSyntaxHighlighter::Dark : PythonSyntaxHighlighter::Light);
    }

private:
    void applyTheme() {
        if (isDarkTheme) {
            qApp->setStyleSheet(darkThemeStylesheet);
            themeButton->setText("SWITCH TO LIGHT THEME");
        } else {
            qApp->setStyleSheet(lightThemeStylesheet);
            themeButton->setText("SWITCH TO DARK THEME");
        }
    }

    QTextEdit *codeEditor;
    QTabWidget *tabWidget;
    QWidget *tokensTab;
    QTextEdit *tokensText;
    QWidget *identifiersTab;
    QTextEdit *identifiersText;
    QWidget* treeTab;
    QGraphicsView* treeView;
    QGraphicsScene* treeScene;
    QPushButton *themeButton;
    PythonSyntaxHighlighter *highlighter;
    bool isDarkTheme;

    const QString darkThemeStylesheet = R"(
        QMainWindow {
            background-color: #2b2b2b;
            color: #ffffff;
        }
        QTextEdit#codeEditor {
            background-color: #1e1e1e;
            color: #ffffff;
            font-family: "Courier New";
            font-size: 14px;
            letter-spacing: 3.5px;
        }
        QTextEdit#tokensText, QTextEdit#identifiersText {
            background-color: #1e1e1e;
            color: #ffffff;
            font-family: "Courier New";
        }
        QTabWidget#tabWidget {
            background-color: #2d2d2d;
        }
        QTabBar::tab {
            background-color: #3c3c3c;
            color: #ffffff;
        }
        QTabBar::tab:selected {
            background-color: #4282da;
        }
        QGraphicsView#treeView {
            background-color: #1e1e1e;
        }
        QPushButton#themeButton {
            background-color: #4282da;
            color: #ffffff;
            padding: 5px;
        }
        QPushButton#themeButton:hover {
            background-color: #5b5b5b;
        }
        QStatusBar#statusBar {
            background-color: #3c3c3c;
            color: #ffffff;
        }
        QLabel {
            color: #ffffff;
        }
    )";

    const QString lightThemeStylesheet = R"(
        QMainWindow {
            background-color: #ffffff;
            color: #000000;
        }
        QTextEdit#codeEditor {
            background-color: #ffffff; /* Changed from #f0f0f0 to #ffffff */
            color: #000000;
            font-family: "Courier New";
            font-size: 14px;
            letter-spacing: 3.5px;
        }
        QTextEdit#tokensText, QTextEdit#identifiersText {
            background-color: #f0f0f0;
            color: #000000;
            font-family: "Courier New";
        }
        QTabWidget#tabWidget {
            background-color: #ffffff;
        }
        QTabBar::tab {
            background-color: #e0e0e0;
            color: #000000;
        }
        QTabBar::tab:selected {
            background-color: #4282da;
        }
        QGraphicsView#treeView {
            background-color: #f0f0f0;
        }
        QPushButton#themeButton {
            background-color: #e0e0e0;
            color: #000000;
            padding: 5px;
        }
        QPushButton#themeButton:hover {
            background-color: #d0d0d0;
        }
        QStatusBar#statusBar {
            background-color: #e0e0e0;
            color: #000000;
        }
        QLabel {
            color: #000000;
        }
    )";

protected:
    void resizeEvent(QResizeEvent* event) override {
        QMainWindow::resizeEvent(event);
        if (!treeScene->items().isEmpty()) {
            treeView->fitInView(treeScene->itemsBoundingRect(), Qt::KeepAspectRatio);
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    LexerAnalyzerWindow window;
    window.showMaximized();
    return app.exec();
}

#include "main.moc"
