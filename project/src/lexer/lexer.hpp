#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <regex>
#include "Token.hpp"

class Lexer {
public:
    // Constructor: Accepts the entire source code as a string.
    Lexer(const std::string &source);

    // Destructor.
    ~Lexer();

    // Performs the tokenization of the source code and returns a vector of tokens.
    std::vector<Token> tokenize();

private:
    // The input source code.
    std::string sourceCode;
    
    // Current position in the source code
    size_t currentPos;
    int currentLine;
    int currentColumn;

    // Processes indentation based on Python's INDENT/DEDENT rules.
    void processIndentation(std::vector<Token>& tokens);

    // Determines and returns the token type for the given lexeme.
    TokenType determineTokenType(const std::string &lexeme);

    // Helper functions for token recognition
    bool isKeyword(const std::string &lexeme) const;
    bool isOperator(const std::string &lexeme) const;
    bool isNumber(const std::string &lexeme) const;
    bool isString(const std::string &lexeme) const;
    bool isComment(const std::string &lexeme) const;

    // Helper functions for token extraction
    Token extractString();
    Token extractNumber();
    Token extractIdentifier();
    Token extractOperator();
    Token extractComment();

    // Helper function to skip whitespace and update position
    void skipWhitespace();

    // Helper function to get current character
    char currentChar() const;
    
    // Helper function to advance to next character
    void advance();
};

#endif // LEXER_HPP
