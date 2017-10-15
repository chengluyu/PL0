#ifndef PL_ZERO_BYTECODE_H
#define PL_ZERO_BYTECODE_H

#include <tuple>
#include <vector>

#include "token.h"

namespace pl0 {

enum class opcode : int {
	LIT, LOD, STO, CAL, INT, JMP, JPC, OPR
};

enum class opt : int {
	RET = 0,
	SUB, ADD, DIV, MUL,
	LE, LEQ, GE, GEQ, EQ, NEQ, ODD,
	WRITE = 14, READ = 16
};

inline constexpr int operator* (opt x) {
	return static_cast<int>(x);
}

#define OPERATOR(name, string) { token_type::name, opt::name },
const std::unordered_map<token_type, opt> token2opt = {
	TOKEN_LIST(IGNORE_TOKEN, OPERATOR, IGNORE_TOKEN)
};
#undef OPERATOR

typedef std::tuple<opcode, int, int> instruction;

typedef std::vector<instruction> bytecode;

}

#endif
