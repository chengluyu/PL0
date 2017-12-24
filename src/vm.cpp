#include <cassert>
#include <functional>
#include <iostream>
#include <unordered_map>

#include "vm.h"

void pl0::execute(const bytecode & code) {
    int program_counter = 0;
    auto code_length = static_cast<int>(code.size());
    auto *top_frame = new stack_frame{ code_length, nullptr, nullptr };

    while (program_counter < code_length) {
        auto ins = code[program_counter++];

        switch (ins.op) {
        case opcode::LIT:
            top_frame->push(ins.address);
            break;
        case opcode::LOD:
            top_frame->push(top_frame->local(ins.level, ins.address));
            break;
        case opcode::STO:
            top_frame->local(ins.level, ins.address) = top_frame->pop();
            break;
        case opcode::CAL:
            top_frame = new stack_frame{ program_counter, top_frame, top_frame->resolve(ins.level) };
            program_counter = ins.address;
            break;
        case opcode::INT:
            top_frame->allocate(ins.address - 3);
            break;
        case opcode::JMP:
            program_counter = ins.address;
            break;
        case opcode::JPC:
            if (!top_frame->pop()) {
                program_counter = ins.address;
            }
            break;
        case opcode::OPR:
            if (ins.address == *opt::ODD) {
                int result = top_frame->pop() % 2;
                top_frame->push(result);
            } else if (ins.address == *opt::READ) {
                int tmp;
                std::cin >> tmp;
                top_frame->push(tmp);
            } else if (ins.address == *opt::WRITE) {
                std::cout << top_frame->pop() << '\n';
            } else if (ins.address == *opt::RET) {
                top_frame->leave(program_counter, top_frame);
            } else {
                int rhs = top_frame->pop(), lhs = top_frame->pop();
                auto f = opt2functor.find(opt(ins.address))->second;
                top_frame->push(f(lhs, rhs));
            }
            break;
        }
    }
}
