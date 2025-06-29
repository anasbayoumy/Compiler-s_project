#include <iostream>
#include <fstream>
#include <regex>
#include <unordered_set>
#include <stack>
#include <map>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <string>
#include <filesystem>
using namespace std;

string output[500];
int outputIndex = 0;

unordered_set<string> keywords = {
    "False", "await", "else", "import", "pass", "None", "break", "except",
    "in",  "True", "finally", "is", "return", "and", "continue",
    "for", "try", "as", "def", "from", "while", "not", "with", "elif", "if", "or"
};

bool isIdentifier(const string& word) {
    return regex_match(word, regex("[A-Za-z_][A-Za-z0-9_]*"));
}

bool isNumber(const string& word) {
    return regex_match(word, regex("\\d+(\\.\\d+)?"));
}

void storeOutput(const string& msg) {
    if (outputIndex < 500)
        if(msg.size() > 0)
            output[outputIndex++] = msg;
}

void processToken(const string& token, int lineNumber) {
    if (keywords.count(token))
    storeOutput("Line " + to_string(lineNumber) + " - Keyword: " + token);
    else if (isNumber(token))
        storeOutput("Line " + to_string(lineNumber) + " - Number: " + token);
    else if (regex_match(token, regex("\\d+[A-Za-z_]+[A-Za-z0-9_]*")))
        storeOutput("Line " + to_string(lineNumber) + " - Error Invalid Identifier: " + token);
    else if (isIdentifier(token))
        storeOutput("Line " + to_string(lineNumber) + " - Identifier: " + token);
    else
        storeOutput("Line " + to_string(lineNumber) + " - Unknown: " + token);
}

bool isCommentLine(const string& line) {
    for (char ch : line) {
        if (isspace(ch)) continue;
        return ch == '#';  // if the first non-space char is #
    }
    return false; // empty line is not a comment
}

void handleIndentation(const string& line, int lineNumber, stack<int>& indentLevels) {
    int spaces = 0;
    for (char c : line) {
        if (c == ' ') spaces++;
        else break;
    }

    if (spaces % 4 != 0) {
        storeOutput("Line " + to_string(lineNumber) + " - Indentation Error: Not a multiple of 4");
        return;
    }

    int currentIndent = indentLevels.top();
    if (spaces > currentIndent) {
        indentLevels.push(spaces);
        storeOutput("Line " + to_string(lineNumber) + " - INDENT");
    } else if (spaces < currentIndent) {
        while (spaces < indentLevels.top()) {
            indentLevels.pop();
            storeOutput("Line " + to_string(lineNumber) + " - DEDENT");
        }
    }
}

void analyzeLine(string& line, int lineNumber, stack<char>& brackets, ifstream& file) {
    string word;
    for (size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];

        // Handle triple quotes
        if ((ch == '"' || ch == '\'') && i + 2 < line.size() &&
            line[i + 1] == ch && line[i + 2] == ch) {
            if (!word.empty()) {
                processToken(word, lineNumber);
                word.clear();
            }
            break;
        }

        // Handle single-line strings
        if (ch == '"' || ch == '\'') {
            if (!word.empty()) {
                processToken(word, lineNumber);
                word.clear();
            }
            char quote = ch;
            string strToken;
            strToken.push_back(ch);
            size_t start = i;
            ++i;
            bool terminated = false;
            while (i < line.size()) {
                char c = line[i];
                strToken.push_back(c);
                if (c == quote && line[i - 1] != '\\') {
                    terminated = true;
                    break;
                }
                ++i;
            }
            if (terminated) {
                storeOutput("Line " + to_string(lineNumber) + " - String: " + strToken);
            } else {
                storeOutput("Line " + to_string(lineNumber) + " - Syntax Error: Unterminated string: " + strToken);
            }
            continue;
        }

        // Build token
        if (isalnum(ch) || ch == '_') {
            word.push_back(ch);
        } else {
            if (!word.empty()) {
                processToken(word, lineNumber);
                word.clear();
            }

            if (ch == '(' || ch == '{' || ch == '[') {
                brackets.push(ch);
                storeOutput("Line " + to_string(lineNumber) + " - Symbol (opening bracket): " + ch);
            } else if (ch == ')' || ch == '}' || ch == ']') {
                if (brackets.empty() ||
                    (ch == ')' && brackets.top() != '(') ||
                    (ch == '}' && brackets.top() != '{') ||
                    (ch == ']' && brackets.top() != '[')) {
                    storeOutput("Line " + to_string(lineNumber) + " - Syntax Error: Mismatched bracket '" + string(1, ch));
                } else {
                    brackets.pop();
                    storeOutput("Line " + to_string(lineNumber) + " - Symbol (closing bracket): " + string(1, ch));
                }
            } else if (!isspace(ch) && !isalnum(ch) && ch != '_') { ////////////////////////
                storeOutput("Line " + to_string(lineNumber) + " - Symbol: " + string(1, ch));
            }
        }
    }

    if (!word.empty())
        processToken(word, lineNumber);




    // Immediate unmatched bracket check at end of line
    bool hasCloser = false;
    for (char ch : line) {
        if (ch == ')' || ch == '}' || ch == ']') {
            hasCloser = true;
            break;
        }
    }
    if (!hasCloser && !brackets.empty()) {
        storeOutput("Line " + to_string(lineNumber) + " - Syntax Error: Unmatched opening bracket(s)");
        while (!brackets.empty()) brackets.pop();
    }
}

void printOutput() {
    for (int i = 0; i < outputIndex; ++i) {
        cout << output[i] << '\n';
    }
}

string to_lower(string s) {
    for (char& c : s) c = tolower(c);
    return s;
}

struct SymbolInfo {
    string name;
    string type = "N/A";
    string value = "N/A";
};

