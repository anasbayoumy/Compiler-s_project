# Python Syntax Analyzer (Parser)

This is a syntax analyzer (parser) for Python code that works with the tokens produced by the lexer. It implements a recursive descent parser that builds a parse tree from the token stream.

## Features

- Parses Python-like syntax including:
  - Variable assignments
  - Expressions (arithmetic, function calls)
  - Control structures (if/elif/else, while, for)
  - Function definitions
  - Class definitions
  - Import statements
  - Return statements
- Builds a detailed parse tree
- Provides comprehensive error handling and reporting
- Integrates with the existing lexer

## Files

- `parser_logic.cpp` - The main parser implementation with full syntax analysis
- `simple_parser.cpp` - A simplified parser that just builds a basic parse tree
- `test_tokens.txt` - Example token file for testing
- `tokens.txt` - Another example token file

## How to Use

### Compiling the Parser

```bash
# Compile the main parser
g++ -std=c++17 -Wall -Wextra parser_logic.cpp -o parser_logic

# Compile the simplified parser
g++ -std=c++17 -Wall -Wextra simple_parser.cpp -o simple_parser
```

### Running the Parser

```bash
# Run the main parser
./parser_logic [input_tokens_file] [output_parse_tree_file]

# Run the simplified parser
./simple_parser [input_tokens_file] [output_parse_tree_file]
```

If no input file is provided, the program will use example tokens for testing.

### Input Format

The parser expects tokens in the following format:

```
[line_number] <token_type; token_value> <token_type; token_value> ...
```

For example:

```
[1] <keyword; def> <id; factorial> <symbol; (> <id; n> <symbol; )> <symbol; :>
[2] <indent;> <keyword; if> <id; n> <symbol; <=> <int; 1> <symbol; :>
[3] <indent;> <keyword; return> <int; 1> <dedent;>
[4] <keyword; else> <symbol; :>
[5] <indent;> <keyword; return> <id; n> <symbol; *> <id; factorial> <symbol; (> <id; n> <symbol; -> <int; 1> <symbol; )> <dedent;> <dedent;>
```

## Parse Tree Structure

The parse tree is a hierarchical representation of the program structure. Each node has:

- A type (e.g., "Program", "Assignment", "Expression")
- An optional value (e.g., variable name, function name)
- A line number for error reporting
- Child nodes representing nested structures

## Grammar Rules

The parser implements the following grammar rules:

- Program → Statement*
- Statement → Assignment | IfStatement | WhileLoop | ForLoop | FunctionDefinition | ClassDefinition | ImportStatement | ReturnStatement | Expression
- Assignment → Identifier "=" Expression
- Expression → Value (Operator Value)?
- Value → Identifier | Number | String | FunctionCall
- IfStatement → "if" Condition ":" Block ("elif" Condition ":" Block)* ("else" ":" Block)?
- WhileLoop → "while" Condition ":" Block
- ForLoop → "for" Identifier "in" Expression ":" Block
- FunctionDefinition → "def" Identifier "(" Parameters ")" ":" Block
- ClassDefinition → "class" Identifier ("(" BaseClasses ")")? ":" Block
- ImportStatement → "import" Identifier ("as" Identifier)?
- ReturnStatement → "return" Expression?
- Block → Indent Statement+ Dedent | Statement
- Condition → Expression (ComparisonOperator Expression)?

## Error Handling

The parser provides detailed error messages with line numbers. When an error is encountered, the parser attempts to recover and continue parsing to find additional errors.

## Integration with GUI

To integrate this parser with the GUI, you'll need to:

1. Add the parser files to your project
2. Call the parser after the lexer has processed the input
3. Display the parse tree in the GUI

## Example

Input Python code:
```python
def factorial(n):
    if n <= 1:
        return 1
    else:
        return n * factorial(n - 1)
```

This will produce a parse tree with the function definition, if statement, and return statements properly structured.
