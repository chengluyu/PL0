#ifndef PL_ZERO_ASSEMBLER_H
#define PL_ZERO_ASSEMBLER_H

#include <vector>

#include "bytecode.h"
#include "util.h"
#define IGNORE 0

namespace pl0 {

class backpatcher {
	std::vector<instruction> &code_;
	std::size_t at_;

	backpatcher(std::vector<instruction> &code, std::size_t at) : code_(code), at_(at) {}
	friend class assembler;
public:
	void set_level(int level) {
		opcode op;
		int address;
		std::tie(op, std::ignore, address) = code_[at_];
		code_[at_] = std::make_tuple(op, level, address);
	}

	void set_address(int address) {
		opcode op;
		int level;
		std::tie(op, level, std::ignore) = code_[at_];
		code_[at_] = std::make_tuple(op, level, address);
	}
};

class assembler {
	std::vector<instruction> code_;
	void emit(opcode op, int level, int address) {
		code_.push_back({ op, level, address });
	}
public:
	void load(int value) {
		emit(opcode::LIT, IGNORE, value);
	}

	void load(int distance, int index) {
		emit(opcode::LOD, distance, index);
	}

	void store(int distance, int index) {
		emit(opcode::STO, distance, index);
	}

	void call(int distance, int entry) {
		emit(opcode::CAL, distance, entry);
	}

	backpatcher call() {
		emit(opcode::CAL, IGNORE, IGNORE);
		return backpatcher{ code_, code_.size() - 1 };
	}

	void branch(int target) {
		emit(opcode::JMP, IGNORE, target);
	}

	backpatcher branch() {
		emit(opcode::JMP, IGNORE, IGNORE);
		return backpatcher{ code_, code_.size() - 1 };
	}

	void branch_if_false(int target) {
		emit(opcode::JPC, IGNORE, target);
	}

	backpatcher branch_if_false() {
		emit(opcode::JPC, IGNORE, IGNORE);
		return backpatcher{ code_, code_.size() - 1 };
	}

	void enter(int scope_var_count) {
		emit(opcode::INT, IGNORE, scope_var_count);
	}

	void leave() {
		emit(opcode::OPR, IGNORE, *opt::RET);
	}

	void read() {
		emit(opcode::OPR, IGNORE, *opt::READ);
	}

	void write() {
		emit(opcode::OPR, IGNORE, *opt::WRITE);
	}

	void operation(token_type tk) {
		auto iter = token2opt.find(tk);
		if (iter == token2opt.end()) {
			throw general_error("token ", *tk, " cannot be used as operator");
		}
		emit(opcode::OPR, IGNORE, *iter->second);
	}
};

}

#endif