map<string, SymbolInfo> symbolTable;

vector<string> sanitize_tokens_vector(const vector<string>& token_lines) {
    vector<string> sanitized_tokens;

    regex malformed_double_assign(R"(<symbol;\s*=>\s*>\s*<symbol;\s*=>\s*>)");
    regex float_parts(R"(<number;\s*(\d+)\s*> <symbol;\s*\.{1}\s*> <number;\s*(\d+)\s*>)");
    regex func_call(R"(<id;\s*([^>]+)\s*>\s*<symbol;\s*\(\s*>)");
    regex number_token(R"(<number;\s*(\d+)\s*>)");
    regex math_expr(R"(<(id|number|float|int);\s*([^>]+)\s*> <symbol;\s*([+\-*/])\s*> <(id|number|float|int);\s*([^>]+)\s*>)");

    for (string line : token_lines) {
        line = regex_replace(line, malformed_double_assign, "<symbol; ==>");

        // Handle math expressions
        smatch math_match;
        while (regex_search(line, math_match, math_expr)) {
            string left_type = math_match[1], left_val = math_match[2];
            string op = math_match[3];
            string right_type = math_match[4], right_val = math_match[5];

            if (left_type == "id" && symbolTable.count(left_val))
                left_val = symbolTable[left_val].value, left_type = symbolTable[left_val].type;
            if (right_type == "id" && symbolTable.count(right_val))
                right_val = symbolTable[right_val].value, right_type = symbolTable[right_val].type;

            double left = (left_type == "float") ? stod(left_val) : stoi(left_val);
            double right = (right_type == "float") ? stod(right_val) : stoi(right_val);
            double result = 0;
            string result_type = "int";

            if (op == "+") result = left + right;
            else if (op == "-") result = left - right;
            else if (op == "*") result = left * right;
            else if (op == "/") {
                if (right == 0) break;
                result = left / right;
                result_type = "float";
            }

            if (result_type == "int" && result != int(result))
                result_type = "float";

            string result_token = (result_type == "int")
                ? "<int; " + to_string(int(result)) + ">"
                : "<float; " + to_string(result) + ">";

            line.replace(math_match.position(0), math_match.length(0), result_token);
        }

        // Handle floats like 2 . 4 → <float; 2.4>
        smatch float_match;
        while (regex_search(line, float_match, float_parts)) {
            string fullFloat = float_match[1].str() + "." + float_match[2].str();
            line.replace(float_match.position(0), float_match.length(0), "<float; " + fullFloat + ">");
        }

        // Replace numbers with ints
        smatch number_match;
        while (regex_search(line, number_match, number_token)) {
            string intVal = number_match[1].str();
            line.replace(number_match.position(0), number_match.length(0), "<int; " + intVal + ">");
        }

        // Replace function names
        smatch func_match;
        while (regex_search(line, func_match, func_call)) {
            string funcName = func_match[1].str();
            string replacement = "<Function; " + funcName + ">";
            size_t pos = line.find("<id; " + funcName + ">", func_match.position(0));
            if (pos != string::npos)
                line.replace(pos, 8 + funcName.size() + 1, replacement);
        }

        // List detection (like: <id; pp> <symbol; => <symbol; [> <int; 1> <symbol; ,> ... <symbol; ]>)
        smatch list_match;
        regex list_expr(R"(<id;\s*(\w+)\s*>\s*<symbol;\s*=>\s*<symbol;\s*\[>\s*((?:<(int|float);\s*[^>]+>\s*(?:<symbol;\s*,>\s*)?)*)<symbol;\s*\]>)");

        // Save line number prefix like "[1] "
        string line_number_prefix = "";
        size_t bracket_pos = line.find(']');
        if (bracket_pos != string::npos && line[0] == '[') {
            line_number_prefix = line.substr(0, bracket_pos + 1) + " ";
            line = line.substr(bracket_pos + 2); // Skip "] "
        }

        if (regex_search(line, list_match, list_expr)) {
            string var = list_match[1];
            string inner = list_match[2];

            // Extract all numbers
            regex item(R"(<(int|float);\s*([^>]+)\s*>)");
            smatch m;
            string items = "[";
            string temp = inner;
            bool first = true;
            while (regex_search(temp, m, item)) {
                if (!first) items += ",";
                items += m[2].str();
                temp = m.suffix();
                first = false;
            }
            items += "]";

            line = "<id; " + var + "> <symbol; => <list; " + items + ">";
            symbolTable[var] = SymbolInfo{var, "list", items};
        }

        // Reattach line number
        line = line_number_prefix + line;

        // Assignment handling for int/float
        smatch assign_match;
        if (regex_search(line, assign_match, regex(R"(<id;\s*(\w+)\s*>\s*<symbol;\s*=>\s*<(int|float);\s*([\d\.]+)\s*>)"))) {
            string var = assign_match[1];
            string type = assign_match[2];
            string val = assign_match[3];
            symbolTable[var] = SymbolInfo{var, type, val};
        }

        // Replace " with '
        size_t pos;
        while ((pos = line.find('"')) != string::npos)
            line.replace(pos, 1, "'");

        // Fix malformed >ymbol
        string from1 = ">ymbol;";
        string to1 = "> <symbol;";
        size_t pos1 = line.find(from1);
        if (pos1 != string::npos)
            line.replace(pos1, from1.length(), to1);

        // Handle bools
        line = regex_replace(
            line,
            regex(R"(<id;\s*(\w+)\s*>\s*<symbol;\s*=>\s*<keyword;\s*(True|False)\s*>)"),
            "<id; $1> <symbol; => <bool; $2>"
        );

        sanitized_tokens.push_back(line);
    }

    // Add dedent if needed
    if (!sanitized_tokens.empty() && sanitized_tokens.back().find("<indent; indent>") != string::npos) {
        string last_line = "[" + to_string(sanitized_tokens.size() + 2) + "] <dedent; dedent>";
        sanitized_tokens.push_back(last_line);
    }

    return sanitized_tokens;
}

