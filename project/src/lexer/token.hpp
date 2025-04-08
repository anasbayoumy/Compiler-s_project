#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

// Enumeration of token types based on the project specification.
enum class TokenType {
    KEYWORD,     // Reserved words in Python (import, def, etc.)
    IDENTIFIER,  // Variable and function identifiers
    NUMBER,      // Numeric literals (integer and float)
    STRING,      // String literals
    OPERATOR,    // Operators and punctuations
    INDENT,      // An increase in indentation (for block start)
    DEDENT,      // A decrease in indentation (for block end)
    NEWLINE,     // End of statement line
    COMMENT,     // Comments
    ERROR        // Error tokens for invalid input
};

// Token class: represents a single token with its lexeme and type.
class Token {
public:
    // Constructor initializes the token with its lexeme, type, and position
    Token(const std::string &lexeme, TokenType type, int line = 0, int column = 0)
        : lexeme(lexeme), type(type), line(line), column(column) {}

    // Returns the lexeme (string representation) of the token
    std::string getLexeme() const { return lexeme; }

    // Returns the token type
    TokenType getType() const { return type; }

    // Returns the line number where the token was found
    int getLine() const { return line; }

    // Returns the column number where the token was found
    int getColumn() const { return column; }

private:
    std::string lexeme; // The actual string captured from the source code
    TokenType type;     // The type of the token
    int line;          // Line number in the source code
    int column;        // Column number in the source code
};

#endif // TOKEN_HPP
