#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <memory>
#include <fstream>
#include <map>
#include <stack>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <iomanip>

// Token structure to represent a single token
struct Token {
    std::string type;
    std::string value;
    int line;

    Token(const std::string& t, const std::string& v, int l) : type(t), value(v), line(l) {}

    std::string toString() const {
        return "<" + type + "; " + value + ">";
    }
};

// Node class for the parse tree
class ParseTreeNode {
public:
    std::string type;
    std::string value;
    std::vector<std::shared_ptr<ParseTreeNode>> children;
    int line;

    ParseTreeNode(const std::string& t, const std::string& v = "", int l = -1)
        : type(t), value(v), line(l) {}

    void addChild(std::shared_ptr<ParseTreeNode> child) {
        children.push_back(child);
    }

    // Print the parse tree with indentation
    void print(int indent = 0) const {
        std::string indentation(indent * 2, ' ');
        std::cout << indentation << type;
        if (!value.empty()) {
            std::cout << ": " << value;
        }
        if (line > 0) {
            std::cout << " (line " << line << ")";
        }
        std::cout << std::endl;

        for (const auto& child : children) {
            child->print(indent + 1);
        }
    }

    // Save parse tree to a file
    void saveToFile(std::ofstream& file, int indent = 0) const {
        std::string indentation(indent * 2, ' ');
        file << indentation << type;
        if (!value.empty()) {
            file << ": " << value;
        }
        if (line > 0) {
            file << " (line " << line << ")";
        }
        file << std::endl;

        for (const auto& child : children) {
            child->saveToFile(file, indent + 1);
        }
    }
};

// Parser class for syntax analysis
class SyntaxAnalyzer {
private:
    std::vector<Token> tokens;
    size_t currentTokenIndex;
    std::shared_ptr<ParseTreeNode> parseTree;
    std::vector<std::string> errors;
    bool hasError;

    // Helper methods
    Token currentToken() const {
        if (isAtEnd()) {
            return Token("EOF", "", -1);
        }
        return tokens[currentTokenIndex];
    }

    Token peekNextToken() const {
        if (currentTokenIndex + 1 >= tokens.size()) {
            return Token("EOF", "", -1);
        }
        return tokens[currentTokenIndex + 1];
    }

    bool match(const std::string& tokenType) {
        if (isAtEnd()) return false;
        return currentToken().type == tokenType;
    }

    bool matchValue(const std::string& tokenType, const std::string& value) {
        if (isAtEnd()) return false;
        return currentToken().type == tokenType && currentToken().value == value;
    }

    void advance() {
        if (!isAtEnd()) {
            currentTokenIndex++;
        }
    }

    void addError(const std::string& message) {
        hasError = true;
        std::string errorMsg = "Line " + std::to_string(currentToken().line) + ": " + message;
        errors.push_back(errorMsg);
    }

    bool isAtEnd() const {
        return currentTokenIndex >= tokens.size();
    }

    // Error recovery - try to synchronize to continue parsing
    bool recover() {
        // Skip tokens until we find a statement boundary
        while (!isAtEnd()) {
            if (matchValue("symbol", ";") ||
                matchValue("keyword", "if") ||
                matchValue("keyword", "while") ||
                matchValue("keyword", "for") ||
                matchValue("keyword", "def") ||
                matchValue("keyword", "class") ||
                matchValue("keyword", "import") ||
                matchValue("keyword", "return")) {
                return true;
            }
            advance();
        }
        return false;
    }

    // Parse tree building methods
    std::shared_ptr<ParseTreeNode> parseProgram() {
        auto programNode = std::make_shared<ParseTreeNode>("Program");

        while (!isAtEnd()) {
            auto statement = parseStatement();
            if (statement) {
                programNode->addChild(statement);
            } else if (!recover()) {
                break;
            }
        }

        return programNode;
    }