vector<string> parse_token_lines(const string arr[], int size) {
    vector<string> result;
    int current_line = -1;
    string current_tokens = "";

    map<string, string> token_map = {
        {"Keyword", "keyword"},
        {"Identifier", "id"},
        {"Symbol", "symbol"},
        {"Symbol (opening bracket)", "symbol"},
        {"Symbol (closing bracket)", "symbol"},
        {"Number", "number"},
        {"String", "string"},
        {"INDENT", "indent"},
        {"DEDENT", "dedent"},
        {"Syntax Error", "error"},
        {"Error Invalid Identifier", "error"}
    };

    regex full_regex(R"(Line (\d+) - ([^:]+): (.+))");
    regex short_regex(R"(Line (\d+) - (.+))");

    string line;
    for (int i = 0; i < size; ++i) {
        line = arr[i];
        if (line.empty()) continue;

        smatch match;
        int line_num = -1;
        string token_type, token_value;

        if (regex_match(line, match, full_regex)) {
            line_num = stoi(match[1]);
            token_type = match[2];
            token_value = match[3];
        } else if (regex_match(line, match, short_regex)) {
            line_num = stoi(match[1]);
            token_type = match[2];
            token_value = "";
        } else {
            continue;
        }

        if (current_line != line_num) {
            if (current_line != -1) {
                result.push_back("[" + to_string(current_line) + "] " + current_tokens);
            }
            current_line = line_num;
            current_tokens = "";
        }

        if (token_type == "INDENT" || token_type == "DEDENT") {
            current_tokens += "<" + to_lower(token_type) + "; " + to_lower(token_type) + "> ";
        } else if (token_type.find("Error") != string::npos) {
            current_tokens += "<error; " + token_value + "> ";
        } else {
            string category = token_map.count(token_type) ? token_map[token_type] : "unknown";
            current_tokens += "<" + category + "; " + token_value + "> ";
        }
    }

    if (!current_tokens.empty()) {
        result.push_back("[" + to_string(current_line) + "] " + current_tokens);
    }

    // Post-processing
    for (string& line : result) {
        // Replace double quotes with single quotes
        size_t pos2;
        while ((pos2 = line.find('"')) != string::npos) {
            line.replace(pos2, 1, "'");
        }

        // Convert boolean assignments
        line = regex_replace(
            line,
            regex(R"(<id;\s*(\w+)\s*>\s*<symbol;\s*=>\s*<keyword;\s*(True|False)\s*>)"),
            "<id; $1> <symbol; => <bool; $2>"
        );

        // Convert numbers to int or float
        line = regex_replace(
            line,
            regex(R"(<number;\s*([+-]?\d+)\s*>)"),
            "<int; $1>"
        );
        line = regex_replace(
            line,
            regex(R"(<number;\s*([+-]?\d*\.\d+)\s*>)"),
            "<float; $1>"
        );
    }
    if (!result.empty() && result.back().find("<indent; indent>") != string::npos) {
            string last_line = "[" + to_string(result.size()+2) + "] <dedent; dedent>";
            result.push_back(last_line);
        }
    return result;
}


void build_and_draw_symbol_table(const vector<string>& token_lines) {
    map<string, SymbolInfo> symbol_map;

    regex id_pattern(R"(<id;\s*([^>]+)\s*>)");
    regex assign_match(R"(<id;\s*(\w+)\s*>\s*<symbol;\s*=>\s*<(int|float|string|bool|list);\s*([^>]+)\s*>)");
    regex value_pattern(R"(<(float|int|string|id|bool|list);\s*([^>]+)\s*>)");

    // NEW: Direct assignment pattern <id; x> <symbol; => <int; 30>
    regex direct_assignment_pattern(R"(<id;\s*([^>]+)\s*>\s*<symbol;\s*=>\s*>\s*<(float|int|string|id);\s*([^>]+)\s*>)");

    for (const string& line : token_lines) {
        if (line.find("<Function;") != string::npos) continue;

        // NEW: Match full direct assignments first
        smatch assign_match;
        if (regex_search(line, assign_match, direct_assignment_pattern)) {
            string id = assign_match[1];
            string type = assign_match[2];
            string value = assign_match[3];

            // Insert or update symbol in map with type and value
            symbol_map[id] = SymbolInfo{id, type, value};
        }

        // Existing logic for id patterns
        sregex_iterator it(line.begin(), line.end(), id_pattern);
        sregex_iterator end;

        while (it != end) {
            string id = (*it)[1];

            // If the symbol is already present, don't add it again
            if (symbol_map.count(id) == 0) {
                symbol_map[id] = SymbolInfo{id};  // Default to empty type and value
            }

            size_t id_pos = it->position();
            size_t eq_pos = line.find("<symbol; =>", id_pos);
            if (eq_pos != string::npos) {
                smatch match;
                string rest = line.substr(eq_pos);
                if (regex_search(rest, match, value_pattern)) {
                    // Update the symbol's type and value
                    symbol_map[id].type = match[1];
                    symbol_map[id].value = match[2];
                }
            }

            ++it;
        }
    }

    // Output the symbol table
    cout << "Index  |  ID      | Type    | Value\n";
    cout << "-------------------------------------\n";
    int index = 0;
    for (const auto& [name, info] : symbol_map) {
        cout << setw(6) << index++ << " | "
             << setw(8) << info.name << " | "
             << setw(7) << (info.type.empty() ? "N/A" : info.type) << " | "
             << (info.type.empty() ? "N/A" : info.value) << "\n";
    }
}

