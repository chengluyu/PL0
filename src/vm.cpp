#include <cassert>
#include <functional>
#include <iostream>
#include <type_traits>
#include <unordered_map>

#include "vm.h"

const int STACK_SIZE = 1024;

enum offset {
    /* offset to return address */          return_address,
    /* base pointer of enclosing frame */   enclosing_frame,
    /* base pointer of declaration frame */ declaration_frame,
    /* offset to local variables */         local
};

void pl0::execute(const bytecode & code) {
    int pc = 0;
    int *stack = new int[STACK_SIZE];
    int bp = 0;
    int sp = bp;

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
        const instruction &ins = code[pc];
        pc++;

        /* get bp of the frame whose distance from current frame is given */
        auto resolve = [&]() -> int {
            int fp = bp, dist = ins.level;
            while (dist > 0) {
                fp = stack[fp + offset::declaration_frame];
                dist--;
            }
            return fp;
        };

        switch (ins.op) {
        case opcode::LIT:
            stack[++sp] = ins.address;
            break;
        case opcode::LOD:
            stack[++sp] = stack[resolve() + offset::local + ins.address];
            break;
        case opcode::STO:
            stack[resolve() + offset::local + ins.address] = stack[sp--];
            break;
        case opcode::CAL:
            // save context
            stack[sp + offset::return_address] = pc;
            stack[sp + offset::enclosing_frame] = bp;
            stack[sp + offset::declaration_frame] = resolve();
            bp = sp;
            pc = ins.address;
            break;
        case opcode::INT:
            sp += ins.address;
            break;
        case opcode::JMP:
            pc = ins.address;
            break;
        case opcode::JPC:
            if (!stack[sp--]) pc = ins.address;
            break;
        case opcode::OPR:
            if (ins.address == *opt::ODD) {
                int result = stack[sp--] % 2;
                stack[++sp] = result;
            } else if (ins.address == *opt::READ) {
                int tmp;
                std::cin >> tmp;
                stack[++sp] = tmp;
            } else if (ins.address == *opt::WRITE) {
                std::cout << stack[sp--] << '\n';
            } else if (ins.address == *opt::RET) {
                // restore context
                pc = stack[bp];
                sp = bp;
                bp = stack[bp + 1];
            } else {
                int rhs = stack[sp--], lhs = stack[sp--];
                auto op = bifunctors.find(opt(ins.address))->second;
                stack[++sp] = op(lhs, rhs);
            }
            break;
        }
    }
    delete [] stack;
}