    std::shared_ptr<ParseTreeNode> parseStatement() {
        if (matchValue("keyword", "if")) {
            return parseIfStatement();
        } else if (matchValue("keyword", "while")) {
            return parseWhileLoop();
        } else if (matchValue("keyword", "for")) {
            return parseForLoop();
        } else if (matchValue("keyword", "def")) {
            return parseFunctionDefinition();
        } else if (matchValue("keyword", "class")) {
            return parseClassDefinition();
        } else if (matchValue("keyword", "import")) {
            return parseImportStatement();
        } else if (matchValue("keyword", "return")) {
            return parseReturnStatement();
        } else if (match("id") || match("Function")) {
            // Check if it's an assignment or function call
            Token current = currentToken();
            Token next = peekNextToken();

            if (next.type == "symbol" && (next.value == "=" || next.value == "+=")) {
                return parseAssignment();
            } else if (next.type == "symbol" && next.value == "(") {
                return parseFunctionCall();
            }
        }

        // If we can't identify the statement type, try to parse it as an expression
        auto expr = parseExpression();
        if (expr) {
            return expr;
        }

        addError("Expected statement");
        return nullptr;
    }

    std::shared_ptr<ParseTreeNode> parseAssignment() {
        if (!match("id")) {
            addError("Expected identifier in assignment");
            return nullptr;
        }

        auto assignNode = std::make_shared<ParseTreeNode>("Assignment", "", currentToken().line);
        auto varNode = std::make_shared<ParseTreeNode>("Variable", currentToken().value, currentToken().line);
        assignNode->addChild(varNode);

        advance(); // Move past identifier

        if (!match("symbol") || (currentToken().value != "=" && currentToken().value != "+=")) {
            addError("Expected '=' or '+=' in assignment");
            return nullptr;
        }

        std::string opValue = currentToken().value;
        auto opNode = std::make_shared<ParseTreeNode>("Operator", opValue, currentToken().line);
        assignNode->addChild(opNode);

        advance(); // Move past operator

        auto exprNode = parseExpression();
        if (!exprNode) {
            addError("Expected expression in assignment");
            return nullptr;
        }

        assignNode->addChild(exprNode);
        return assignNode;
    }

    std::shared_ptr<ParseTreeNode> parseExpression() {
        // For simplicity, we'll just handle basic expressions
        if (match("int") || match("float") || match("string") || match("id")) {
            auto exprNode = std::make_shared<ParseTreeNode>("Expression", "", currentToken().line);
            auto valueNode = std::make_shared<ParseTreeNode>(currentToken().type, currentToken().value, currentToken().line);
            exprNode->addChild(valueNode);
            advance();

            // Check for binary operations
            if (match("symbol") && (currentToken().value == "+" ||
                                   currentToken().value == "-" ||
                                   currentToken().value == "*" ||
                                   currentToken().value == "/")) {
                auto opNode = std::make_shared<ParseTreeNode>("Operator", currentToken().value, currentToken().line);
                exprNode->addChild(opNode);
                advance();

                if (match("int") || match("float") || match("string") || match("id")) {
                    auto rightNode = std::make_shared<ParseTreeNode>(currentToken().type, currentToken().value, currentToken().line);
                    exprNode->addChild(rightNode);
                    advance();
                    return exprNode;
                } else {
                    addError("Expected right operand in expression");
                    return nullptr;
                }
            }

            return exprNode;
        } else if (match("Function")) {
            return parseFunctionCall();
        }

        return nullptr;
    }

    std::shared_ptr<ParseTreeNode> parseIfStatement() {
        if (!matchValue("keyword", "if")) {
            addError("Expected 'if' keyword");
            return nullptr;
        }

        auto ifNode = std::make_shared<ParseTreeNode>("IfStatement", "", currentToken().line);
        advance(); // Move past 'if'

        auto conditionNode = parseCondition();
        if (!conditionNode) {
            addError("Expected condition in if statement");
            return nullptr;
        }
        ifNode->addChild(conditionNode);

        if (!matchValue("symbol", ":")) {
            addError("Expected ':' after if condition");
            return nullptr;
        }
        advance(); // Move past ':'

        auto blockNode = parseBlock();
        if (!blockNode) {
            addError("Expected block in if statement");
            return nullptr;
        }
        ifNode->addChild(blockNode);

        // Check for elif and else blocks
        while (matchValue("keyword", "elif")) {
            advance(); // Move past 'elif'

            auto elifCondNode = parseCondition();
            if (!elifCondNode) {
                addError("Expected condition in elif statement");
                return nullptr;
            }

            auto elifNode = std::make_shared<ParseTreeNode>("ElifStatement", "", elifCondNode->line);
            elifNode->addChild(elifCondNode);

            if (!matchValue("symbol", ":")) {
                addError("Expected ':' after elif condition");
                return nullptr;
            }
            advance(); // Move past ':'

            auto elifBlockNode = parseBlock();
            if (!elifBlockNode) {
                addError("Expected block in elif statement");
                return nullptr;
            }
            elifNode->addChild(elifBlockNode);
            ifNode->addChild(elifNode);
        }

        if (matchValue("keyword", "else")) {
            advance(); // Move past 'else'

            if (!matchValue("symbol", ":")) {
                addError("Expected ':' after else");
                return nullptr;
            }
            advance(); // Move past ':'

            auto elseBlockNode = parseBlock();
            if (!elseBlockNode) {
                addError("Expected block in else statement");
                return nullptr;
            }

            auto elseNode = std::make_shared<ParseTreeNode>("ElseStatement", "", elseBlockNode->line);
            elseNode->addChild(elseBlockNode);
            ifNode->addChild(elseNode);
        }

        return ifNode;
    }