ifstream flatten_multiline_file(ifstream& inputFile) {
    vector<string> result;
    bool in_multiline = false;
    string multiline_delim;
    string combined;

    string line;
    while (getline(inputFile, line)) {
        if (!in_multiline) {
            size_t pos_triple_double = line.find("\"\"\"");
            size_t pos_triple_single = line.find("'''");

            if (pos_triple_double != string::npos || pos_triple_single != string::npos) {
                in_multiline = true;
                multiline_delim = (pos_triple_double != string::npos) ? "\"\"\"" : "'''";
                combined = line;

                size_t end_pos = line.find(multiline_delim, (pos_triple_double != string::npos ? pos_triple_double + 3 : pos_triple_single + 3));
                if (end_pos != string::npos && end_pos > (pos_triple_double != string::npos ? pos_triple_double + 2 : pos_triple_single + 2)) {
                    in_multiline = false;
                    // Replace all triple quotes with "
                    while (combined.find(multiline_delim) != string::npos)
                        combined.replace(combined.find(multiline_delim), 3, "\"");
                    result.push_back(combined);
                }
            } else {
                result.push_back(line);
            }
        } else {
            combined += " " + line;
            if (line.find(multiline_delim) != string::npos) {
                in_multiline = false;
                // Replace all triple quotes with "
                while (combined.find(multiline_delim) != string::npos)
                    combined.replace(combined.find(multiline_delim), 3, "\"");
                result.push_back(combined);
            }
        }
    }

    // Save to a temporary file
    ofstream tempOut("temp_flattened.py");
    for (const string& l : result) {
        tempOut << l << "\n";
    }
    tempOut.close();

    return ifstream("temp_flattened.py");
}

void saveTokensToFile(const vector<string>& tokens) {
    ofstream outFile("Tokens.txt");
    if (!outFile) {
        cerr << "Error: Could not open Tokens.txt for writing.\n";
        return;
    }
    
    for (const string& token : tokens) {
        outFile << token<< '\n';
    }

    outFile.close();
}
/////////////////////////////////////////////////////////////////////////////// parser

struct ParseNode {
    string type;
    string value;
    vector<shared_ptr<ParseNode>> children;

    ParseNode(string t, string v = "") : type(t), value(v) {}
};
// Token structure
struct Token {
    string type;
    string value;
    int line;

    Token(string t, string v, int l) : type(t), value(v), line(l) {}
};

class Parser {
private:
    vector<Token> tokens;
    size_t current = 0;
    int nodeCounter = 0;

    // Helper function to check if we've reached the end
    bool isAtEnd() {
        return current >= tokens.size();
    }

    // Helper function to peek at current token
    Token peek() {
        if (isAtEnd()) return Token("EOF", "", -1);
        return tokens[current];
    }

    // Helper function to peek ahead n positions
    Token peekAhead(size_t n = 1) {
        if (current + n >= tokens.size()) return Token("EOF", "", -1);
        return tokens[current + n];
    }

    // Helper function to check if current token matches expected type
    bool check(const string& type) {
        return !isAtEnd() && tokens[current].type == type;
    }

    // Helper function to advance and return previous token
    Token advance() {
        if (!isAtEnd()) current++;
        return tokens[current - 1];
    }

    // Helper function to match and consume token if it matches
    bool match(const vector<string>& types) {
        for (auto& t : types) {
            if (check(t)) {
                advance();
                return true;
            }
        }
        return false;
    }

    // Helper function to consume token of expected type
    Token consume(const string& type, const string& msg) {
        if (check(type)) return advance();
        throw runtime_error(msg + " (found '" + peek().type + ":" + peek().value + "' instead)");
    }


    shared_ptr<ParseNode> consumeNode(const string& expectedToken, const string& errorMessage) {
        if (!check(expectedToken)) {
            throw runtime_error(errorMessage);
        }
        auto token = advance();  // consume the token
        return make_shared<ParseNode>(expectedToken);  // create a node for the token
 }
    // Parse a file input
    shared_ptr<ParseNode> program() {
        auto node = make_shared<ParseNode>("program");
        node->children.push_back(stmt_list());
        if (!isAtEnd()) {
            Token t = consume("ENDMARKER", "Expected ENDMARKER");
            node->children.push_back(make_shared<ParseNode>("ENDMARKER", t.value));
        }
        return node;
    }

    // Parse a statement list
    shared_ptr<ParseNode> stmt_list() {
        auto node = make_shared<ParseNode>("stmt_list");
        while (!isAtEnd() && !check("ENDMARKER")) {
            if (check("NEWLINE")) { advance(); continue; }
            node->children.push_back(stmt());
            if (check("NEWLINE")) advance();
        }
        return node;
    }

    // Parse a statement
    shared_ptr<ParseNode> stmt() {
        if (isSimpleStmt()) return simple_stmts();
        return block_stmt();
    }

    // Check for simple statement
    bool isSimpleStmt() {
        if (check("NAME") || check("pass") || check("break") || check("continue") || check("return") || check("import") || check("from")) return true;
        return false;
    }

    // Parse simple statements
    shared_ptr<ParseNode> simple_stmts() {
        auto node = make_shared<ParseNode>("simple_stmts");
        node->children.push_back(small_stmt());
        while (match({";"})) {
            if (check("NEWLINE") || isAtEnd()) break;
            node->children.push_back(small_stmt());
        }
        return node;
    }

    // Parse a small statement
    shared_ptr<ParseNode> small_stmt() {
        if (check("NAME") && current + 1 < tokens.size() &&
            (tokens[current+1].type == "=" ||
             tokens[current+1].type == "+=" ||
             tokens[current+1].type == "-=" ||
             tokens[current+1].type == "*=" ||
             tokens[current+1].type == "/=" ||
             tokens[current+1].type == "%=" ||
             tokens[current+1].type == "//="))
            return assignment();

        if (check("pass") || check("break") || check("continue") || check("return"))
            return control_flow();

        if (check("import") || check("from"))
            return declaration();

        if (check("NAME"))
            return invocation(); // call function

        throw runtime_error("Unknown small statement type: " + peek().type);
    }

