#ifndef PL_ZERO_BYTECODE_H
#define PL_ZERO_BYTECODE_H

#include <tuple>
#include <vector>

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

typedef std::tuple<opcode, int, int> instruction;

typedef std::vector<instruction> bytecode;

}

#endif