    std::shared_ptr<ParseTreeNode> parseCondition() {
        auto condNode = std::make_shared<ParseTreeNode>("Condition", "", currentToken().line);

        auto leftExpr = parseExpression();
        if (!leftExpr) {
            addError("Expected expression in condition");
            return nullptr;
        }
        condNode->addChild(leftExpr);

        if (match("symbol") && (currentToken().value == "==" ||
                               currentToken().value == "!=" ||
                               currentToken().value == "<" ||
                               currentToken().value == ">" ||
                               currentToken().value == "<=" ||
                               currentToken().value == ">=")) {
            auto opNode = std::make_shared<ParseTreeNode>("ComparisonOperator", currentToken().value, currentToken().line);
            condNode->addChild(opNode);
            advance();

            auto rightExpr = parseExpression();
            if (!rightExpr) {
                addError("Expected right expression in condition");
                return nullptr;
            }
            condNode->addChild(rightExpr);
        }

        return condNode;
    }

    std::shared_ptr<ParseTreeNode> parseBlock() {
        auto blockNode = std::make_shared<ParseTreeNode>("Block", "", currentToken().line);

        if (match("indent")) {
            advance(); // Move past indent

            while (!match("dedent") && !isAtEnd()) {
                auto stmt = parseStatement();
                if (stmt) {
                    blockNode->addChild(stmt);
                } else if (!recover()) {
                    break;
                }
            }

            if (match("dedent")) {
                advance(); // Move past dedent
            } else if (!isAtEnd()) {
                addError("Expected dedent at end of block");
            }
        } else {
            // Single statement block without indentation
            auto stmt = parseStatement();
            if (stmt) {
                blockNode->addChild(stmt);
            }
        }

        return blockNode;
    }

    std::shared_ptr<ParseTreeNode> parseWhileLoop() {
        if (!matchValue("keyword", "while")) {
            addError("Expected 'while' keyword");
            return nullptr;
        }

        auto whileNode = std::make_shared<ParseTreeNode>("WhileLoop", "", currentToken().line);
        advance(); // Move past 'while'

        auto conditionNode = parseCondition();
        if (!conditionNode) {
            addError("Expected condition in while loop");
            return nullptr;
        }
        whileNode->addChild(conditionNode);

        if (!matchValue("symbol", ":")) {
            addError("Expected ':' after while condition");
            return nullptr;
        }
        advance(); // Move past ':'

        auto blockNode = parseBlock();
        if (!blockNode) {
            addError("Expected block in while loop");
            return nullptr;
        }
        whileNode->addChild(blockNode);

        return whileNode;
    }

    std::shared_ptr<ParseTreeNode> parseForLoop() {
        if (!matchValue("keyword", "for")) {
            addError("Expected 'for' keyword");
            return nullptr;
        }

        auto forNode = std::make_shared<ParseTreeNode>("ForLoop", "", currentToken().line);
        advance(); // Move past 'for'

        if (!match("id")) {
            addError("Expected identifier in for loop");
            return nullptr;
        }

        auto varNode = std::make_shared<ParseTreeNode>("Variable", currentToken().value, currentToken().line);
        forNode->addChild(varNode);
        advance(); // Move past identifier

        if (!matchValue("keyword", "in")) {
            addError("Expected 'in' keyword in for loop");
            return nullptr;
        }
        advance(); // Move past 'in'

        auto iterableNode = parseExpression();
        if (!iterableNode) {
            addError("Expected iterable expression in for loop");
            return nullptr;
        }
        forNode->addChild(iterableNode);

        if (!matchValue("symbol", ":")) {
            addError("Expected ':' after for loop header");
            return nullptr;
        }
        advance(); // Move past ':'

        auto blockNode = parseBlock();
        if (!blockNode) {
            addError("Expected block in for loop");
            return nullptr;
        }
        forNode->addChild(blockNode);

        return forNode;
    }

