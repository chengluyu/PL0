from enum import Enum, IntEnum, unique

@unique
class Opcode(Enum):
    LIT = 0
    LOD = 1
    STO = 2
    CAL = 3
    INT = 4
    JMP = 5
    JPC = 6
    OPR = 7

@unique
class Opt(IntEnum):
    RET = 0
    SUB = 1
    ADD = 2
    DIV = 3
    MUL = 4
    LE = 5
    LEQ = 6
    GE = 7 
    GEQ = 8
    EQ = 9
    NEQ = 10
    ODD = 11 # emmm, I assume that ODD is 11
    WRITE = 14
    READ = 16
        
OPS = {
    '+': Opt.ADD, '-': Opt.SUB, '*': Opt.MUL, '/': Opt.DIV,
    '=': Opt.EQ, '#': Opt.NEQ, '<': Opt.LE, '<=': Opt.LEQ,
    '>': Opt.GE, '>=': Opt.GEQ, 'odd': Opt.ODD
}

class Assembler:
    def __init__(self):
        self.code = list()
        self.comment_for_next = None
    def comment(self, text):
        self.comment_for_next = text
    def curr_addr(self):
        return len(self.code) - 1
    def next_addr(self):
        return len(self.code)
    def refill_addr(self, index, address):
        (opcode, level, _, comment) = self.code[index]
        self.code[index] = (opcode, level, address, comment)
    def emit(self, opcode, level, address):
        ins = (opcode, level, address, self.comment_for_next)
        self.comment_for_next = None
        self.code.append(ins)
    # LIT: load constant to stack top
    def load_const(self, value):
        if not isinstance(value, int):
            raise ValueError('the operand of LIT must be a interger')
        self.emit(Opcode.LIT, 0, value)
    # LOD: load variable
    def load_var(self, level, address):
        self.emit(Opcode.LOD, level, address)
    # STO: store to variable
    def store_var(self, level, address):
        self.emit(Opcode.STO, level, address)
    # CAL: call a procedure
    def call(self, level, address):
        self.emit(Opcode.CAL, level, address)
        if address is None:
            return self.curr_addr()
    # JMP: unconditionally jump
    def jump(self, target=None):
        self.emit(Opcode.JMP, 0, target)
        if target is None:
            return self.curr_addr()
    # JPC: conditionally jump
    def jump_if_false(self, target=None):
        self.emit(Opcode.JPC, 0, target)
        if target is None:
            return self.curr_addr()
    # INT: init vars
    def enter(self, var_count):
        self.emit(Opcode.INT, 0, var_count + 3)
    # helpers for OPR
    def operator(self, op):
        if op in OPS:
            self.emit(Opcode.OPR, 0, OPS[op])
        else:
            raise ValueError('invalid operator to assemble')
    def read(self):
        self.emit(Opcode.OPR, 0, Opt.READ)
    def write(self):
        self.emit(Opcode.OPR, 0, Opt.WRITE)
    def leave(self):
        self.emit(Opcode.OPR, 0, Opt.RET)