#include <cassert>
#include <functional>
#include <iostream>
#include <type_traits>
#include <unordered_map>

#include "vm.h"

const int STACK_SIZE = 1024;

enum offset {
    return_address = 0,
    enclosing_frame = 1,
    declaration_frame = 2,
    local = 3
};

void pl0::execute(const bytecode & code, int entry_addr) {
    int pc = 0;
    int *stack = new int[STACK_SIZE];
    int bp = 0;
    int sp = bp;
    
    auto push = [&](int value) {
        stack[++sp] = value;
    };

    auto pop = [&]() -> int {
        return stack[sp--];
    };

    const std::unordered_map<opt, std::function<int(int, int)>> bifunctors = {
        { opt::ADD, std::plus<int>() },
        { opt::SUB, std::minus<int>() },
        { opt::DIV, std::divides<int>() },
        { opt::MUL, std::multiplies<int>() },
        { opt::LE, std::less<int>() },
        { opt::LEQ, std::less_equal<int>() },
        { opt::GE, std::greater<int>() },
        { opt::GEQ, std::greater_equal<int>() },
        { opt::EQ, std::equal_to<int>() },
        { opt::NEQ, std::not_equal_to<int>() }
    };

    int codelen = static_cast<int>(code.size());
    stack[bp + offset::return_address] = codelen;

    while (pc < codelen) {
        opcode opc;
        int level, address;
        std::tie(opc, level, address) = code[pc];
        pc++;

        /* get bp of the frame whose distance from current frame is given */
        auto resolve = [&]() -> int {
            int fp = bp, dist = level;
            while (dist > 0) {
                fp = stack[fp + offset::declaration_frame];
                dist--;
            }
            return fp;
        };

        switch (opc) {
        case opcode::LIT:
            push(address);
            break;
        case opcode::LOD:
            push(stack[resolve() + offset::local + address]);
            break;
        case opcode::STO:
            stack[resolve() + offset::local + address] = pop();
            break;
        case opcode::CAL:
            // save context
            stack[sp + offset::return_address] = pc;
            stack[sp + offset::enclosing_frame] = bp;
            stack[sp + offset::declaration_frame] = resolve();
            bp = sp;
            pc = address;
            break;
        case opcode::INT:
            sp += address;
            break;
        case opcode::JMP:
            pc = address;
            break;
        case opcode::JPC:
            if (!pop()) pc = address;
            break;
        case opcode::OPR:
            if (address == *opt::ODD) {
                push(pop() % 2);
            } else if (address == *opt::READ) {
                int tmp;
                std::cin >> tmp;
                push(tmp);
            } else if (address == *opt::WRITE) {
                std::cout << pop() << '\n';
            } else if (address == *opt::RET) {
                // restore context
                pc = stack[bp];
                sp = bp;
                bp = stack[bp + 1];
            } else {
                int rhs = pop(), lhs = pop();
                auto op = bifunctors.find(opt(address))->second;
                push(op(lhs, rhs));
            }
            break;
        }
    }
}
