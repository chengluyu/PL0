from scope import *
from lexer import Lexer
from codegen import Opcode, Opt, Assembler

"""
program = block "." .

block = [ "const" ident "=" number {"," ident "=" number} ";"]
        [ "var" ident {"," ident} ";"]
        { "procedure" ident ";" block ";" } statement .

statement = [ ident ":=" expression | "call" ident 
              | "?" ident | "!" expression 
              | "begin" statement {";" statement } "end" 
              | "if" condition "then" statement 
              | "while" condition "do" statement ].

condition = "odd" expression |
            expression ("="|"#"|"<"|"<="|">"|">=") expression .

expression = [ "+"|"-"] term { ("+"|"-") term}.

term = factor {("*"|"/") factor}.

factor = ident | number | "(" expression ")".
"""
COMPARATOR = set(['=', '#', '<', '>', '<=', '>='])

class Parser:
    def __init__(self, lexer):
        self.lexer = lexer
        self.scope = None
        self.asm = Assembler()
        self.call_refills = dict()
    def program(self):
        # this is the global scope
        self.scope = Scope(self.scope)
        self.subprogram('(main)', True)
        self.lexer.expect('.')
        self.lexer.expect('eos')
        # retreat the global scope
        self.scope = self.scope.enclosing_scope
        return self.asm.code
    # the subprogram returns the entry address of this subprogram
    def subprogram(self, subprog_name, is_main=False):
        # main only
        if is_main:
            self.asm.comment('jump to main')
            jump_main_entry = self.asm.jump()
        while self.lexer.peep('const'):
            self.decl_consts()
        while self.lexer.peep('var'):
            self.decl_vars()
        while self.lexer.peep('procedure'):
            self.decl_proc()
        # main only
        if is_main:
            main_entry = self.asm.next_addr()
            self.asm.refill_addr(jump_main_entry, main_entry)
        entry_addr = self.asm.next_addr()
        self.asm.comment(subprog_name)
        self.asm.enter(self.scope.var_count)
        self.stmt()
        self.asm.leave()
        return entry_addr
    def decl_single_var(self, index):
        (_, ident) = self.lexer.expect('identifier')
        self.scope.define(VariableSymbol(ident, self.scope.level, index))
    def decl_vars(self):
        self.lexer.expect('var')
        self.decl_single_var(self.scope.var_count)
        while self.lexer.match(','):
            self.decl_single_var(self.scope.var_count)
        self.lexer.expect(';')
    def decl_single_const(self):
        (_, ident) = self.lexer.expect('identifier')
        self.lexer.expect('=')
        (_, value) = self.lexer.expect('number')
        return ConstantSymbol(ident, value)
    def decl_consts(self):
        self.lexer.expect('const')
        self.scope.define(self.decl_single_const())
        while self.lexer.match(','):
            self.scope.define(self.decl_single_const())
        self.lexer.expect(';')
    def decl_proc(self):
        self.lexer.expect('procedure')
        (_, proc_name) = self.lexer.expect('identifier')
        proc_symb = ProcedureSymbol(proc_name, self.scope.level)
        self.scope.define(proc_symb)
        self.call_refills[proc_name] = []
        self.lexer.expect(';')
        # create new scope for this procedure
        self.scope = Scope(self.scope)
        entry_addr = self.subprogram(proc_name)
        self.lexer.expect(';')
        proc_symb.entry = entry_addr
        for addr in self.call_refills[proc_name]:
            self.asm.refill_addr(addr, entry_addr)
        del self.call_refills[proc_name]
        # destory the scope
        self.scope = self.scope.enclosing_scope
    def stmt_block(self):
        self.lexer.expect('begin')
        self.stmt()
        while self.lexer.match(';'):
            self.stmt()
        self.lexer.expect('end')
    def if_stmt(self):
        self.lexer.expect('if')
        self.cond()
        self.lexer.expect('then')
        jmp_altr = self.asm.jump_if_false()
        self.stmt()
        if self.lexer.match('else'):
            cons_end = self.asm.jump()
            altr_begin = self.asm.next_addr()
            self.stmt()
            altr_end = self.asm.next_addr()
            self.asm.refill_addr(jmp_altr, altr_begin)
            self.asm.refill_addr(cons_end, altr_end)
        else:
            cons_end = self.asm.next_addr()
            self.asm.refill_addr(jmp_altr, cons_end)
    def while_stmt(self):
        loop_begin = self.asm.next_addr()
        self.lexer.expect('while')
        self.cond()
        self.lexer.expect('do')
        cond_fail = self.asm.jump_if_false()
        self.stmt()
        self.asm.jump(loop_begin)
        loop_end = self.asm.next_addr()
        self.asm.refill_addr(cond_fail, loop_end)
    def call_stmt(self):
        self.lexer.expect('call')
        (_, ident) = self.lexer.expect('identifier')
        symb = self.scope.resolve(ident)
        if symb is None:
            raise Exception('undeclared procedure "%s"' % ident)
        if symb.kind != SymbolKind.PROC:
            raise Exception('only procedure can be called')
        refill = self.asm.call(self.scope.level - symb.level, symb.entry)
        if refill is not None: # so we need to fill the address field later
            self.call_refills[ident].append(refill)
    def stmt(self):
        if self.lexer.peep('read'):
            self.read_stmt()
        elif self.lexer.peep('write'):
            self.write_stmt()
        elif self.lexer.peep('if'):
            self.if_stmt()
        elif self.lexer.peep('begin'):
            self.stmt_block()
        elif self.lexer.peep('while'):
            self.while_stmt()
        elif self.lexer.peep('call'):
            self.call_stmt()
        else:
            self.assign_stmt()
    def read_stmt(self):
        self.lexer.expect('read')
        #self.lexer.expect('(')
        self.expr()
        self.asm.read()
        while self.lexer.peep(','):
            self.expr()
            self.asm.read()
        #self.lexer.expect(')')
    def write_stmt(self):
        self.lexer.expect('write')
        #self.lexer.expect('(')
        self.expr()
        self.asm.write()
        while self.lexer.peep(','):
            self.expr()
            self.asm.write()
        #self.lexer.expect(')')
    def assign_stmt(self):
        (_, ident) = self.lexer.expect('identifier')
        symb = self.scope.resolve(ident)
        if symb is None:
            raise Exception('undeclared identifier')
        if symb.kind != SymbolKind.VAR:
            raise Exception('only variables can be assigned')
        self.lexer.expect(':=')
        self.expr()
        self.asm.store_var(self.scope.level - symb.level, symb.index)
    def cond(self):
        if self.lexer.match('odd'):
            self.expr()
            self.asm.operator('odd')
        else:
            self.expr()
            (op, _) = self.lexer.next()
            if op not in COMPARATOR:
                raise Exception('expect a comparator instead of %s' % op)
            self.expr()
            self.asm.operator(op)
    def expr(self):
        self.term()
        while self.lexer.peep('*') or self.lexer.peep('/'):
            (op, _) = self.lexer.next()
            self.term()
            self.asm.operator(op)
    def term(self):
        self.factor()
        while self.lexer.peep('+') or self.lexer.peep('-'):
            (op, _) = self.lexer.next()
            self.factor()
            self.asm.operator(op)
    def factor(self):
        if self.lexer.peep('identifier'):
            (_, ident) = self.lexer.next()
            symb = self.scope.resolve(ident)
            if symb is None:
                raise Exception('undeclared identifier')
            if symb.kind == SymbolKind.VAR:
                self.asm.load_var(self.scope.level - symb.level, symb.index)
            elif symb.kind == SymbolKind.CONST:
                self.asm.load_const(symb.value)
            else:
                raise Exception('procedures cannot be referenced in expression')
        elif self.lexer.peep('number'):
            (_, num) = self.lexer.next()
            self.asm.load_const(num)
        elif self.lexer.match('('):
            self.expr()
            self.lexer.expect(')')
        else:
            raise Exception('expect an identifier, an integer or an expression instead of %s' % self.lexer.peek[0])
                