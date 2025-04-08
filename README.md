# Python-like Compiler Project

A compiler project that implements a Python-like language with lexical analysis, symbol table management, and error handling capabilities.

## Project Structure

```
project/
├── src/
│   ├── lexer/              # Lexical Analysis
│   │   ├── lexer.hpp      # Lexer class definition
│   │   ├── lexer.cpp      # Lexer implementation
│   │   └── lexer_test.cpp # Lexer test cases
│   │
│   ├── symbol_table/      # Symbol Table Management
│   │   ├── STable.hpp     # Symbol Table class definition
│   │   ├── STable.cpp     # Symbol Table implementation
│   │   └── symbol_table_test.cpp # Symbol Table test cases
│   │
│   ├── Error_Handeling/   # Error Handling System
│   │   └── Error.hpp      # Error handling definitions
│   │
│   └── GUI/              # Graphical User Interface
│       └── mainwindow.hpp # Main window class definition
```

## Features

### 1. Lexical Analysis
- Tokenizes Python-like source code
- Handles keywords, identifiers, numbers, strings, and operators
- Supports indentation-based block structure
- Processes comments and whitespace

### 2. Symbol Table Management
- Hierarchical scope management (global, function, block)
- Type inference for variables
- Function and parameter tracking
- Support for multiple data types:
  - INTEGER
  - FLOAT
  - STRING
  - BOOLEAN
  - LIST
  - DICT
  - NONE
  - UNKNOWN

### 3. Error Handling
- Comprehensive error reporting system
- Line and column number tracking
- Multiple error types support
- Detailed error messages

### 4. GUI Interface
- Modern Qt-based interface
- Source code input area
- Token visualization
- Symbol table display
- Error message panel

## Building the Project

### Prerequisites
- C++17 compatible compiler
- CMake 3.10 or higher
- Qt6 development libraries
- Make or Ninja build system

### Build Steps
1. Create build directory:
   ```bash
   mkdir build
   cd build
   ```

2. Configure with CMake:
   ```bash
   cmake ..
   ```

3. Build the project:
   ```bash
   cmake --build .
   ```

## Testing

The project includes test cases for both the lexer and symbol table components:

### Lexer Tests
```bash
cd src/lexer
g++ -o lexer_test lexer.cpp lexer_test.cpp
./lexer_test
```

### Symbol Table Tests
```bash
cd src/symbol_table
g++ -o symbol_table_test symbol_table_test.cpp STable.cpp ../lexer/lexer.cpp -I../
./symbol_table_test
```

## Usage Example

The compiler can process Python-like code such as:

```python
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
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.
