#ifndef PL_ZERO_UTIL_H
#define PL_ZERO_UTIL_H

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

class general_error {
    std::string message_;
public:
    template <typename... Args>
    general_error(Args... args) {
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

}

#endif
