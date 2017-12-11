#ifndef PL_ZERO_UTIL_H
#define PL_ZERO_UTIL_H

#include <sstream>

namespace pl0 {

#if defined(__clang__)
#if __clang_major__ > 3 and __clang_minor__ >= 9
#define NEED_CONCAT_POLYFILL 0
#else
#define NEED_CONCAT_POLYFILL 1
#endif
#elif defined(__GNUC__)
#if __GNUC__ >= 6
#define NEED_CONCAT_POLYFILL 0
#else
#define NEED_CONCAT_POLYFILL 1
#endif
#elif defined(_MSC_VER)
#define NEED_CONCAT_POLYFILL 1
#else
#error Unsupported compiler.
#endif

#if NEED_CONCAT_POLYFILL == 1

namespace polyfill {

template <typename T>
inline void write_stream(std::ostringstream &oss, T value) {
    oss << value;
}

template <typename T, typename... Args>
inline void write_stream(std::ostringstream &oss, T value, Args... args) {
    oss << value;
    write_stream(oss, args...);
}

}

template <typename ... Args>
inline std::string concat(Args ... args) {
    std::ostringstream oss;
    polyfill::write_stream(oss, args...);
    return oss.str();
}

#else

template <typename ... Args>
inline std::string concat(Args ... args) {
    std::ostringstream oss;
    (oss << ... << args);
    return oss.str();
}

#endif

class location {
    int line_;
    int column_;
public:
    location(int line, int column) : line_(line), column_(column) { }

    std::string to_string() const {
        return std::to_string(line_) + ':' + std::to_string(column_);
    }
};

class basic_error {
    std::string message_;
public:
    explicit basic_error(std::string message) : message_(std::move(message)) { }

    virtual const std::string &what() const { return message_; }
};

class general_error : public basic_error {
public:
    template <typename... Args>
    explicit general_error(Args... args) : basic_error(concat(args...)) { }
};

class syntax_error : public basic_error {
    location loc_;
public:
    template <typename ... Args>
    explicit syntax_error(location loc, Args ... args) : loc_(loc), basic_error(concat(args...)) { }
};


}

#endif
