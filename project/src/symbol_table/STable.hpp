#ifndef STABLE_HPP
#define STABLE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>

// Forward declaration of the Symbol class
class Symbol;

// Enum for different types of symbols
enum class SymbolType {
    VARIABLE,
    FUNCTION,
    CLASS,
    PARAMETER,
    MODULE,
    UNKNOWN
};

// Enum for basic data types
enum class DataType {
    INTEGER,
    FLOAT,
    STRING,
    BOOLEAN,
    LIST,
    DICT,
    NONE,
    UNKNOWN
};

// Class to represent a symbol (variable, function, etc.)
class Symbol {
public:
    // Default constructor
    Symbol() : name(""), 
               symbolType(SymbolType::UNKNOWN),
               dataType(DataType::UNKNOWN),
               lineNumber(0),
               columnNumber(0),
               scopeLevel(0),
               isInitialized(false) {}

    // Main constructor
    Symbol(const std::string& name, 
           SymbolType type = SymbolType::UNKNOWN,
           DataType dataType = DataType::UNKNOWN,
           int lineNumber = 0,
           int columnNumber = 0);

    // Getters
    std::string getName() const { return name; }
    SymbolType getSymbolType() const { return symbolType; }
    DataType getDataType() const { return dataType; }
    int getLineNumber() const { return lineNumber; }
    int getColumnNumber() const { return columnNumber; }
    int getScopeLevel() const { return scopeLevel; }

    // Setters
    void setDataType(DataType type) { dataType = type; }
    void setScopeLevel(int level) { scopeLevel = level; }
    void setInitialized(bool init) { isInitialized = init; }

    // For function symbols
    void addParameter(const Symbol& param) { parameters.push_back(param); }
    const std::vector<Symbol>& getParameters() const { return parameters; }

private:
    std::string name;
    SymbolType symbolType;
    DataType dataType;
    int lineNumber;
    int columnNumber;
    int scopeLevel;
    bool isInitialized;
    std::vector<Symbol> parameters;  // For functions
};

// Class to manage symbol table with scope handling
class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();

    // Scope management
    void enterScope();
    void exitScope();
    int getCurrentScope() const;

    // Symbol management
    bool insert(const Symbol& symbol);
    bool insertFunction(const std::string& name, int line, int column);
    bool insertVariable(const std::string& name, DataType type, int line, int column);
    
    // Symbol lookup
    std::optional<Symbol> lookup(const std::string& name) const;
    std::optional<Symbol> lookupInCurrentScope(const std::string& name) const;

    // Utility functions
    bool isDefined(const std::string& name) const;
    bool isDefinedInCurrentScope(const std::string& name) const;
    void printTable() const;  // For debugging

private:
    struct Scope {
        std::unordered_map<std::string, Symbol> symbols;
        int level;
    };

    std::vector<Scope> scopes;
    int currentScopeLevel;
};

// Utility functions
std::string symbolTypeToString(SymbolType type);
std::string dataTypeToString(DataType type);

#endif // STABLE_HPP
