#include "assembler.h"

#define IGNORE 0

namespace pl0 {

void assembler::emit(opcode op, int level, int address) {
    code_.push_back({ op, level, address });
}

int assembler::get_next_address() {
    return static_cast<int>(code_.size());
}

int assembler::get_last_address() {
    return static_cast<int>(code_.size() - 1);
}

void assembler::load(int value) {
    emit(opcode::LIT, IGNORE, value);
}

void assembler::load(int distance, int index) {
    emit(opcode::LOD, distance, index);
}

void assembler::store(int distance, int index) {
    emit(opcode::STO, distance, index);
}

void assembler::call(int distance, int entry) {
    emit(opcode::CAL, distance, entry);
}

backpatcher assembler::call(int caller_level) {
    emit(opcode::CAL, caller_level, IGNORE);
    return backpatcher { code_, get_last_address() };
}

void assembler::branch(int target) {
    emit(opcode::JMP, IGNORE, target);
}

backpatcher assembler::branch() {
    emit(opcode::JMP, IGNORE, IGNORE);
    return backpatcher { code_, get_last_address() };
}

void assembler::branch_if_false(int target) {
    emit(opcode::JPC, IGNORE, target);
}

backpatcher assembler::branch_if_false() {
    emit(opcode::JPC, IGNORE, IGNORE);
    return backpatcher { code_, get_last_address() };
}

void assembler::enter(int scope_var_count) {
    emit(opcode::INT, IGNORE, scope_var_count);
}

void assembler::leave() {
    emit(opcode::OPR, IGNORE, *opt::RET);
}

void assembler::read() {
    emit(opcode::OPR, IGNORE, *opt::READ);
}

void assembler::write() {
    emit(opcode::OPR, IGNORE, *opt::WRITE);
}

void assembler::operation(token tk) {
    auto iter = token2opt.find(tk);
    if (iter == token2opt.end()) {
        throw general_error("token ", *tk, " cannot be used as operator");
    }
    emit(opcode::OPR, IGNORE, *iter->second);
}

bytecode assembler::get_bytecode() {
    return code_;
}

}
