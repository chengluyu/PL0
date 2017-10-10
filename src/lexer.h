#ifndef PL_ZERO_LEXER_H
#define PL_ZERO_LEXER_H

#include <istream>
#include <sstream>
#include <stdexcept>

#include "token.h"

namespace pl0 {

#ifdef _MSC_VER
namespace polyfill {

template <typename T, typename... Args>
void fold_write_stream(std::ostringstream &oss, T value, Args... args) {
	oss << value;
	fold_write_stream(oss, args...);
}

template <typename T>
void fold_write_stream(std::ostringstream &oss, T value) {
	oss << value;
}

}
#endif

class lexical_error {
	std::string message_;
public:
	template <typename... Args>
	lexical_error(Args... args) {
		std::ostringstream oss;
#ifdef _MSC_VER
		// Visual Studio 2017 does not support fold expression now.
		// We need to make a polyfill.
		polyfill::fold_write_stream(oss, args...);
#else
		oss << ... << args;
#endif
		message_ = oss.str();
	}

	const std::string &what() const { return message_; }
};

class lexer {
	std::istream &input_stream_;
	token peek_;
public:
	lexer(std::istream &input_stream) : input_stream_(input_stream) {
		advance();
	}

	void advance();

	bool peek(token_type tk) {
		return peek_.first == tk;
	}

	token next() {
		token save = std::move(peek_);
		advance();
		return save;
	}

	token expect(token_type tk) {
		if (peek_.first == tk) return next();
		throw lexical_error("expect ", *tk, " instead of ", *peek_.first);
	}

	std::optional<token> match(token_type tk) {
		return peek_.first == tk ? std::make_optional(peek_) : std::make_optional<token>();
	}
};

}

#endif