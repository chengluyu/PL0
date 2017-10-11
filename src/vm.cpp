#include <cassert>
#include <functional>
#include <iostream>
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
	int *bp = stack;
	int *sp = bp + offset::declaration_frame;

	auto resolve = [&](int dist) -> int* {
		int *fp = bp;
		while (dist > 0) {
			fp = reinterpret_cast<int*>(fp[offset::declaration_frame]);
			dist--;
		}
		return fp;
	};
	
	auto push = [&](auto value) {
		*++sp = reinterpret_cast<int>(value);
	};

	auto pop = [&]() -> int {
		return *sp--;
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
		{ opt::NEQ, std::not_equal_to<int>() },
	};

	while (pc < code.size()) {
		const instruction & ins = code[pc];
		pc++;
		switch (std::get<0>(ins)) {
		case opcode::LIT:
			push(std::get<2>(ins));
			break;
		case opcode::LOD:
			push(resolve(std::get<1>(ins))[offset::local + std::get<2>(ins)]);
			break;
		case opcode::STO:
			resolve(std::get<1>(ins))[offset::local + std::get<2>(ins)] = pop();
			break;
		case opcode::CAL:
			push(pc);
			push(bp);
			push(resolve(std::get<1>(ins)));
			bp = sp - 3;
			pc = std::get<2>(ins);
			break;
		case opcode::INT:
			sp += std::get<2>(ins);
			break;
		case opcode::JMP:
			pc = std::get<2>(ins);
			break;
		case opcode::JPC:
			if (pop()) pc = std::get<2>(ins);
			break;
		case opcode::OPR:
			if (std::get<2>(ins) == *opt::ODD) {
				push(pop() % 2);
			} else if (std::get<2>(ins) == *opt::READ) {
				int tmp;
				std::cin >> tmp;
				push(tmp);
			} else if (std::get<2>(ins) == *opt::WRITE) {
				std::cout << pop() << '\n';
			} else if (std::get<2>(ins) == *opt::RET) {
				sp = bp + offset::enclosing_frame;
				bp = reinterpret_cast<int*>(pop());
				pc = pop();
			} else {
				int rhs = pop(), lhs = pop();
				bifunctors.find(opt(std::get<2>(ins)))->second(lhs, rhs);
			}
			break;
		}
	}
}
