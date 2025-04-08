# Python Compiler: Lexical and Syntax Analysis

## Project Overview

This project aims to develop the lexical analysis and syntax analysis phases (i.e., the lexer and parser) of a compiler for the Python programming language. The project will identify and process basic Python language constructs and create a user-friendly graphical interface for interacting with the lexer and parser.

## Project Team

- **Team Size:** 6 students

## Programming Language

- **Implementation Language:** C++

## Language Specifications

The compiler will recognize and process the following constructs of the Python language:

- **Keywords**
- **Variable Identifiers**
- **Function Identifiers**
- **Data Types**
- **Functions**
- **Statements:**
  - Assignment Statement
  - Declaration Statement
  - Return Statement
  - Iterative Statement
  - Conditional Statements
  - Function Call Statement
- **Expressions:**
  - Arithmetic Expressions
  - Boolean Expressions

## Project Deliverables

### 1. Lexical Analysis (Lexer)

The lexer is responsible for reading the input source code file and categorizing its contents into tokens based on Python language specifications.

- **Features:**
  - **Tokenization:** Identify tokens for keywords, identifiers, operators, etc.
  - **Symbol Table Creation:** Build a symbol table to store identifiers.
  - **Graphical User Interface (GUI):** Provide a user interface for viewing the tokenized output.
  - **Lexical Error Handling:** Detect and report lexical errors in the source code.

### 2. Syntax Analysis (Parser)

The parser will process the tokens produced by the lexer to create a parse tree, ensuring the input code adheres to Python’s grammar rules.

- **Features:**
  - **Grammar Rules Integration:** Define and implement the grammar rules for Python.
  - **Parsing Implementation:** Develop the parsing mechanism to process the tokens.
  - **Parse Tree Generation:** Construct a parse tree representing the syntactic structure of the code.
  - **Graphical User Interface (GUI):** Provide a visual representation of the parse tree.
  - **Parsing Error Handling:** Detect and report syntax errors during parsing.

## Getting Started

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/yourusername/your-repo-name.git
   cd your-repo-name
