#include"front/lexical.h"

#include<map>
#include<cassert>
#include<string>

#define TODO assert(0 && "todo")

// #define DEBUG_DFA
// #define DEBUG_SCANNER

std::string frontend::toString(State s) {
    switch (s) {
    case State::Empty:
        return "Empty";
    case State::Ident:
        return "Ident";
    case State::IntLiteral:
        return "IntLiteral";
    case State::FloatLiteral:
        return "FloatLiteral";
    case State::op:
        return "op";
    default:
        assert(0 && "invalid State");
    }
    return "";
}

std::set<std::string> frontend::keywords = {
    "const", "int", "float", "if", "else", "while", "continue", "break", "return", "void"
};

frontend::DFA::DFA(): cur_state(frontend::State::Empty), cur_str() {}

frontend::DFA::~DFA() {}
bool space_cr_line(char input) {
    return (input == '\r' || input == '\n' || input == ' ' || input == '\t');
}
bool  isoperator(char  c)  {
    return c == '/' ||
           c == '-' ||
           c == '+' ||
           c == '*' ||
           c == '(' ||
           c == ')' ||
           c == '%' ||
           c == '<' ||
           c == '>' ||
           c == ':' ||
           c == '=' ||
           c == ';' ||
           c == ',' ||
           c == '[' ||
           c == ']' ||
           c == '{' ||
           c == '}' ||
           c == '!' ||
           c == '&' ||
           c == '|';
}

