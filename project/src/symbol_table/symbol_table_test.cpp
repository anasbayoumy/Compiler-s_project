#include "../lexer/lexer.hpp"
#include "STable.hpp"
#include <iostream>
#include <string>

// Helper function to determine data type from a token
DataType inferDataTypeFromToken(const Token& token) {
    std::cout << "Inferring type for token: " << token.getLexeme() << std::endl;
    if (token.getType() == TokenType::NUMBER) {
        // Check if the number has a decimal point
        return (token.getLexeme().find('.') != std::string::npos) 
               ? DataType::FLOAT 
               : DataType::INTEGER;
    } else if (token.getType() == TokenType::STRING) {
        return DataType::STRING;
    } else if (token.getLexeme() == "True" || token.getLexeme() == "False") {
        return DataType::BOOLEAN;
    }
    return DataType::UNKNOWN;
}

int main() {
    // Test code with variable declarations and function definitions
    std::string testCode = R"(
def calculate_area(width, height):
    area = width * height
    return area

x = 42
y = 3.14
name = "Python"
is_valid = True

if x > y:
    result = x + y
    print(result)
)";

    std::cout << "Testing Symbol Table with the following code:\n" << testCode << "\n\n";

    // Create lexer and symbol table
    Lexer lexer(testCode);
    SymbolTable symbolTable;

    std::cout << "Created lexer and symbol table\n";

    // Get tokens
    std::vector<Token> tokens = lexer.tokenize();
    std::cout << "Tokenized input. Found " << tokens.size() << " tokens\n";

    // Process tokens and build symbol table
    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens[i];
        std::cout << "Processing token: " << token.getLexeme() << " (Type: " << static_cast<int>(token.getType()) << ")\n";
        
        // Function declaration
        if (token.getType() == TokenType::KEYWORD && token.getLexeme() == "def") {
            std::cout << "Found function declaration\n";
            // Next token should be function name
            if (i + 1 < tokens.size() && tokens[i + 1].getType() == TokenType::IDENTIFIER) {
                const Token& funcNameToken = tokens[i + 1];
                symbolTable.insertFunction(funcNameToken.getLexeme(), 
                                        funcNameToken.getLine(), 
                                        funcNameToken.getColumn());
                std::cout << "Inserted function: " << funcNameToken.getLexeme() << std::endl;
                
                // Enter new scope for function parameters and body
                symbolTable.enterScope();
                std::cout << "Entered new scope for function\n";
                
                // Process parameters
                i += 2; // Skip to opening parenthesis
                while (i < tokens.size() && tokens[i].getLexeme() != ":") {
                    if (tokens[i].getType() == TokenType::IDENTIFIER) {
                        Symbol param(tokens[i].getLexeme(), 
                                   SymbolType::PARAMETER,
                                   DataType::UNKNOWN,
                                   tokens[i].getLine(),
                                   tokens[i].getColumn());
                        symbolTable.insert(param);
                        std::cout << "Added parameter: " << tokens[i].getLexeme() << std::endl;
                    }
                    i++;
                }
            }
        }
        // Variable assignment
        else if (token.getType() == TokenType::IDENTIFIER) {
            if (i + 1 < tokens.size() && tokens[i + 1].getLexeme() == "=") {
                std::cout << "Found variable assignment: " << token.getLexeme() << std::endl;
                // Look ahead to determine type from the assigned value
                if (i + 2 < tokens.size()) {
                    const Token& valueToken = tokens[i + 2];
                    DataType dataType = inferDataTypeFromToken(valueToken);
                    symbolTable.insertVariable(token.getLexeme(),
                                            dataType,
                                            token.getLine(),
                                            token.getColumn());
                    std::cout << "Inserted variable: " << token.getLexeme() << " with type: " 
                             << static_cast<int>(dataType) << std::endl;
                }
            }
        }
        // Block start (if, while, for, etc.)
        else if (token.getLexeme() == ":" && 
                 i > 0 && 
                 tokens[i-1].getType() != TokenType::IDENTIFIER) {
            symbolTable.enterScope();
            std::cout << "Entered new block scope\n";
        }
        // Handle scope exits (this is simplified; in a real compiler you'd need more sophisticated scope tracking)
        else if (token.getType() == TokenType::DEDENT) {
            symbolTable.exitScope();
            std::cout << "Exited scope\n";
        }
    }

    // Print the symbol table contents
    std::cout << "\nFinal Symbol Table:\n";
    symbolTable.printTable();

    return 0;
} 