    std::shared_ptr<ParseTreeNode> parseFunctionDefinition() {
        if (!matchValue("keyword", "def")) {
            addError("Expected 'def' keyword");
            return nullptr;
        }

        auto funcDefNode = std::make_shared<ParseTreeNode>("FunctionDefinition", "", currentToken().line);
        advance(); // Move past 'def'

        if (!match("id")) {
            addError("Expected function name");
            return nullptr;
        }

        auto nameNode = std::make_shared<ParseTreeNode>("FunctionName", currentToken().value, currentToken().line);
        funcDefNode->addChild(nameNode);
        advance(); // Move past function name

        if (!matchValue("symbol", "(")) {
            addError("Expected '(' after function name");
            return nullptr;
        }
        advance(); // Move past '('

        // Parse parameters
        auto paramsNode = std::make_shared<ParseTreeNode>("Parameters");
        while (!matchValue("symbol", ")") && !isAtEnd()) {
            if (!match("id")) {
                addError("Expected parameter name");
                return nullptr;
            }

            auto paramNode = std::make_shared<ParseTreeNode>("Parameter", currentToken().value, currentToken().line);
            paramsNode->addChild(paramNode);
            advance(); // Move past parameter name

            if (matchValue("symbol", ",")) {
                advance(); // Move past ','
            } else if (!matchValue("symbol", ")")) {
                addError("Expected ',' or ')' after parameter");
                return nullptr;
            }
        }

        funcDefNode->addChild(paramsNode);

        if (!matchValue("symbol", ")")) {
            addError("Expected ')' after parameters");
            return nullptr;
        }
        advance(); // Move past ')'

        if (!matchValue("symbol", ":")) {
            addError("Expected ':' after function header");
            return nullptr;
        }
        advance(); // Move past ':'

        auto blockNode = parseBlock();
        if (!blockNode) {
            addError("Expected function body");
            return nullptr;
        }
        funcDefNode->addChild(blockNode);

        return funcDefNode;
    }

    std::shared_ptr<ParseTreeNode> parseFunctionCall() {
        if (!match("id") && !match("Function")) {
            addError("Expected function name");
            return nullptr;
        }

        auto funcCallNode = std::make_shared<ParseTreeNode>("FunctionCall", currentToken().value, currentToken().line);
        advance(); // Move past function name

        if (!matchValue("symbol", "(")) {
            addError("Expected '(' after function name");
            return nullptr;
        }
        advance(); // Move past '('

        // Parse arguments
        auto argsNode = std::make_shared<ParseTreeNode>("Arguments");
        while (!matchValue("symbol", ")") && !isAtEnd()) {
            auto argExpr = parseExpression();
            if (!argExpr) {
                addError("Expected argument expression");
                return nullptr;
            }

            argsNode->addChild(argExpr);

            if (matchValue("symbol", ",")) {
                advance(); // Move past ','
            } else if (!matchValue("symbol", ")")) {
                addError("Expected ',' or ')' after argument");
                return nullptr;
            }
        }

        funcCallNode->addChild(argsNode);

        if (!matchValue("symbol", ")")) {
            addError("Expected ')' after arguments");
            return nullptr;
        }
        advance(); // Move past ')'

        return funcCallNode;
    }

