from enum import Enum, unique


@unique
class SymbolKind(Enum):
    VAR = 0
    CONST = 1
    PROC = 2


class Symbol:
    def __init__(self, name, kind):
        self.name = name
        self.kind = kind
        self.scope = None


class VariableSymbol (Symbol):
    def __init__(self, name, level, index):
        super().__init__(name, SymbolKind.VAR)
        self.level = level
        self.index = index


class ConstantSymbol (Symbol):
    def __init__(self, name, value):
        super().__init__(name, SymbolKind.CONST)
        self.value = value


class ProcedureSymbol (Symbol):
    def __init__(self, name, level, entry=None):
        super().__init__(name, SymbolKind.PROC)
        self.level = level
        self.entry = entry


class Scope:
    def __init__(self, enclosing_scope):
        self.enclosing_scope = enclosing_scope
        self.level = 0
        if enclosing_scope is not None:
            self.level = enclosing_scope.level + 1
        self.var_count = 0
        self.members = dict()

    def define(self, sym):
        if sym.name in self.members:
            raise Exception('duplicated symbol: %s' % sym.name)
        if sym.kind == SymbolKind.VAR:
            self.var_count += 1
        sym.scope = self
        self.members[sym.name] = sym

    def resolve(self, name):
        if name in self.members:
            return self.members[name]
        elif self.enclosing_scope is None:
            return None
        else:
            return self.enclosing_scope.resolve(name)
