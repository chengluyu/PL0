from enum import Enum, unique
import re

KEYWORDS = set(['const', 'var', 'procedure', 'call', 'if', 'then', 'else',
                'while', 'do', 'begin', 'end', 'odd', 'read', 'write'])
RE_SYMBOL = re.compile(r"[+\-*/=,.<>#();]|>=|<=|:=", re.MULTILINE)
RE_NUMBER = re.compile(r"\d+")
RE_IDENT = re.compile(r"[A-Za-z_][0-9A-Za-z_]*", re.MULTILINE)
RE_WHITESPACE = re.compile(r"\s+")

class Lexer:
    def __init__(self, filepath):
        self.fd = open(filepath, 'r')
        self.current_line = self.fd.readline()
        self.line = 1
        self.column = 0
        self.peek = None
        self.advance()
    def advance(self):
        # skip whitespaces
        result = RE_WHITESPACE.match(self.current_line, self.column)
        if result is not None:
            self.column = result.end()
        # check if current line is ended
        while len(self.current_line) == self.column or self.current_line[
                self.column] == '\n':
            newline = self.fd.readline()
            # test if eof
            if not newline:
                self.peek = ('eos', None)
                return
            self.line += 1
            self.column = 0
            self.current_line = newline
            # skip whitespaces after advance next line
            result = RE_WHITESPACE.match(self.current_line, self.column)
            if result is not None:
                self.column = result.end()
        # match symbols
        result = RE_SYMBOL.match(self.current_line, self.column)
        if result is not None:
            self.column = result.end()
            self.peek = (result.group(), None)
            return
        # match identifier
        result = RE_IDENT.match(self.current_line, self.column)
        if result is not None:
            self.column = result.end()
            ident = result.group()
            if ident in KEYWORDS:
                self.peek = (ident, None)
            else:
                self.peek = ('identifier', ident)
            return
        # match number
        result = RE_NUMBER.match(self.current_line, self.column)
        if result is not None:
            self.column = result.end()
            self.peek = ('number', int(result.group()))
            return
        # illegal token
        self.peek = ('illegal', (self.line, self.column))
    def peep(self, token):
        return self.peek[0] == token
    def next(self):
        result = self.peek
        self.advance()
        return result
    def match(self, token):
        if self.peek[0] == token:
            result = self.peek
            self.advance()
            return result
    def expect(self, token):
        result = self.match(token)
        if not result:
            raise Exception('expect %s instead of %s at line %d column %d' %
                            (token, self.peek, self.line, self.column))
        return result

if __name__ == '__main__':
    lexer = Lexer('./example/prime.txt')
    while lexer.peek[0] != 'eos' and lexer.peek[0] != 'illegal':
        print(lexer.peek)
        lexer.advance()
    print(lexer.peek)
    