#include <iostream>
#include <fstream>
#include <regex>
#include <unordered_set>
#include <stack>
#include <map>
#include <iomanip>
#include <vector>
using namespace std;

string output[500];
int outputIndex = 0;

unordered_set<string> keywords = {
    "False", "await", "else", "import", "pass", "None", "break", "except",
    "in", "raise", "True", "class", "finally", "is", "return", "and", "continue",
    "for", "lambda", "try", "as", "def", "from", "nonlocal", "while", "assert",
    "del", "global", "not", "with", "async", "elif", "if", "or", "yield"
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

bool handleTripleQuote(ifstream& file, string& currentLine, size_t& i, int& lineNumber, char quoteType) {
    string triple = string(3, quoteType);
    string str = triple;
    i += 2; // Skip opening quotes

    string line;
    while (getline(file, line)) {
        ++lineNumber;
        str += "\n" + line;
        size_t pos = line.find(triple);
        if (pos != string::npos) {
            str += triple;
            storeOutput("Line " + to_string(lineNumber) + " - String: " + str);
            return true;
        }
    }

    storeOutput("Line " + to_string(lineNumber) + " - Syntax Error: Unterminated multi-line string");
    return false;
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

        // Process math operations iteratively
        smatch math_match;
        while (regex_search(line, math_match, math_expr)) {
            string left_type = math_match[1].str();
            string left_val = math_match[2].str();
            string op = math_match[3].str();
            string right_type = math_match[4].str();
            string right_val = math_match[5].str();

            double left = 0.0, right = 0.0;
            string result_type = "int";

            // Resolve identifiers
            if (left_type == "id" && symbolTable.count(left_val)) {
                left_type = symbolTable[left_val].type;
                left_val = symbolTable[left_val].value;
            }
            if (right_type == "id" && symbolTable.count(right_val)) {
                right_type = symbolTable[right_val].type;
                right_val = symbolTable[right_val].value;
            }

            // Convert values
            left = (left_type == "float") ? stod(left_val) : stoi(left_val);
            right = (right_type == "float") ? stod(right_val) : stoi(right_val);

            double result = 0;
            if (op == "+") result = left + right;
            else if (op == "-") result = left - right;
            else if (op == "*") result = left * right;
            else if (op == "/") {
                if (right == 0) break;
                result = left / right;
                result_type = "float"; // make division default to float
            }

            // Force float if result is not whole
            if (result_type == "int" && result != int(result)) result_type = "float";

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
            if (pos != string::npos) {
                line.replace(pos, 8 + funcName.size() + 1, replacement);
            }
        }

        // Assignment handling and update symbol table
        smatch assign_match;
        if (regex_search(line, assign_match, regex(R"(<id;\s*(\w+)\s*>\s*<symbol;\s*=>\s*<(int|float);\s*([\d\.]+)\s*>)"))) {
            string var = assign_match[1];
            string type = assign_match[2];
            string val = assign_match[3];
            symbolTable[var] = SymbolInfo{var, type, val};
        }

        sanitized_tokens.push_back(line);
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

    for (int i = 0; i < size; ++i) {
        string line = arr[i];
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
            current_tokens += "<" + to_lower(token_type) + "> ";
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

    return result;
}

void build_and_draw_symbol_table(const vector<string>& token_lines) {
    map<string, SymbolInfo> symbol_map;

    regex id_pattern(R"(<id;\s*([^>]+)\s*>)");
    regex assign_pattern(R"(<symbol;\s*=>\s*>)");
    regex value_pattern(R"(<(float|int|string|id);\s*([^>]+)\s*>)");

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
    cout << "Index | ID      | Type    | Value\n";
    cout << "-----------------------------------\n";
    int index = 0;
    for (const auto& [name, info] : symbol_map) {
        cout << setw(6) << index++ << " | "
             << setw(8) << info.name << " | "
             << setw(7) << (info.type.empty() ? "N/A" : info.type) << " | "
             << (info.value.empty() ? "N/A" : info.value) << "\n";
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

      cout<< " Sanitized tokens"<<endl;
    
      for (const string& line : Sanitized_tokens) {cout << line << endl;}

     cout << endl;
     cout << endl;
     build_and_draw_symbol_table(Sanitized_tokens);

    // Clean up temporary file
    file.close();
    original.close();
    remove("temp_flattened.py");
}