    // Parse a control flow statement
    shared_ptr<ParseNode> control_flow() {
        if (match({"pass"})) return make_shared<ParseNode>("pass_stmt");
        if (match({"break"})) return make_shared<ParseNode>("break_stmt");
        if (match({"continue"})) return make_shared<ParseNode>("continue_stmt");
        if (match({"return"})) {
            auto node = make_shared<ParseNode>("return_stmt");
            if (!check("NEWLINE") && !check(";")) node->children.push_back(expr());
            return node;
        }
        throw runtime_error("Unknown control flow statement");
    }

    // Parse a declaration statement
    shared_ptr<ParseNode> declaration() {
        if (match({"import"})) return import_decl();
        if (match({"from"})) return import_decl();
        throw runtime_error("Unknown declaration statement");
    }

    // Parse an import declaration
    shared_ptr<ParseNode> import_decl() {
        auto node = make_shared<ParseNode>("import_decl");
        if (tokens[current-1].type == "import") {
            node->children.push_back(module_ref());
            if (match({"as"})) {
                node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected NAME after 'as'").value));
            }
            while (match({","})) {
                node->children.push_back(module_ref());
                if (match({"as"})) {
                    node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected NAME after 'as'").value));
                }
            }
        } else if (tokens[current-1].type == "from") {
            node->children.push_back(module_ref());
            consume("import", "Expected 'import'");
            if (check("NAME")) {
                node->children.push_back(make_shared<ParseNode>("NAME", advance().value));
                if (match({"as"})) {
                    node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected NAME after 'as'").value));
                }
            } else if (match({"*"})) {
                node->children.push_back(make_shared<ParseNode>("*"));
            }
        }
        return node;
    }

    // Parse a module reference
    shared_ptr<ParseNode> module_ref() {
        auto node = make_shared<ParseNode>("module_ref");
        node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected module name").value));
        while (match({"."})) {
            node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected name after '.'").value));
        }
        return node;
    }

    // Parse an assignment statement
    shared_ptr<ParseNode> assignment() {
        auto node = make_shared<ParseNode>("assignment");
        node->children.push_back(targets());  // x
        node->children.push_back(assign_op());  // =
        node->children.push_back(exprs());  // a / b (with exprs as parent)
        return node;
    }

    // Parse targets
    shared_ptr<ParseNode> targets() {
        auto node = make_shared<ParseNode>("targets");
        node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected target name").value));
        while (match({","})) {
            node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected name after ','").value));
        }
        return node;
    }

    // Parse an assignment operator
    shared_ptr<ParseNode> assign_op() {
        if (match({"=", "+=", "-=", "*=", "/=", "%=", "//="})) {
            return make_shared<ParseNode>("assign_op", tokens[current-1].type);
        }
        throw runtime_error("Expected assignment operator");
    }

    // Parse expressions
    shared_ptr<ParseNode> exprs() {
        auto node = make_shared<ParseNode>("exprs");
        auto expr_node = expr();
        // Flatten the expression tree into exprs children
        if (expr_node->value.empty() && !expr_node->children.empty()) {
            // Operator expression (like a / b)
            for (auto& child : expr_node->children) {
                node->children.push_back(child);
            }
        } else {
            // Simple value (like NUMBER: 2)
            node->children.push_back(expr_node);
        }
        return node;
    }

    // Parse an invocation
    shared_ptr<ParseNode> invocation() {
        auto node = make_shared<ParseNode>("invocation");
        node->children.push_back(callable());
        node->children.push_back(consumeNode("(", "Expected '(' after function name"));  // now '(' is a node

        if (!check(")")) {
            node->children.push_back(arguments());  // optional arguments
        }

        node->children.push_back(consumeNode(")", "Expected ')' to close function call"));  // ')' node
        return node;
    }

    // Parse a callable
    shared_ptr<ParseNode> callable() {
        if (check("NAME")) {
            return make_shared<ParseNode>("NAME", advance().value);
        }
        return module_ref();
    }

    // Parse arguments
    shared_ptr<ParseNode> arguments() {
        auto node = make_shared<ParseNode>("arguments");
        node->children.push_back(expr());
        while (match({","})) {
            if (check(")")) break; // Handle trailing comma
            node->children.push_back(expr());
        }
        return node;
    }

    // Parse a block statement
    shared_ptr<ParseNode> block_stmt() {
        if (check("if")) return conditional();
        if (check("while")) return loop();
        if (check("for")) return loop();
        if (check("def")) return definition();
        if (check("class")) return definition();
        throw runtime_error("Unknown block statement type: " + peek().type);
    }

    // Parse a conditional statement
    shared_ptr<ParseNode> conditional() {
        auto node = make_shared<ParseNode>("conditional");
        node->children.push_back(if_chain());
        if (match({"else"})) {
            consume(":", "Expected ':' after else");
            node->children.push_back(suite());
        }
        return node;
    }

    // Parse an if chain
    shared_ptr<ParseNode> if_chain() {
        auto node = make_shared<ParseNode>("if_chain");
        consume("if", "Expected 'if'");
        node->children.push_back(comparison_expr());
        consume(":", "Expected ':' after condition");
        node->children.push_back(suite());
        while (match({"elif"})) {
            node->children.push_back(comparison_expr());
            consume(":", "Expected ':' after elif condition");
            node->children.push_back(suite());
        }
        return node;
    }