    std::shared_ptr<ParseTreeNode> parseClassDefinition() {
        if (!matchValue("keyword", "class")) {
            addError("Expected 'class' keyword");
            return nullptr;
        }

        auto classDefNode = std::make_shared<ParseTreeNode>("ClassDefinition", "", currentToken().line);
        advance(); // Move past 'class'

        if (!match("id")) {
            addError("Expected class name");
            return nullptr;
        }

        auto nameNode = std::make_shared<ParseTreeNode>("ClassName", currentToken().value, currentToken().line);
        classDefNode->addChild(nameNode);
        advance(); // Move past class name

        // Check for inheritance
        if (matchValue("symbol", "(")) {
            advance(); // Move past '('

            auto baseClassesNode = std::make_shared<ParseTreeNode>("BaseClasses");

            while (!matchValue("symbol", ")") && !isAtEnd()) {
                if (!match("id")) {
                    addError("Expected base class name");
                    return nullptr;
                }

                auto baseClassNode = std::make_shared<ParseTreeNode>("BaseClass", currentToken().value, currentToken().line);
                baseClassesNode->addChild(baseClassNode);
                advance(); // Move past base class name

                if (matchValue("symbol", ",")) {
                    advance(); // Move past ','
                } else if (!matchValue("symbol", ")")) {
                    addError("Expected ',' or ')' after base class");
                    return nullptr;
                }
            }

            classDefNode->addChild(baseClassesNode);

            if (!matchValue("symbol", ")")) {
                addError("Expected ')' after base classes");
                return nullptr;
            }
            advance(); // Move past ')'
        }

        if (!matchValue("symbol", ":")) {
            addError("Expected ':' after class header");
            return nullptr;
        }
        advance(); // Move past ':'

        auto blockNode = parseBlock();
        if (!blockNode) {
            addError("Expected class body");
            return nullptr;
        }
        classDefNode->addChild(blockNode);

        return classDefNode;
    }

    std::shared_ptr<ParseTreeNode> parseImportStatement() {
        if (!matchValue("keyword", "import")) {
            addError("Expected 'import' keyword");
            return nullptr;
        }

        auto importNode = std::make_shared<ParseTreeNode>("ImportStatement", "", currentToken().line);
        advance(); // Move past 'import'

        if (!match("id")) {
            addError("Expected module name");
            return nullptr;
        }

        auto moduleNode = std::make_shared<ParseTreeNode>("ModuleName", currentToken().value, currentToken().line);
        importNode->addChild(moduleNode);
        advance(); // Move past module name

        // Handle 'from ... import ...' syntax
        if (matchValue("keyword", "as")) {
            advance(); // Move past 'as'

            if (!match("id")) {
                addError("Expected alias name after 'as'");
                return nullptr;
            }

            auto aliasNode = std::make_shared<ParseTreeNode>("Alias", currentToken().value, currentToken().line);
            importNode->addChild(aliasNode);
            advance(); // Move past alias
        }

        return importNode;
    }

    std::shared_ptr<ParseTreeNode> parseReturnStatement() {
        if (!matchValue("keyword", "return")) {
            addError("Expected 'return' keyword");
            return nullptr;
        }

        auto returnNode = std::make_shared<ParseTreeNode>("ReturnStatement", "", currentToken().line);
        advance(); // Move past 'return'

        // Return value is optional
        if (!isAtEnd() && !matchValue("symbol", ";")) {
            auto exprNode = parseExpression();
            if (exprNode) {
                returnNode->addChild(exprNode);
            }
        }

        return returnNode;
    }

public:
    SyntaxAnalyzer() : currentTokenIndex(0), hasError(false) {}

    // Extract tokens from the lexer output format
    std::vector<Token> extractTokens(const std::vector<std::string>& tokenLines) {
        std::vector<Token> result;
        std::regex linePattern(R"(\[(\d+)\]\s*(.*))");
        std::regex tokenPattern(R"(<([^;]+);\s*([^>]*)>)");

        for (const auto& line : tokenLines) {
            std::smatch lineMatch;
            if (std::regex_match(line, lineMatch, linePattern)) {
                int lineNumber = std::stoi(lineMatch[1]);
                std::string tokensStr = lineMatch[2];

                std::smatch tokenMatch;
                std::string::const_iterator searchStart(tokensStr.cbegin());

                while (std::regex_search(searchStart, tokensStr.cend(), tokenMatch, tokenPattern)) {
                    std::string tokenType = tokenMatch[1];
                    std::string tokenValue = tokenMatch[2];

                    // Trim whitespace
                    tokenType.erase(0, tokenType.find_first_not_of(" \t\n\r\f\v"));
                    tokenType.erase(tokenType.find_last_not_of(" \t\n\r\f\v") + 1);
                    tokenValue.erase(0, tokenValue.find_first_not_of(" \t\n\r\f\v"));
                    tokenValue.erase(tokenValue.find_last_not_of(" \t\n\r\f\v") + 1);

                    result.emplace_back(tokenType, tokenValue, lineNumber);
                    searchStart = tokenMatch.suffix().first;
                }
            }
        }

        return result;
    }

