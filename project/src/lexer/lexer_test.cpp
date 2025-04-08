#include "lexer.hpp"
#include <iostream>
#include <string>

// Helper function to convert TokenType to string
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KEYWORD: return "KEYWORD";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::OPERATOR: return "OPERATOR";
        case TokenType::INDENT: return "INDENT";
        case TokenType::DEDENT: return "DEDENT";
        case TokenType::NEWLINE: return "NEWLINE";
        case TokenType::COMMENT: return "COMMENT";
        case TokenType::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

int main() {
    // Test cases
    std::string testCode = R"(
def hello_world():
    print("Hello, World!")
    x = 42
    y = 3.14
    # This is a comment
    if x > y:
        print("x is greater")
    else:
        print("y is greater")
)";

    std::cout << "Testing lexer with the following code:\n" << testCode << "\n\n";

    // Create lexer and tokenize
    Lexer lexer(testCode);
    std::vector<Token> tokens = lexer.tokenize();

    // Print tokens
    std::cout << "Generated tokens:\n";
    std::cout << "Type\t\tLexeme\t\tLine\tColumn\n";
    std::cout << "----------------------------------------\n";
    
    for (const auto& token : tokens) {
        std::cout << tokenTypeToString(token.getType()) << "\t"
                << token.getLexeme() << "\t"
                << token.getLine() << "\t"
                << token.getColumn() << "\n";
    }

    return 0;
} 




