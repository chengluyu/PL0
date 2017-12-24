#include <cctype>

#include "lexer.h"

namespace pl0 {

inline bool is_identifier_start(int ch) {
    return isalpha(ch) || ch == '_';
}

inline bool is_identifier_part(int ch) {
    return isalnum(ch) || ch == '_';
}

int lexer::get() {
    loc_.column++;
    return input_stream_.get();;
}

token lexer::select(char cond, token conseq, token altern) {
    if (input_stream_.peek() == cond) {
        get();
        return conseq;
    } else {
        return altern;
    }
}

void lexer::advance() {
    // check if already end of source or meet illegal chars
    if (peek_ == token::EOS || peek_ == token::ILLEGAL) {
        return;
    }
    // ignore whitespaces
    while (isspace(input_stream_.peek())) {
        if (get() == '\n') {
            loc_.line++;
            loc_.column = 1;
        }
    }
    // check if input stream has ended
    if (input_stream_.peek() == std::char_traits<char>::eof()) {
        peek_ = token::EOS;
        return;
    }
    // clear literal buffer
    literal_buffer_.clear();
    // identifier or keyword
    if (is_identifier_start(input_stream_.peek())) {
        do {
            literal_buffer_.push_back(get());
        } while (is_identifier_part(input_stream_.peek()));
        auto iter = keyword_map.find(literal_buffer_);
        peek_ = iter == keyword_map.end() ? token::IDENTIFIER : iter->second;
        return;
    }
    // number
    if (isdigit(input_stream_.peek())) {
        do {
            literal_buffer_.push_back(get());
        } while (isdigit(input_stream_.peek()));
        peek_ = token::NUMBER;
        return;
    }
    // punctuator, operator or unrecognized character
    switch (get()) {
    case '+': peek_ = token::ADD; break;
    case '-': peek_ = token::SUB; break;
    case '*': peek_ = token::MUL; break;
    case '/': peek_ = token::DIV; break;
    case ':': peek_ = select('=', token::ASSIGN, token::ILLEGAL); break;
    case '(': peek_ = token::LPAREN; break;
    case ')': peek_ = token::RPAREN; break;
    case ';': peek_ = token::SEMICOLON; break;
    case '.': peek_ = token::PERIOD; break;
    case ',': peek_ = token::COMMA; break;
    case '=': peek_ = token::EQ; break;
    case '#': peek_ = token::NEQ; break;
    case '<': peek_ = select('=', token::LEQ, token::LE); break;
    case '>': peek_ = select('=', token::GEQ, token::GE); break;
    default:  peek_ = token::ILLEGAL; break;
    }
}


}
