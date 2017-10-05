from codegen import Opt, Opcode
from lexer import Lexer
from parse import Parser

BIFUNCTORS = {
    Opt.ADD: (lambda x, y: x + y),
    Opt.SUB: (lambda x, y: x - y),
    Opt.MUL: (lambda x, y: x * y),
    Opt.DIV: (lambda x, y: x // y),
    Opt.EQ: (lambda x, y: 1 if x == y else 0),
    Opt.NEQ: (lambda x, y: 1 if x != y else 0),
    Opt.LE: (lambda x, y: 1 if x < y else 0),
    Opt.LEQ: (lambda x, y: 1 if x <= y else 0),
    Opt.GE: (lambda x, y: 1 if x > y else 0),
    Opt.GEQ: (lambda x, y: 1 if x >= y else 0)
}

def fill(n):
    result = list()
    for i in range(n):
        result.append(0)
    return result

class StackFrame:
    def __init__(self, return_address, enclosing_frame, parent_frame=None):
        self.parent_frame = parent_frame
        self.enclosing_frame = enclosing_frame
        self.return_address = return_address
        self.locals = None
        self.eval_stack = list()
    def allocate_locals(self, local_var_count):
        self.locals = fill(local_var_count)

class VM:
    def __init__(self):
        pass
    def execute(self, code, entry=0):
        top = StackFrame(len(code), None)
        program_counter = entry
        # callbacks for each opcode
        def get_target_frame(level):
            nonlocal top
            target_frame = top
            while level > 0:
                level -= 1
                target_frame = target_frame.parent_frame
            return target_frame
        def on_lit(_, value):
            nonlocal top
            top.eval_stack.append(value)
        def on_lod(level, index):
            nonlocal top
            target_frame = get_target_frame(level)
            value = target_frame.locals[index]
            top.eval_stack.append(value)
        def on_sto(level, index):
            nonlocal top
            target_frame = get_target_frame(level)
            assert len(top.eval_stack) > 0
            value = top.eval_stack.pop()
            target_frame.locals[index] = value
        def on_cal(level, entry):
            nonlocal program_counter
            nonlocal top
            # print('Jump to procedure at %d' % entry)
            frame = top
            while level > 0 and frame is not None:
                level -= 1
                frame = frame.parent_frame
            top = StackFrame(program_counter, top, frame)
            program_counter = entry
        def on_int(_, size):
            nonlocal top
            assert size >= 3;
            top.allocate_locals(size - 3)
        def on_jmp(_, target):
            nonlocal program_counter
            # print('Unconditionally jump to %d' % target)
            assert target < len(code);
            program_counter = target
        def on_jpc(_, target):
            nonlocal program_counter
            nonlocal top
            assert len(top.eval_stack) > 0
            value = top.eval_stack.pop()
            if value == 0:
                # print('Conditionally jump to %d' % target)
                program_counter = target
        def on_opr(_, op):
            nonlocal program_counter
            nonlocal top
            if op in BIFUNCTORS:
                assert len(top.eval_stack) >= 2
                rhs = top.eval_stack.pop()
                lhs = top.eval_stack.pop()
                top.eval_stack.append(BIFUNCTORS[op](lhs, rhs))
            elif op == Opt.READ:
                value = int(input('--> '))
                top.eval_stack.append(value)
            elif op == Opt.WRITE:
                assert len(top.eval_stack) > 0
                arg = top.eval_stack.pop()
                print(arg)
            elif op == Opt.RET:
                program_counter = top.return_address
                top = top.enclosing_frame
            else:
                raise Exception('invalid operator')            
        # dict for callbacks
        cbs = {
            Opcode.LIT: on_lit,
            Opcode.LOD: on_lod,
            Opcode.STO: on_sto,
            Opcode.CAL: on_cal,
            Opcode.INT: on_int,
            Opcode.JMP: on_jmp,
            Opcode.JPC: on_jpc,
            Opcode.OPR: on_opr
        }
        # do iteration on instructions
        while program_counter < len(code):
            print('Execute instruction at %d' % program_counter)
            (opcode, level, address, _) = code[program_counter]
            program_counter += 1
            cbs[opcode](level, address)
