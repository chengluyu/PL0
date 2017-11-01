#ifndef PL_ZERO_TOKEN_H
#define PL_ZERO_TOKEN_H

#include <string>
#include <unordered_map>
#include <utility>

namespace pl0 {

#define IGNORE_TOKEN(name, string)

#define TOKEN_LIST(T, O, K)\
    T(UNUSED, "unused") \
    /* End of source indicator */ \
    T(EOS, "end of source") \
    T(NUMBER, "number") \
    T(IDENTIFIER, "identifier") \
    /* Unary operator */ \
    O(ODD, "odd") \
    /* Binary operators */ \
    O(ADD, "+") \
    O(SUB, "-") \
    O(MUL, "*") \
    O(DIV, "/") \
    T(ASSIGN, ":=") \
    /* Compare operators */ \
    O(EQ, "=") \
    O(NEQ, "#") \
    O(LE, "<") \
    O(LEQ, "<=") \
    O(GE, ">") \
    O(GEQ, ">=") \
    /* Punctuators */ \
    T(LPAREN, "(") \
    T(RPAREN, ")") \
    T(SEMICOLON, ";") \
    T(PERIOD, ".") \
    T(COMMA, ",") \
    /* Keywords */ \
    K(BEGIN, "begin") \
    K(CALL, "call") \
    K(CONST, "const") \
    K(DO, "do") \
    K(ELSE, "else") \
    K(END, "end") \
    K(IF, "if") \
    K(PROCEDURE, "procedure") \
    K(READ, "read") \
    K(RETURN, "return") \
    K(THEN, "then") \
    K(VAR, "var") \
    K(WHILE, "while") \
    K(WRITE, "write") \
    T(ILLEGAL, "illegal token")

#define T(name, string) name,
enum class token : int {
    TOKEN_LIST(T, T, T)
};
#undef T

#define T(name, string) string,
const char* const token_string[] = {
    TOKEN_LIST(T, T, T)
};
#undef T

#define K(name, string) { string, token::name },
const std::unordered_map<std::string, token> keyword_map = {
    TOKEN_LIST(IGNORE_TOKEN, IGNORE_TOKEN, K)
};
#undef K

inline const char* const operator* (token tkty) {
    return token_string[static_cast<int>(tkty)];
}

inline bool is_compare_operator(token tk) {
    return static_cast<int>(token::EQ) <= static_cast<int>(tk) &&
           static_cast<int>(tk) <= static_cast<int>(token::GEQ);
}

}

#endif
