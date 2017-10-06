from codegen import Opt, Opcode, prettify_instruction
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

class StackFrame:
    def __init__(self, return_address, enclosing_frame, parent_frame=None):
        self.parent_frame = parent_frame
        self.enclosing_frame = enclosing_frame
        self.return_address = return_address
        self.locals = None
        self.stack = list()

    def allocate_locals(self, local_var_count):
        self.locals = [0 for i in range(local_var_count)]
        
    def resolve_frame(self, dist):
        frame = self
        while dist > 0:
            dist -= 1
            frame = frame.parent_frame
        return frame

class VM:
    def __init__(self, code, entry=0):
        self.code = code
        self.top = StackFrame(len(code), None)
        self.program_counter = entry
        self.handlers = {
            Opcode.LIT: self.on_lit,
            Opcode.LOD: self.on_lod,
            Opcode.STO: self.on_sto,
            Opcode.CAL: self.on_cal,
            Opcode.INT: self.on_int,
            Opcode.JMP: self.on_jmp,
            Opcode.JPC: self.on_jpc,
            Opcode.OPR: self.on_opr
        }

    def on_lit(self, _, value):
        self.top.stack.append(value)

    def on_lod(self, level, index):
        target_frame = self.top.resolve_frame(level)
        self.top.stack.append(target_frame.locals[index])

    def on_sto(self, level, index):
        target_frame = self.top.resolve_frame(level)
        assert len(self.top.stack) > 0
        target_frame.locals[index] = self.top.stack.pop()

    def on_cal(self, level, entry):
        frame = self.top.resolve_frame(level)
        self.top = StackFrame(self.program_counter, self.top, frame)
        self.program_counter = entry

    def on_int(self, _, size):
        assert size >= 3;
        self.top.allocate_locals(size - 3)

    def on_jmp(self, _, target):
        assert target < len(self.code);
        self.program_counter = target

    def on_jpc(self, _, target):
        assert len(self.top.stack) > 0
        if self.top.stack.pop() == 0:
            self.program_counter = target

    def on_opr(self, _, op):
        if op in BIFUNCTORS:
            assert len(self.top.stack) >= 2
            rhs = self.top.stack.pop()
            lhs = self.top.stack.pop()
            res = BIFUNCTORS[op](lhs, rhs)
            self.top.stack.append(res)
        elif op == Opt.READ:
            value = int(input('--> '))
            self.top.stack.append(value)
        elif op == Opt.WRITE:
            assert len(self.top.stack) > 0
            arg = self.top.stack.pop()
            print(arg)
        elif op == Opt.RET:
            self.program_counter = self.top.return_address
            self.top = self.top.enclosing_frame
        else:
            raise Exception('invalid operator %s' % op)

    def execute(self, show_trace=False):
        while self.program_counter < len(self.code):
            ins = self.code[self.program_counter]
            (opcode, level, address, comment) = ins
            if show_trace:
                print('%d\t' % self.program_counter, prettify_instruction(ins))
            self.program_counter += 1
            self.handlers[opcode](level, address)
