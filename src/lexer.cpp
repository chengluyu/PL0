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
    // clear buffer
    literal_buffer_.clear();
    // ignore whitespaces
    while (isspace(input_stream_.peek())) {
        input_stream_.ignore();
    }
    // check if input stream has ended
    if (input_stream_.peek() == std::char_traits<char>::eof()) {
        peek_ = token::EOS;
        return;
    }
    // identifier or keyword
    if (is_identifier_start(input_stream_.peek())) {
        do {
            literal_buffer_.push_back(input_stream_.get());
        } while (is_identifier_part(input_stream_.peek()));
        auto iter = keyword_map.find(literal_buffer_);
        if (iter == keyword_map.end()) {
            peek_ = token::IDENTIFIER;
        } else {
            peek_ = iter->second;
        }
        return;
    }
    // number
    if (isdigit(input_stream_.peek())) {
        do {
            literal_buffer_.push_back(input_stream_.get());
        } while (isdigit(input_stream_.peek()));
        peek_ = token::NUMBER;
        return;
    }
    // punctuator or operator
    switch (input_stream_.get()) {
    case '+': peek_ = token::ADD; break;
    case '-': peek_ = token::SUB; break;
    case '*': peek_ = token::MUL; break;
    case '/': peek_ = token::DIV; break;
    case ':':
        if (input_stream_.peek() == '=') {
            peek_ = token::ASSIGN;
            input_stream_.ignore();
        } else {
            peek_ = token::ILLEGAL;
            input_stream_.unget();
        }
        break;
    case '(': peek_ = token::LPAREN; break;
    case ')': peek_ = token::RPAREN; break;
    case ';': peek_ = token::SEMICOLON; break;
    case '.': peek_ = token::PERIOD; break;
    case ',': peek_ = token::COMMA; break;
    case '=': peek_ = token::EQ; break;
    case '#': peek_ = token::NEQ; break;
    case '<':
        if (input_stream_.peek() == '=') {
            peek_ = token::LEQ;
            input_stream_.ignore();
        } else {
            peek_ = token::LE;
        }
        break;
    case '>':
        if (input_stream_.peek() == '=') {
            peek_ = token::GEQ;
            input_stream_.ignore();
        } else {
            peek_ = token::GE;
        }
        break;
    default:
        peek_ = token::ILLEGAL;
        input_stream_.unget();
        break;
    }
}

}
