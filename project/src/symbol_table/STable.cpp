#include "STable.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

// Symbol implementation
Symbol::Symbol(const std::string& name, 
               SymbolType type,
               DataType dataType,
               int lineNumber,
               int columnNumber)
    : name(name),
      symbolType(type),
      dataType(dataType),
      lineNumber(lineNumber),
      columnNumber(columnNumber),
      scopeLevel(0),
      isInitialized(false) {}

// SymbolTable implementation
SymbolTable::SymbolTable() : currentScopeLevel(0) {
    // Create global scope
    enterScope();
}

SymbolTable::~SymbolTable() {
    // Clean up all scopes
    while (!scopes.empty()) {
        exitScope();
    }
}

void SymbolTable::enterScope() {
    Scope newScope;
    newScope.level = currentScopeLevel++;
    scopes.push_back(newScope);
}

void SymbolTable::exitScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
        currentScopeLevel--;
    }
}

int SymbolTable::getCurrentScope() const {
    return currentScopeLevel - 1;
}

bool SymbolTable::insert(const Symbol& symbol) {
    if (scopes.empty()) {
        return false;
    }

    // Check if symbol already exists in current scope
    if (isDefinedInCurrentScope(symbol.getName())) {
        return false;
    }

    // Create a copy of the symbol and set its scope level
    Symbol scopedSymbol = symbol;
    scopedSymbol.setScopeLevel(getCurrentScope());

    // Insert into current scope
    scopes.back().symbols[symbol.getName()] = scopedSymbol;
    return true;
}

bool SymbolTable::insertFunction(const std::string& name, int line, int column) {
    Symbol func(name, SymbolType::FUNCTION, DataType::NONE, line, column);
    return insert(func);
}

bool SymbolTable::insertVariable(const std::string& name, DataType type, int line, int column) {
    Symbol var(name, SymbolType::VARIABLE, type, line, column);
    return insert(var);
}

std::optional<Symbol> SymbolTable::lookup(const std::string& name) const {
    // Search from innermost to outermost scope
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto symbolIt = it->symbols.find(name);
        if (symbolIt != it->symbols.end()) {
            return symbolIt->second;
        }
    }
    return std::nullopt;
}

std::optional<Symbol> SymbolTable::lookupInCurrentScope(const std::string& name) const {
    if (scopes.empty()) {
        return std::nullopt;
    }

    auto& currentScope = scopes.back();
    auto it = currentScope.symbols.find(name);
    if (it != currentScope.symbols.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool SymbolTable::isDefined(const std::string& name) const {
    return lookup(name).has_value();
}

bool SymbolTable::isDefinedInCurrentScope(const std::string& name) const {
    return lookupInCurrentScope(name).has_value();
}

void SymbolTable::printTable() const {
    std::cout << "\nSymbol Table Contents:\n";
    std::cout << "=====================\n\n";
    
    for (size_t i = 0; i < scopes.size(); ++i) {
        std::cout << "=== Scope Level " << i << " ===\n";
        for (const auto& [name, symbol] : scopes[i].symbols) {
            std::cout << "Name: " << std::left << std::setw(15) << name 
                     << " | Type: " << std::setw(10) << symbolTypeToString(symbol.getSymbolType())
                     << " | Data Type: " << std::setw(10) << dataTypeToString(symbol.getDataType())
                     << " | Line: " << std::setw(4) << symbol.getLineNumber()
                     << " | Column: " << symbol.getColumnNumber() << "\n";
        }
        std::cout << "\n";
    }
}

// Utility functions implementation
std::string symbolTypeToString(SymbolType type) {
    switch (type) {
        case SymbolType::VARIABLE: return "Variable";
        case SymbolType::FUNCTION: return "Function";
        case SymbolType::CLASS: return "Class";
        case SymbolType::PARAMETER: return "Parameter";
        case SymbolType::MODULE: return "Module";
        case SymbolType::UNKNOWN: return "Unknown";
        default: return "Invalid";
    }
}

std::string dataTypeToString(DataType type) {
    switch (type) {
        case DataType::INTEGER: return "Integer";
        case DataType::FLOAT: return "Float";
        case DataType::STRING: return "String";
        case DataType::BOOLEAN: return "Boolean";
        case DataType::LIST: return "List";
        case DataType::DICT: return "Dictionary";
        case DataType::NONE: return "None";
        case DataType::UNKNOWN: return "Unknown";
        default: return "Invalid";
    }
}
