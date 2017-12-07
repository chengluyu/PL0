#ifndef PL0_ARGPARSER_H
#define PL0_ARGPARSER_H

#include <queue>
#include <string>
#include <unordered_map>
#include "util.h"

namespace pl0 {

template <class TargetType>
class argument_parser {

    class handler {
        int arg_count_;
    public:
        explicit handler(int arg_count) : arg_count_{arg_count} { }

        virtual void apply(const std::vector<std::string> &args, TargetType &target) = 0;

        int arg_count() const { return arg_count_; }
    };

    template <typename PropertyType, typename Formatter>
    class store_handler : public handler {
        PropertyType TargetType::*setter_;

        Formatter formatter_;
    public:
        store_handler(PropertyType TargetType::*setter, Formatter formatter)
                : handler(1), setter_{setter}, formatter_{std::move(formatter)} { }

        void apply(const std::vector<std::string> &args, TargetType &target) final {
            target.*setter_ = formatter_(args[0]);
        }
    };

    template <typename PropertyType>
    class store_handler<PropertyType, void> : public handler {
        PropertyType TargetType::*setter_;
    public:
        explicit store_handler(PropertyType TargetType::*setter)
                : handler(1), setter_{setter} { }

        void apply(const std::vector<std::string> &args, TargetType &target) final {
            target.*setter_ = args[0];
        }
    };

    class flag_handler : public handler {
        bool TargetType::*setter_;

        bool store_value_;
    public:
        explicit flag_handler(bool TargetType::*setter, bool store_value)
                : handler(0), setter_{setter}, store_value_{store_value} { }

        void apply(const std::vector<std::string> &args, TargetType &target) final {
            target.*setter_ = store_value_;
        }
    };

    template <typename Callable>
    class custom_handler : public handler {
        Callable callable_;
    public:
        explicit custom_handler(Callable callable) : handler(0), callable_{std::move(callable)} { }

        void apply(const std::vector<std::string> &args, TargetType &target) final {
            callable_();
        }
    };

    std::unordered_map<std::string, handler *> handlers_;
    std::string description_;

    template <typename IterableStringList>
    inline void add_handler(IterableStringList list, handler *the_handler) {
        for (auto name : list)
            handlers_[name] = the_handler;
    }

    static bool is_flag(const std::string &flag) {
        if (flag.length() > 1 && flag[0] == '-') {
            size_t i = flag[1] == '-' ? 1 : 0;
            do {
                i++;
                while (i < flag.length() && isalnum(flag[i]))
                    i++;
            } while (i < flag.length() && (flag[i] == '-' || flag[i] == '_'));
            return i == flag.length();
        }
        return false;
    }
public:
    explicit argument_parser(std::string description) : description_{std::move(description)} { }

    ~argument_parser() = default;

    [[noreturn]] void show_help() {
        std::cout << description_;
        exit(EXIT_SUCCESS);
    }

    template <typename IterableStringList>
    void store(IterableStringList flags, std::string TargetType::*setter) {
        add_handler(flags, new store_handler<std::string, void>(setter));
    }

    template <typename IterableStringList, typename PropertyType, typename Formatter>
    void store(IterableStringList flags, PropertyType TargetType::*setter, Formatter formatter) {
        add_handler(flags, new store_handler<PropertyType, Formatter>(setter, formatter));
    };

    void flags(const std::vector<std::string> &flags, bool TargetType::*setter, bool default_value = true) {
        add_handler(flags, new flag_handler(setter, default_value));
    }

    void parse(int argc, const char *argv[], TargetType &target, std::vector<std::string> &rest) {
        if (argc < 2)
            show_help();

        rest.clear();

        std::queue<std::string> args;
        for (int i = 1; i < argc; i++)
            args.push(argv[i]);

        while (!args.empty()) {
            std::string arg = args.front();
            args.pop();

            if (is_flag(arg)) {
                auto result = handlers_.find(arg);
                if (result == handlers_.end())
                    throw basic_error("unsupported option '" + arg + '\'');
                handler *the_handler = result->second;

                std::vector<std::string> params;
                for (int i = 0; i < the_handler->arg_count(); i++) {
                    if (args.empty())
                        throw basic_error(arg + " requires " + std::to_string(the_handler->arg_count()));
                    params.push_back(args.front());
                    args.pop();
                }
                the_handler->apply(params, target);
            } else {
                rest.push_back(arg);
            }
        }
    }
};

}

#endif //PL0_ARGPARSER_H
