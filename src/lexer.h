#ifndef PL_ZERO_LEXER_H
#define PL_ZERO_LEXER_H

#include <istream>
#include <sstream>
#include <stdexcept>

#include "token.h"
#include "util.h"

namespace pl0 {

class lexer {
    std::istream &input_stream_;
    token peek_;
public:
    lexer(std::istream &input_stream) : input_stream_(input_stream) {
        advance();
    }

    void advance();

    token_type peek() {
        return peek_.first;
    }

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
        throw general_error("expect ", *tk, " instead of ", *peek_.first);
    }

    std::optional<token> match(token_type tk) {
        return peek_.first == tk ? std::make_optional(next()) : std::nullopt;
    }
};

}

#endif