    // Special comparison expression handler for if statements
    shared_ptr<ParseNode> comparison_expr() {
        auto left = expr();

        // Debug output
        cout << "In comparison_expr. Current token: " << (current < tokens.size() ? tokens[current].type : "EOF") << endl;

        // Special case for handling comparison operators
        if (current < tokens.size()) {
            string opType = tokens[current].type;

            // Debug the token
            cout << "Checking operator: " << opType << endl;

            if (opType == "==" || opType == "<" || opType == ">" ||
                opType == ">=" || opType == "<=" || opType == "!=" || opType == "=") {

                advance();
                auto op = make_shared<ParseNode>(opType);
                op->children.push_back(left);
                op->children.push_back(expr());
                return op;
            }
        }

        return left;
    }

    // Parse a loop statement
    shared_ptr<ParseNode> loop() {
        if (match({"while"})) {
            auto node = make_shared<ParseNode>("while_loop");
            node->children.push_back(expr());
            consume(":", "Expected ':' after while condition");
            node->children.push_back(suite());
            return node;
        }
        if (match({"for"})) {
            auto node = make_shared<ParseNode>("for_loop");
            node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected loop variable").value));
            consume("in", "Expected 'in' after loop variable");
            node->children.push_back(expr());
            consume(":", "Expected ':' after for loop iterable");
            node->children.push_back(suite());
            return node;
        }
        throw runtime_error("Unknown loop type");
    }

    // Parse a definition statement
    shared_ptr<ParseNode> definition() {
        if (match({"def"})) return func_def();
        if (match({"class"})) return class_def();
        throw runtime_error("Unknown definition type");
    }

    // Parse a function definition
    shared_ptr<ParseNode> func_def() {
        auto node = make_shared<ParseNode>("func_def");
        node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected function name").value));
        node->children.push_back(params());
        consume(":", "Expected ':' after function parameters");
        node->children.push_back(suite());
        return node;
    }

    // Parse function parameters
    shared_ptr<ParseNode> params() {
        consume("(", "Expected '(' after function name");
        auto node = make_shared<ParseNode>("params");
        if (!check(")")) {
            node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected parameter name").value));
            while (match({","})) {
                if (check(")")) break; // Handle trailing comma
                node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected parameter name").value));
            }
        }
        consume(")", "Expected ')' to close parameter list");
        return node;
    }

    // Parse a class definition
    shared_ptr<ParseNode> class_def() {
        auto node = make_shared<ParseNode>("class_def");
        node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected class name").value));
        if (match({"("})) {
            node->children.push_back(make_shared<ParseNode>("NAME", consume("NAME", "Expected parent class name").value));
            consume(")", "Expected ')' to close parent class list");
        }
        consume(":", "Expected ':' after class definition");
        node->children.push_back(suite());
        return node;
    }

    // Parse a suite (indented block)
    shared_ptr<ParseNode> suite() {
        auto node = make_shared<ParseNode>("suite");

        // Debug output
        cout << "In suite. Current token: " << peek().type << " '" << peek().value << "'" << endl;

        // Handle the case where we have an INDENT token directly
        if (check("INDENT")) {
            advance();
            while (!check("DEDENT") && !isAtEnd()) {
                node->children.push_back(stmt());
            }
            if (check("DEDENT")) advance();
            return node;
        }

        // Handle the case where we need a NEWLINE followed by INDENT
        if (check("NEWLINE")) {
            advance();
            if (check("INDENT")) {
                advance();
                while (!check("DEDENT") && !isAtEnd()) {
                    node->children.push_back(stmt());
                }
                if (check("DEDENT")) advance();
                return node;
            }
        }

        // If we don't have an indented block, parse a simple statement
        return simple_stmts();
    }

    // Parse an expression
    shared_ptr<ParseNode> expr() {
        return logical_or();
    }

    // Parse a logical OR expression
    shared_ptr<ParseNode> logical_or() {
        auto node = logical_and();
        while (match({"or"})) {
            auto op = make_shared<ParseNode>("or");
            op->children.push_back(node);
            op->children.push_back(logical_and());
            node = op;
        }
        return node;
    }

    // Parse a logical AND expression
    shared_ptr<ParseNode> logical_and() {
        auto node = logical_not();
        while (match({"and"})) {
            auto op = make_shared<ParseNode>("and");
            op->children.push_back(node);
            op->children.push_back(logical_not());
            node = op;
        }
        return node;
    }

    // Parse a logical NOT expression
    shared_ptr<ParseNode> logical_not() {
        if (match({"not"})) {
            auto node = make_shared<ParseNode>("not");
            node->children.push_back(logical_not());
            return node;
        }
        return comparison();
    }

    // Parse a comparison
    shared_ptr<ParseNode> comparison() {
        auto node = arithmetic();
        while (match({"<", ">", "==", ">=", "<=", "!="})) {
            auto op = make_shared<ParseNode>(tokens[current-1].type);
            op->children.push_back(node);
            op->children.push_back(arithmetic());
            node = op;
        }
        return node;
    }

    // Parse an arithmetic expression
    shared_ptr<ParseNode> arithmetic() {
        auto node = term();
        while (match({"+", "-"})) {
            // Create a temporary node to hold the operation
            auto op_node = make_shared<ParseNode>("");
            op_node->children.push_back(node);
            op_node->children.push_back(make_shared<ParseNode>(tokens[current-1].type));
            op_node->children.push_back(term());
            node = op_node;
        }
        return node;
    }

    // Parse a term
    shared_ptr<ParseNode> term() {
        auto node = factor();
        while (match({"*", "/", "//", "%"})) {
            // Create a temporary node to hold the operation
            auto op_node = make_shared<ParseNode>("");
            op_node->children.push_back(node);
            op_node->children.push_back(make_shared<ParseNode>(tokens[current-1].type));
            op_node->children.push_back(factor());
            node = op_node;
        }
        return node;
    }

