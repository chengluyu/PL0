#ifndef PL_ZERO_VM_H
#define PL_ZERO_VM_H

#include <functional>

#include "bytecode/bytecode.h"

namespace pl0 {

class stack_frame {
    int return_address_;
    stack_frame *dynamic_link_;
    stack_frame *static_link_;
    std::vector<std::pair<std::string, int>> locals_;
    std::vector<int> intermediates_;
public:
    stack_frame(int ret_address, stack_frame *dyn_link, stack_frame *static_link)
            : return_address_(ret_address), dynamic_link_(dyn_link), static_link_(static_link) { }

    /**
     * Destroy and immediately return to enclosing stack frame
     * @param return_address
     * @param frame_pointer
     */
    void leave(int &return_address, stack_frame *&frame_pointer) {
        return_address = return_address_;
        frame_pointer = dynamic_link_;
        delete this;
    }

    stack_frame *resolve(int level_dist) {
        stack_frame *target_frame = this;
        while (level_dist > 0) {
            target_frame = target_frame->static_link_;
            level_dist--;
        }
        return target_frame;
    }

    void allocate(int count) {
        for (int i = 0; i < count; i++)
            locals_.emplace_back(std::pair{ std::string{ }, 0 });
    }

    int &local(int level_dist, int index) {
        return resolve(level_dist)->locals_[index].second;
    }

    void push(int value) {
        intermediates_.push_back(value);
    }

    int pop() {
        int result = intermediates_.back();
        intermediates_.pop_back();
        return result;
    }
};

const std::unordered_map<opt, std::function<int (int, int)>> opt2functor = {
    { opt::ADD, std::plus<>() },
    { opt::SUB, std::minus<>() },
    { opt::DIV, std::divides<>() },
    { opt::MUL, std::multiplies<>() },
    { opt::LE, std::less<>() },
    { opt::LEQ, std::less_equal<>() },
    { opt::GE, std::greater<>() },
    { opt::GEQ, std::greater_equal<>() },
    { opt::EQ, std::equal_to<>() },
    { opt::NEQ, std::not_equal_to<>() }
};

void execute(const bytecode &code);

}

#endif
