#ifndef PL_ZERO_ASSEMBLER_H
#define PL_ZERO_ASSEMBLER_H

#include <vector>

#include "bytecode.h"
#include "util.h"

namespace pl0 {

class assembler {
    bytecode code_;

    void emit(opcode op, int level, int address);
public:
    int  get_next_address();
    int  get_last_address();
    void load(int value);
    void load(int distance, int index);
    void store(int distance, int index);
    void        call(int distance, int entry);
    backpatcher call();
    void        branch(int target);
    backpatcher branch();
    void        branch_if_false(int target);
    backpatcher branch_if_false();
    void enter(int scope_var_count);
    void leave();
    void read();
    void write();
    void operation(token tk);

    bytecode get_bytecode();
};

}

#endif
