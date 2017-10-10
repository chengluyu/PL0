#ifndef PL_ZERO_TOKEN_H
#define PL_ZERO_TOKEN_H

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace pl0 {

#define IGNORE_TOKEN(name, string)

#define TOKEN_LIST(T, K)\
	/* End of source indicator */ \
	T(EOS, "end of source") \
	T(NUMBER, "number") \
	T(IDENTIFIER, "identifier") \
	/* Binary operators */ \
	T(ADD, "+") \
	T(SUB, "-") \
	T(MUL, "*") \
	T(DIV, "/") \
	T(ASSIGN, ":=") \
	/* Compare operators */ \
	T(EQ, "=") \
	T(NEQ, "#") \
	T(LE, "<") \
	T(LEQ, "<=") \
	T(GE, ">") \
	T(GEQ, ">=") \
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
	K(THEN, "then") \
	K(VAR, "var") \
	K(WHILE, "while") \
	K(WRITE, "write") \
	T(ILLEGAL, "illegal token")

#define T(name, string) name,
enum class token_type : int {
	TOKEN_LIST(T, T)
};
#undef T

#define T(name, string) string,
const char* const token_string[] = {
	TOKEN_LIST(T, T)
};
#undef T

#define K(name, string) { string, token_type::name },
const std::unordered_map<std::string, token_type> keyword_map = {
	TOKEN_LIST(IGNORE_TOKEN, K)
};
#undef K

inline const char* const operator* (token_type tkty) {
	return token_string[static_cast<int>(tkty)];
}

typedef std::pair<token_type, std::optional<std::string>> token;

inline token token_only(token_type tk) {
	return std::make_pair(tk, std::make_optional<std::string>());
}

inline token token_with_string(token_type tk, std::string &&literal) {
	return std::make_pair(tk, std::make_optional(literal));
}

}

#endif