    // Parse a factor
    shared_ptr<ParseNode> factor() {
        if (match({"+", "-", "~"})) {
            auto node = make_shared<ParseNode>(tokens[current-1].type);
            node->children.push_back(factor());
            return node;
        }
        return primary();  // Resolves to NAME, NUMBER, etc.
    }

    // Parse a primary expression
    shared_ptr<ParseNode> primary() {
        if (check("NUMBER")) return make_shared<ParseNode>("NUMBER", advance().value);
        if (check("BOOL")) return make_shared<ParseNode>("BOOL", advance().value);
        if (check("STRING")) return make_shared<ParseNode>("STRING", advance().value);
        if (check("None") || check("True") || check("False")) return make_shared<ParseNode>(advance().type);
        if (check("NAME")) return make_shared<ParseNode>("NAME", advance().value);
        if (check("(")) return grouped();
        if (check("[")) return list_();
        if (check("{")) return dict_();
        throw runtime_error("Unknown primary expression type");
    }

    // Parse a grouped expression
    shared_ptr<ParseNode> grouped() {
        consume("(", "Expected '('");
        auto node = make_shared<ParseNode>("grouped");
        if (!check(")")) node->children.push_back(expr_list());
        consume(")", "Expected ')'");
        return node;
    }

    // Parse a list expression
    shared_ptr<ParseNode> list_() {
        consume("[", "Expected '['");
        auto node = make_shared<ParseNode>("list");
        if (!check("]")) node->children.push_back(expr_list());
        consume("]", "Expected ']'");
        return node;
    }

    // Parse a dictionary expression
    shared_ptr<ParseNode> dict_() {
        consume("{", "Expected '{'");
        auto node = make_shared<ParseNode>("dict");
        if (!check("}")) node->children.push_back(key_values());
        consume("}", "Expected '}'");
        return node;
    }

    // Parse an expression list
    shared_ptr<ParseNode> expr_list() {
        auto node = make_shared<ParseNode>("expr_list");
        node->children.push_back(expr());
        while (match({","})) {
            if (check(")") || check("]")) break; // Handle trailing comma
            node->children.push_back(expr());
        }
        return node;
    }

    // Parse key-value pairs
    shared_ptr<ParseNode> key_values() {
        auto node = make_shared<ParseNode>("key_values");
        node->children.push_back(expr());
        consume(":", "Expected ':' after dictionary key");
        node->children.push_back(expr());
        while (match({","})) {
            if (check("}")) break; // Handle trailing comma
            node->children.push_back(expr());
            consume(":", "Expected ':' after dictionary key");
            node->children.push_back(expr());
        }
        return node;
    }

    // Generate DOT representation of the parse tree
    void generateDOT(shared_ptr<ParseNode> node, ofstream& dotFile, string parent = "") {
        if (!node) return;
        string nodeId = "node" + to_string(nodeCounter++);
        // Escape special characters in labels
        string label = node->type;
        string value = node->value;

        // Replace special characters that would cause issues in DOT syntax
        auto replaceAll = [](string& str, const string& from, const string& to) {
            size_t pos = 0;
            while((pos = str.find(from, pos)) != string::npos) {
                str.replace(pos, from.length(), to);
                pos += to.length();
            }
        };

        // replaceAll(label, "\"", "\\\"");
        // replaceAll(value, "\"", "\\\"");
        // replaceAll(value, "\\", "\\\\");

        dotFile << nodeId << " [label=\"" << label;
        if (!value.empty()) dotFile << ": " << value;
        dotFile << "\"]\n";

        if (!parent.empty()) dotFile << parent << " -> " << nodeId << ";\n";

        for (auto& child : node->children) {
            generateDOT(child, dotFile, nodeId);
        }
    }

public:
    // Load tokens from file
    void loadTokens(const string& filename) {
        // Attempt to open the file
        ifstream file(filename);
        string line;
        regex lineRegex(R"(\[(\d+)\])");
        regex tokenRegex(R"(<([^;]+);\s*([^>]*)>)");  // Changed to allow empty values
        smatch matches;

        if (!file.is_open()) {
            throw runtime_error("Could not open tokens file: " + filename);
        }

        cout << "Successfully opened token file: " << filename << endl;

        while (getline(file, line)) {
            if (line.empty()) continue;

            if (regex_search(line, matches, lineRegex)) {
                int lineNum = stoi(matches[1]);
                string::const_iterator searchStart(line.cbegin());

                while (regex_search(searchStart, line.cend(), matches, tokenRegex)) {
                    string type = matches[1];
                    string value = matches[2];

                    // For debugging
                    //cout << "Token: " << type << " = " << value << " (line " << lineNum << ")" << endl;           ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                    // Normalize token types according to grammar
                    if (type == "id") type = "NAME";
                    else if (type == "int" || type == "float") type = "NUMBER";
                    else if (type == "string") type = "STRING";
                    else if (type == "bool") type = "BOOL";
                    else if (type == "keyword") type = value;
                    else if (type == "symbol") {
                        // Trim whitespace from value
                        value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));
                        value.erase(value.find_last_not_of(" \t\n\r\f\v") + 1);

                        // Special handling for comparison operators
                        if (value == ">") type = ">";
                        else if (value == "<") type = "<";
                        else if (value == "==") type = "==";
                        else if (value == ">=") type = ">=";
                        else if (value == "<=") type = "<=";
                        else if (value == "!=") type = "!=";
                        else if (value == "=") type = "=";
                        else if (value == "greater") type = ">";  // Handle "greater" as ">"
                        else type = value;
                    }
                    else if (type == "Function") type = "NAME";
                    else if (type == "indent") type = "INDENT";
                    else if (type == "dedent") type = "DEDENT";
                    else if (type == "newline") type = "NEWLINE";

                    // Skip tokens that are just whitespace
                    if (type == " " || type.empty()) {
                        searchStart = matches.suffix().first;
                        continue;
                    }

                    // Add token to list
                    tokens.push_back(Token(type, value, lineNum));
                    searchStart = matches.suffix().first;
                }
            }
        }

        // Add end marker
        //tokens.push_back(Token("ENDMARKER", "<EOF>", -1));

        if (tokens.empty()) {
            throw runtime_error("No tokens found in file");
        }

        cout << "Total tokens loaded: " << tokens.size() << endl;
    }

    // Parse the tokens and generate parse tree
    shared_ptr<ParseNode> parse() {
        try {
            current = 0; // Reset position
            auto root = program();
            if (!root) {
                throw runtime_error("Failed to parse program");
            }
            return root;
        } catch (const exception& e) {
            cerr << "Parse error: " << e.what() << endl;
            if (current < tokens.size()) {
                cerr << "Current token: " << tokens[current].type << " '" << tokens[current].value << "' at line " << tokens[current].line << endl;
            }
            return nullptr;
        }
    }

    // Generate DOT file for visualization
    void generateDOTFile(shared_ptr<ParseNode> root, const string& filename) {
        // Create directories if they don't exist
        ofstream dotFile(filename);
        if (!dotFile.is_open()) {
            throw runtime_error("Could not create DOT file: " + filename);
        }

        dotFile << "digraph ParseTree {\nnode [shape=box];\n";
        nodeCounter = 0;
        generateDOT(root, dotFile);
        dotFile << "}\n";
        dotFile.close();

        cout << "DOT file generated: " << filename << endl;
    }
};

