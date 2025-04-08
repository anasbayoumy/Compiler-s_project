#include "Lexer.hpp"
// #include "IndentHandler.cpp"  // Note: Ideally, include a header for indent handling.
#include <regex>
#include <iostream>
#include <unordered_set>

// Constructor initializes the source code and position tracking
Lexer::Lexer(const std::string &source) 
    : sourceCode(source), currentPos(0), currentLine(1), currentColumn(1) {
}

// Destructor
Lexer::~Lexer() {}

// Helper function to get current character
char Lexer::currentChar() const {
    return currentPos < sourceCode.length() ? sourceCode[currentPos] : '\0';
}

// Helper function to advance to next character
void Lexer::advance() {
    if (currentChar() == '\n') {
        currentLine++;
        currentColumn = 1;
    } else {
        currentColumn++;
    }
    currentPos++;
}

// Helper function to skip whitespace and update position
void Lexer::skipWhitespace() {
    while (currentPos < sourceCode.length() && isspace(currentChar())) {
        advance();
    }
}

// Check if a lexeme is a keyword
bool Lexer::isKeyword(const std::string &lexeme) const {
    static const std::unordered_set<std::string> keywords = {
        "import", "from", "def", "return", "if", "elif", "else",
        "for", "while", "break", "continue", "class", "True", "False",
        "and", "or", "not", "in", "try", "except", "finally", "raise",
        "with", "as", "pass", "yield", "global", "nonlocal", "lambda"
    };
    return keywords.find(lexeme) != keywords.end();
}

// Check if a lexeme is an operator
bool Lexer::isOperator(const std::string &lexeme) const {
    static const std::unordered_set<std::string> operators = {
        "+", "-", "*", "/", "//", "%", "**", "=", "==", "!=", "<", ">",
        "<=", ">=", "+=", "-=", "*=", "/=", "%=", "**=", "//=", "&", "|",
        "^", "~", "<<", ">>", "and", "or", "not", "in", "is", "is not"
    };
    return operators.find(lexeme) != operators.end();
}

// Check if a lexeme is a number
bool Lexer::isNumber(const std::string &lexeme) const {
    static const std::regex numberPattern("^[+-]?\\d+(\\.\\d+)?([eE][+-]?\\d+)?$");
    return std::regex_match(lexeme, numberPattern);
}

// Check if a lexeme is a string
bool Lexer::isString(const std::string &lexeme) const {
    return lexeme.length() >= 2 && 
           ((lexeme.front() == '"' && lexeme.back() == '"') ||
            (lexeme.front() == '\'' && lexeme.back() == '\''));
}

// Check if a lexeme is a comment
bool Lexer::isComment(const std::string &lexeme) const {
    return lexeme.length() >= 1 && lexeme[0] == '#';
}

// Extract a string token
Token Lexer::extractString() {
    char quote = currentChar();
    int startLine = currentLine;
    int startColumn = currentColumn;
    std::string lexeme;
    lexeme += quote;
    advance();

    while (currentPos < sourceCode.length() && currentChar() != quote) {
        if (currentChar() == '\\') {
            lexeme += currentChar();
            advance();
        }
        lexeme += currentChar();
        advance();
    }

    if (currentChar() == quote) {
        lexeme += currentChar();
        advance();
        return Token(lexeme, TokenType::STRING, startLine, startColumn);
    } else {
        return Token(lexeme, TokenType::ERROR, startLine, startColumn);
    }
}

// Extract a number token
Token Lexer::extractNumber() {
    int startLine = currentLine;
    int startColumn = currentColumn;
    std::string lexeme;

    while (currentPos < sourceCode.length() && 
           (isdigit(currentChar()) || currentChar() == '.' || 
            currentChar() == 'e' || currentChar() == 'E' || 
            currentChar() == '+' || currentChar() == '-')) {
        lexeme += currentChar();
        advance();
    }

    if (isNumber(lexeme)) {
        return Token(lexeme, TokenType::NUMBER, startLine, startColumn);
    } else {
        return Token(lexeme, TokenType::ERROR, startLine, startColumn);
    }
}

