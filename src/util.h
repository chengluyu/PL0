#ifndef PL_ZERO_UTIL_H
#define PL_ZERO_UTIL_H

#include <sstream>

namespace pl0 {

#if defined(_MSC_VER) || defined(__GNUC__)
namespace polyfill {

template <typename T>
void fold_write_stream(std::ostringstream &oss, T value) {
    oss << value;
}

template <typename T, typename... Args>
void fold_write_stream(std::ostringstream &oss, T value, Args... args) {
    oss << value;
    fold_write_stream(oss, args...);
}

}
#endif

class general_error {
    std::string message_;
public:
    template <typename... Args>
    general_error(Args... args) {
        std::ostringstream oss;
#if defined(_MSC_VER) || defined(__GNUC__)
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

}

#endif