void create_Tree(string dot_file_name,string image_file_name){
 
    // Command to convert DOT to PNG using Graphviz
    string command = "dot -Tpng -Gdpi=300 " + dot_file_name + " -o " + image_file_name;

    int result = system(command.c_str());

    if (result == 0) {
        cout << "Image generated successfully: " << image_file_name << endl;
    } else {
        cerr << "Failed to generate image. Is Graphviz installed ?" << endl;
    }
}

void replaceEmptyLabel(const std::string& inputFilename) {
    std::ifstream input(inputFilename);
    std::ofstream output("parse_tree1.dot");

    if (!input.is_open() || !output.is_open()) {
        std::cerr << "Failed to open input or output file.\n";
        return;
    }

    std::string line;
    while (std::getline(input, line)) {
        size_t pos = line.find("[label=\"\"]");
        if (pos != std::string::npos) {
            line.replace(pos, std::string("[label=\"\"]").length(), "[label=\"arithm-op\"]");
        }
        output << line << "\n";
    }

    std::cout << "Updated file saved as parse_tree.dot\n";
}


int main() {
    ifstream original("test.py");
if (!original) {
    cerr << "Failed to open file.\n";
    return 1;
}
ifstream file = flatten_multiline_file(original);
    if (!file) {
        cerr << "Failed to open file.\n";
        return 1;
    }

    string line;
    int lineNumber = 0;
    stack<char> brackets;
    stack<int> indentLevels;
    indentLevels.push(0);

    while (getline(file, line)) {
        ++lineNumber;
        
        if (isCommentLine(line)) continue; // skip commented lines
        
        if(line.size()>0){
            storeOutput("\n");
            handleIndentation(line, lineNumber, indentLevels);
            analyzeLine(line, lineNumber, brackets, file);
        }
    }

    vector<string> tokens = parse_token_lines(output, outputIndex);
    vector<string> Sanitized_tokens = sanitize_tokens_vector(tokens);
    saveTokensToFile(tokens);

    for (const string& line : Sanitized_tokens) {
        // Check up to the 15th character only
        size_t openBracket = line.find('[');
        size_t closeBracket = line.find(']');

        //string prefix = line.substr(0, 15);
        string numberStr = line.substr(openBracket + 1, closeBracket - openBracket - 1);
        if (line.find("<error;") != string::npos) {
            cout << "\n Error at line "<<numberStr<<": PROGRAM TERMINATED. " << endl;
            return 0;
        }
    }


      cout<< " Sanitized tokens"<<endl;
    
      for (const string& line : Sanitized_tokens) {cout << line << endl;}

     cout << endl;
     cout << endl;
     build_and_draw_symbol_table(Sanitized_tokens);
////////////////////////////////////////////////////////////
try {
        Parser parser;

        // Load tokens from file
        parser.loadTokens("Tokens.txt");

        // Parse tokens and generate parse tree
        auto parseTree = parser.parse();

        if (parseTree) {
            // Generate DOT file for visualization
            parser.generateDOTFile(parseTree, "C:\\Users\\fadij\\Desktop\\Compilers_proj\\parse_tree.dot");
            cout << "Parse tree generated successfully. Use Graphviz to visualize parse_tree.dot" << endl;
        } else {
            cerr << "Failed to generate parse tree" << endl;
        }

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
    
    string dotFile_to_be_modified = "C:\\Users\\fadij\\Desktop\\Compilers_proj\\parse_tree.dot";
    string dotFile_to_be_executed = "C:\\Users\\fadij\\Desktop\\Compilers_proj\\parse_tree1.dot";
    string imgFile = "C:\\Users\\fadij\\Desktop\\Compilers_proj\\myGraph.png";
    replaceEmptyLabel(dotFile_to_be_modified);
    // Command to convert DOT to PNG using Graphviz
    create_Tree(dotFile_to_be_executed, imgFile);

////////////////////////////////////////////////////////////
    // Clean up temporary file
    file.close();
    original.close();
    remove("temp_flattened.py");
    remove("parse_tree.dot");
    }

