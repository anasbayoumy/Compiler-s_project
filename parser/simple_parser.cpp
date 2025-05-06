#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <memory>
#include <fstream>
#include <map>
#include <stack>

// Token structure
struct Token {
    std::string type;
    std::string value;
    int line;
    
    Token(const std::string& t, const std::string& v, int l) : type(t), value(v), line(l) {}
};

// Node for parse tree
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

// Simple Parser class
class SimpleParser {
private:
    std::vector<Token> tokens;
    std::shared_ptr<ParseTreeNode> parseTree;
    std::vector<std::string> errors;
    
public:
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
    
    // Parse tokens into a simple parse tree
    bool parse(const std::vector<std::string>& tokenLines) {
        tokens = extractTokens(tokenLines);
        
        if (tokens.empty()) {
            errors.push_back("No tokens to parse");
            return false;
        }
        
        // Create a simple parse tree
        parseTree = std::make_shared<ParseTreeNode>("Program");
        
        // Group tokens by line
        int currentLine = -1;
        std::shared_ptr<ParseTreeNode> currentLineNode = nullptr;
        std::stack<std::shared_ptr<ParseTreeNode>> blockStack;
        
        for (const auto& token : tokens) {
            if (token.line != currentLine) {
                currentLine = token.line;
                currentLineNode = std::make_shared<ParseTreeNode>("Line", std::to_string(currentLine), currentLine);
                parseTree->addChild(currentLineNode);
            }
            
            // Create a node for the token
            auto tokenNode = std::make_shared<ParseTreeNode>(token.type, token.value, token.line);
            currentLineNode->addChild(tokenNode);
            
            // Special handling for certain tokens
            if (token.type == "keyword" && (token.value == "def" || token.value == "class")) {
                auto blockNode = std::make_shared<ParseTreeNode>(token.value + "Block", "", token.line);
                parseTree->addChild(blockNode);
                blockStack.push(blockNode);
            } else if (token.type == "indent") {
                if (!blockStack.empty()) {
                    auto indentNode = std::make_shared<ParseTreeNode>("IndentedBlock", "", token.line);
                    blockStack.top()->addChild(indentNode);
                    blockStack.push(indentNode);
                }
            } else if (token.type == "dedent") {
                if (!blockStack.empty()) {
                    blockStack.pop();
                }
            }
        }
        
        return true;
    }
    
    // Get the parse tree
    std::shared_ptr<ParseTreeNode> getParseTree() const {
        return parseTree;
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
    
    // Get errors
    std::vector<std::string> getErrors() const {
        return errors;
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

// Main function
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
        tokenLines.push_back("[3] <indent;> <indent;> <keyword; return> <int; 1> <dedent;>");
        tokenLines.push_back("[4] <keyword; else> <symbol; :>");
        tokenLines.push_back("[5] <indent;> <keyword; return> <id; n> <symbol; *> <id; factorial> <symbol; (> <id; n> <symbol; -> <int; 1> <symbol; )> <dedent;> <dedent;>");
    }
    
    std::cout << "Token lines:" << std::endl;
    for (const auto& line : tokenLines) {
        std::cout << line << std::endl;
    }
    
    // Create and run the parser
    SimpleParser parser;
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
