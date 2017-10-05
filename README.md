<p align="center"><img src="https://raw.githubusercontent.com/chengluyu/PL0/master/pl0.png" width=200></p>

## Introduction

This repository holds my implementation of PL/0 language interpreter. This project is intended for the experiment of my compiler theory course in my university.

You can see the full grammar (written in BNF) of common PL/0 language at [Wikipedia](https://en.wikipedia.org/wiki/PL/0). However, the implementation in this repository is slightly different from the common one. This is mainly because the experiment introduces more requirements at the base of the original version, which is listed below:

1. Procedures can be nested, which indicates that you can define a sub-procedure in a procedure. The inner procedures do have the access to symbols that defined in outer procedures.
2. The parser should not generate abstract syntax trees, instead it should generate byte codes directly. 
3. A virtual machine should be done in order to simulate the execution of byte codes.

## Specs of Target Machine

In this section, the target instruction set will be demonstrated. The target runtime environment is a stack-based machine. There are four register and a stack in the target machine.

The purpose of four registers are listed below:

1. `IP` holds the instruction being executed.
2. `PC` is program counter.
3. `SP` refers the top of the stack.
4. `BP` refers the base of current stack frame.

Stack has two purposes: allocate variables and being used as evaluation stack.

Stack is composed of many consecutive storage unit. In order to gain a deeper understanding, we can imagine that stack is composed of many stack frames and each stack frame is composed of many storage units.

When a procedure is called, a new stack frame will be placed at the top of the stack. Vice versa, the stack frame will be removed when the procedure is returned.

But what is inside the stack frame? At the bottom of the stack frame, there are three variables: 

1. Static link is the `BP` register of the procedure in which this procedure is called.
2. Dynamic link is the `BP` register of the procedure in which this procedure is declared.
3. Return address refers to the next instruction when this procedure is returned.

Local variables which declared in this procedure are located beyond three variables. Those are called static zone. Beyond the static zone there is dynamic zone, which is served as evaluation stack.

### Instruction Set

The instruction consists of three parts: opcode, level, address. There are seven opcodes, which is listed below.

1. `LIT`: Load a immediate value onto the stack. The level field in instruction is unused. The address field holds the immediate value.
2. `LOD`: Load a variable onto the stack. The level fields indicates the distance from current stack frame to the stack frame where target variable locates. The address field is the index of target variable.
3. `STO`: Store the value on the top of the stack to a variable. Two fields acts the same as `LOD`.
4. `CAL`: Call a procedure. The level fields indicates the distance from current stack frame to the stack frame where the callee procedure is defined. The address field is the address of first instruction in the callee procedure.
5. `INT`: Allocate some variables at the top of evaluation stack. The level field is unused. The address field is the count of local variables.
6. `JMP`: Unconditionally jump to the address given in address field. The level field is unused.
7. `JPC`: If the value at top of evaluation is falsy (i.e. zero), jump to the address given in address field. The level field is unused.
8. `OPR`: Do the operation decided by the address field.

## Current Status

The old C code is deprecated and the new plan is listed below:

- [x] The Python 3 version.
- [ ] Add more exemplary code for test.
- [ ] Transcript the Python 3 code to C++.
- [ ] Add breakpoint support (make debug easier).

## License 

MIT