    // Parse tokens into a syntax tree
    bool parse(const std::vector<std::string>& tokenLines) {
        tokens = extractTokens(tokenLines);
        currentTokenIndex = 0;
        hasError = false;
        errors.clear();

        if (tokens.empty()) {
            addError("No tokens to parse");
            return false;
        }

        parseTree = parseProgram();
        return !hasError;
    }

    // Get the parse tree
    std::shared_ptr<ParseTreeNode> getParseTree() const {
        return parseTree;
    }

    // Get parsing errors
    std::vector<std::string> getErrors() const {
        return errors;
    }

    // Check if parsing had errors
    bool hasErrors() const {
        return hasError;
    }

    // Print the parse tree
    void printParseTree() const {
        if (parseTree) {
            parseTree->print();
        } else {
            std::cout << "No parse tree available." << std::endl;
        }
    }

    // Save parse tree to a file
    void saveParseTree(const std::string& filename) const {
        if (!parseTree) {
            std::cerr << "No parse tree available to save." << std::endl;
            return;
        }

        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
            return;
        }

        parseTree->saveToFile(file);
        file.close();

        std::cout << "Parse tree saved to " << filename << std::endl;
    }

    // Print errors
    void printErrors() const {
        if (errors.empty()) {
            std::cout << "No syntax errors found." << std::endl;
            return;
        }

        std::cout << "Syntax Errors:" << std::endl;
        for (const auto& error : errors) {
            std::cout << "  " << error << std::endl;
        }
    }
};

// Function to read token lines from a file
std::vector<std::string> readTokensFromFile(const std::string& filename) {
    std::vector<std::string> tokenLines;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return tokenLines;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            tokenLines.push_back(line);
        }
    }

    file.close();
    return tokenLines;
}

// Function to save token lines to a file
void saveTokensToFile(const std::vector<std::string>& tokenLines, const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
        return;
    }

    for (const auto& line : tokenLines) {
        file << line << std::endl;
    }

    file.close();
}

// Main function to demonstrate the parser
int main(int argc, char* argv[]) {
    // Check if a filename was provided
    std::string tokensFile = "tokens.txt";
    std::string parseTreeFile = "parse_tree.txt";

    if (argc > 1) {
        tokensFile = argv[1];
    }

    if (argc > 2) {
        parseTreeFile = argv[2];
    }

    // Read token lines from file
    std::vector<std::string> tokenLines = readTokensFromFile(tokensFile);

    if (tokenLines.empty()) {
        std::cout << "No tokens found in file. Using example tokens for testing." << std::endl;

        // Example token lines for testing
        tokenLines.push_back("[1] <keyword; def> <id; factorial> <symbol; (> <id; n> <symbol; )> <symbol; :>");
        tokenLines.push_back("[2] <indent;> <keyword; if> <id; n> <symbol; <=> <int; 1> <symbol; :>");
        tokenLines.push_back("[3] <indent;> <keyword; return> <int; 1> <dedent;>");
        tokenLines.push_back("[4] <keyword; else> <symbol; :>");
        tokenLines.push_back("[5] <indent;> <keyword; return> <id; n> <symbol; *> <id; factorial> <symbol; (> <id; n> <symbol; -> <int; 1> <symbol; )> <dedent;> <dedent;>");
    }

    std::cout << "Token lines:" << std::endl;
    for (const auto& line : tokenLines) {
        std::cout << line << std::endl;
    }

    // Create and run the parser
    SyntaxAnalyzer parser;
    bool success = parser.parse(tokenLines);

    if (success) {
        std::cout << "\nParsing successful!" << std::endl;
        std::cout << "Parse Tree:" << std::endl;
        parser.printParseTree();

        // Save parse tree to file
        parser.saveParseTree(parseTreeFile);
    } else {
        std::cout << "\nParsing failed with errors:" << std::endl;
        parser.printErrors();
    }

    return 0;
}
