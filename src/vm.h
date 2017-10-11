#ifndef PL_ZERO_VM_H
#define PL_ZERO_VM_H

#include "bytecode.h"

namespace pl0 {

void execute(const bytecode &code, int entry_addr = 0);

}

#endif