// Extract an identifier token
Token Lexer::extractIdentifier() {
    int startLine = currentLine;
    int startColumn = currentColumn;
    std::string lexeme;

    while (currentPos < sourceCode.length() && 
           (isalnum(currentChar()) || currentChar() == '_')) {
        lexeme += currentChar();
        advance();
    }

    if (isKeyword(lexeme)) {
        return Token(lexeme, TokenType::KEYWORD, startLine, startColumn);
    } else {
        return Token(lexeme, TokenType::IDENTIFIER, startLine, startColumn);
    }
}

// Extract an operator token
Token Lexer::extractOperator() {
    int startLine = currentLine;
    int startColumn = currentColumn;
    std::string lexeme;
    lexeme += currentChar();
    advance();

    // Check for multi-character operators
    if (currentPos < sourceCode.length()) {
        std::string potentialOp = lexeme + currentChar();
        if (isOperator(potentialOp)) {
            lexeme += currentChar();
            advance();
        }
    }

    return Token(lexeme, TokenType::OPERATOR, startLine, startColumn);
}

// Extract a comment token
Token Lexer::extractComment() {
    int startLine = currentLine;
    int startColumn = currentColumn;
    std::string lexeme;

    while (currentPos < sourceCode.length() && currentChar() != '\n') {
        lexeme += currentChar();
        advance();
    }

    return Token(lexeme, TokenType::COMMENT, startLine, startColumn);
}

// Main tokenization function
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    currentPos = 0;
    currentLine = 1;
    currentColumn = 1;

    while (currentPos < sourceCode.length()) {
        skipWhitespace();
        if (currentPos >= sourceCode.length()) break;

        char c = currentChar();
        Token token("", TokenType::ERROR, currentLine, currentColumn);  // Initialize with default values

        if (c == '"' || c == '\'') {
            token = extractString();
        } else if (isdigit(c) || (c == '.' && isdigit(sourceCode[currentPos + 1]))) {
            token = extractNumber();
        } else if (isalpha(c) || c == '_') {
            token = extractIdentifier();
        } else if (c == '#') {
            token = extractComment();
        } else if (ispunct(c)) {
            token = extractOperator();
        } else {
            token = Token(std::string(1, c), TokenType::ERROR, currentLine, currentColumn);
            advance();
        }

        tokens.push_back(token);
    }
    
    // Process indentation
    processIndentation(tokens);

    return tokens;
}

// Process indentation
void Lexer::processIndentation(std::vector<Token>& tokens) {
    std::vector<int> indentStack = {0};
    std::vector<Token> newTokens;
    int currentIndent = 0;
    bool atLineStart = true;

    for (const auto& token : tokens) {
        if (atLineStart) {
            if (token.getType() == TokenType::OPERATOR && token.getLexeme() == " ") {
                currentIndent++;
            } else {
                if (currentIndent > indentStack.back()) {
                    newTokens.push_back(Token("INDENT", TokenType::INDENT, token.getLine(), token.getColumn()));
                    indentStack.push_back(currentIndent);
                } else if (currentIndent < indentStack.back()) {
                    while (currentIndent < indentStack.back()) {
                        newTokens.push_back(Token("DEDENT", TokenType::DEDENT, token.getLine(), token.getColumn()));
                        indentStack.pop_back();
                    }
                }
                currentIndent = 0;
                atLineStart = false;
                newTokens.push_back(token);
            }
        } else {
            if (token.getType() == TokenType::NEWLINE) {
                atLineStart = true;
            }
            newTokens.push_back(token);
        }
    }

    // Add final DEDENTs if needed
    while (indentStack.size() > 1) {
        newTokens.push_back(Token("DEDENT", TokenType::DEDENT, currentLine, currentColumn));
        indentStack.pop_back();
    }

    tokens = newTokens;
}
