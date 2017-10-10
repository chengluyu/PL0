#include <cctype>

#include "lexer.h"

namespace pl0 {

inline bool is_identifier_start(int ch) {
	return isalpha(ch) || ch == '_';
}

inline bool is_identifier_part(int ch) {
	return isalnum(ch) || ch == '_';
}

void lexer::advance() {
	// ignore whitespaces
	while (isspace(input_stream_.peek())) {
		input_stream_.ignore();
	}
	// check if input stream has ended
	if (input_stream_.peek() == std::char_traits<char>::eof()) {
		peek_ = token_only(token_type::EOS);
		return;
	}
	// identifier or keyword
	if (is_identifier_start(input_stream_.peek())) {
		std::string str;
		do {
			str.push_back(input_stream_.get());
		} while (is_identifier_part(input_stream_.peek()));
		auto iter = keyword_map.find(str);
		if (iter == keyword_map.end()) {
			peek_ = token_with_string(token_type::IDENTIFIER, std::move(str));
		} else {
			peek_ = token_only(iter->second);
		}
		return;
	}
	// number
	if (isdigit(input_stream_.peek())) {
		std::string num;
		do {
			num.push_back(input_stream_.get());
		} while (isdigit(input_stream_.peek()));
		peek_ = token_with_string(token_type::NUMBER, std::move(num));
		return;
	}
	// punctuator or operator
	int copy;
	// save a copy for raising exception
	switch (copy = input_stream_.get()) {
	case '+': peek_ = token_only(token_type::ADD); break;
	case '-': peek_ = token_only(token_type::SUB); break;
	case '*': peek_ = token_only(token_type::MUL); break;
	case '/': peek_ = token_only(token_type::DIV); break;
	case ':':
		if (input_stream_.peek() == '=') {
			peek_ = token_only(token_type::ASSIGN);
			input_stream_.ignore();
			break;
		}
		throw lexical_error("expect '=' after ':'");
	case '(': peek_ = token_only(token_type::LPAREN); break;
	case ')': peek_ = token_only(token_type::RPAREN); break;
	case ';': peek_ = token_only(token_type::SEMICOLON); break;
	case '.': peek_ = token_only(token_type::PERIOD); break;
	case ',': peek_ = token_only(token_type::COMMA); break;
	case '=': peek_ = token_only(token_type::EQ); break;
	case '#': peek_ = token_only(token_type::NEQ); break;
	case '<':
		if (input_stream_.peek() == '=') {
			peek_ = token_only(token_type::LEQ);
			input_stream_.ignore();
		} else {
			peek_ = token_only(token_type::LE);
		}
		break;
	case '<=':  break;
	case '>':
		if (input_stream_.peek() == '=') {
			peek_ = token_only(token_type::GEQ);
			input_stream_.ignore();
		} else {
			peek_ = token_only(token_type::GE);
		}
		break;
	default:
		throw lexical_error("unrecognized character: '", static_cast<char>(copy), '\'');
	}
}

}