frontend::TokenType get_op_type(std::string s) {
    using namespace frontend;
    if ((int)s.size() == 2) {
        if (s == "<=") return TokenType::LEQ;
        if (s == ">=") return TokenType::GEQ;
        if (s == "==") return TokenType::EQL;
        if (s == "!=") return TokenType::NEQ;
        if (s == "&&") return TokenType::AND;
        if (s == "||") return TokenType::OR;
    }
    if ((int)s.size() == 1) {
        char c = s[0];
        if (c == '+') return TokenType::PLUS;
        if (c == '-') return TokenType::MINU;
        if (c == '*') return TokenType::MULT;
        if (c == '/') return TokenType::DIV;
        if (c == '%') return TokenType::MOD;
        if (c == '<') return TokenType::LSS;
        if (c == '>') return TokenType::GTR;
        if (c == ':') return TokenType::COLON;
        if (c == '=') return TokenType::ASSIGN;
        if (c == ';') return TokenType::SEMICN;
        if (c == ',') return TokenType::COMMA;
        if (c == '(') return TokenType::LPARENT;
        if (c == ')') return TokenType::RPARENT;
        if (c == '[') return TokenType::LBRACK;
        if (c == ']') return TokenType::RBRACK;
        if (c == '{') return TokenType::LBRACE;
        if (c == '}') return TokenType::RBRACE;
        if (c == '!') return TokenType::NOT;
    }
    assert(0 && "unexpected TokenType");
    return TokenType::NOT;
}
bool doubleop(std::string s, char input) {
    s += input;
    if (s == "<=") return true;
    if (s == ">=") return true;
    if (s == "==") return true;
    if (s == "!=") return true;
    if (s == "&&") return true;
    if (s == "||") return true;
    return false;
}
frontend::TokenType get_ident_type(std::string s) {
    using namespace frontend;
    if (s == "const") return TokenType::CONSTTK;
    if (s == "void") return TokenType::VOIDTK;
    if (s == "int") return TokenType::INTTK;
    if (s == "float") return TokenType::FLOATTK;
    if (s == "if") return TokenType::IFTK;
    if (s == "else") return TokenType::ELSETK;
    if (s == "while") return TokenType::WHILETK;
    if (s == "continue") return TokenType::CONTINUETK;
    if (s == "break") return TokenType::BREAKTK;
    if (s == "return") return TokenType::RETURNTK;
    return TokenType::IDENFR;
}
// . 0 a \n \r ' ' op '_'
bool frontend::DFA::next(char input, Token &buf) {
//#define DEBUG_DFA
#ifdef DEBUG_DFA
#include<iostream>
    std::cout << "in state [" << toString(cur_state) << "], input = \'" << input << "\', str = " << cur_str << "\n";
#endif
    switch (cur_state) {
    case State::Empty:
        if (space_cr_line(input)) {
            cur_state = State::Empty;
            cur_str = "";
            return false;
        } else if (isoperator(input)) {
            cur_state = State::op;
            cur_str = input;
            return false;
        } else if (isdigit(input)) {
            cur_state = State::IntLiteral;
            cur_str = input;
            return false;
        } else if (isalpha(input) || input == '_') {
            cur_state = State::Ident;
            cur_str = input;
            return false;
        }
        break;
    case State::Ident:
        if (space_cr_line(input)) {
            cur_state = State::Empty;
            buf.value = cur_str;
            buf.type = get_ident_type(buf.value);
            cur_str = "";
            return true;
        } else if (isoperator(input)) {
            cur_state = State::op;
            buf.value = cur_str;
            buf.type = get_ident_type(buf.value);
            cur_str = input;
            return true;
        } else {
            cur_state = State::Ident;
            cur_str += input;
            return false;
        }
        break;
    case State::IntLiteral:
        if (input == '.') {
            cur_str += input;
            cur_state = State::FloatLiteral;
            return false;
        } else if (space_cr_line(input)) {
            cur_state = State::Empty;
            buf.value = cur_str;
            buf.type = TokenType::INTLTR;
            cur_str = "";
            return true;
        } else if (isoperator(input)) {
            cur_state = State::op;
            buf.value = cur_str;
            buf.type = TokenType::INTLTR;
            cur_str = input;
            return true;
        } else if (isalpha(input) || isdigit(input)) {
            cur_str += input;
            cur_state = State::IntLiteral;
            return false;
        }
        break;
    case State::FloatLiteral:
        if (space_cr_line(input)) {
            cur_state = State::Empty;
            buf.value = cur_str;
            buf.type = TokenType::FLOATLTR;
            cur_str = "";
            return true;
        } else if (isoperator(input)) {
            buf.value = cur_str;
            buf.type = TokenType::FLOATLTR;
            cur_state = State::op;
            cur_str = input;
            return true;
        } else {
            cur_state = State::FloatLiteral;
            cur_str += input;
            return false;
        }
        break;
    case State::op:
        if (space_cr_line(input)) {
            cur_state = State::Empty;
            buf.value = cur_str;
            buf.type = get_op_type(buf.value);
            cur_str = "";
            return true;
        } else if (isdigit(input)) {
            cur_state = State::IntLiteral;
            buf.value = cur_str;
            buf.type = get_op_type(buf.value);
            cur_str = input;
            return true;
        } else if (doubleop(cur_str, input)) {
            cur_state = State::op;
            cur_str += input;
            return false;
        } else if (isoperator(input)) {
            cur_state = State::op;
            buf.value = cur_str;
            buf.type = get_op_type(buf.value);
            cur_str = input;
            return true;
        } else if (isalpha(input) || input == '_') {
            cur_state = State::Ident;
            buf.value = cur_str;
            buf.type = get_op_type(buf.value);
            cur_str = input;
            return true;
        } else if (input == '.') {
            cur_state = State::FloatLiteral;
            buf.value = cur_str;
            buf.type = get_op_type(buf.value);
            cur_str = input;
            return true;
        }
        break;
    }
    std::cout << toString(cur_state) << std::endl;
    std::cout << cur_str << std::endl;
    std::cout << input << std::endl;
    assert(0 && "unexpected dfa next");
    return false;
}
void frontend::DFA::reset() {
    cur_state = State::Empty;
    cur_str = "";
}
frontend::Scanner::Scanner(std::string filename): fin(filename) {
    if (!fin.is_open()) {
        assert(0 && "in Scanner constructor, input file cannot open");
    }
}
frontend::Scanner::~Scanner() {
    fin.close();
}
std::vector<frontend::Token> frontend::Scanner::run() {
    std::string s;
    int hint = 0;
    DFA dfa;
    std::vector<frontend::Token> tokens;
    frontend::Token tk;
    while (std::getline(fin, s)) {
        // std::cout << s;
        for (int i = 0; i < (int)s.size(); i++) {
            if (hint && i < (int)s.size() - 1 && s[i] == '*' && s[i + 1] == '/') {
                hint = 0;
                i++;
                continue;
            }
            if (!hint && i < (int)s.size() - 1 && s[i] == '/' && s[i + 1] == '/') break;
            if (!hint && i < (int)s.size() - 1 && s[i] == '/' && s[i + 1] == '*') {
                hint = 1;
                i++;
            }
            if (hint == 0 && dfa.next(s[i], tk)) {
                tokens.push_back(tk);
// #define DEBUG_SCANNER
#ifdef DEBUG_SCANNER
#include<iostream>
                std::cout << "token: " << toString(tk.type) << "\t" << tk.value << std::endl;
#endif
            }
        }
    }
    if (dfa.next(' ', tk)) {
        tokens.push_back(tk);
    }
    return tokens;
}