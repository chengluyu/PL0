from enum import Enum, unique
import re

KEYWORDS = set(['const', 'var', 'procedure', 'call', 'if', 'then', 'else',
                'while', 'do', 'begin', 'end', 'odd', 'read', 'write'])
RE_SYMBOL = re.compile(r">=|<=|:=|[+\-*/=,.<>#();]", re.MULTILINE)
RE_NUMBER = re.compile(r"\d+")
RE_IDENT = re.compile(r"[A-Za-z_][0-9A-Za-z_]*", re.MULTILINE)
RE_WHITESPACE = re.compile(r"\s+")


class Span:
    def __init__(self, line, start, end):
        self.line = line
        self.start = start + 1
        self.end = end + 1
    
    def __str__(self):
        return 'line %d column %d to %d' % (self.line, self.start, self.end)


class Token:
    def __init__(self, span, name, value=None):
        self.span = span
        self.name = name
        self.value = value
    
    def __str__(self):
        if self.name == 'identifier':
            return '%s "%s" [%s]' % (self.name, self.value, self.span)
        elif self.name == 'number':
            return '%s %d [%s]' % (self.name, self.value, self.span)
        else:
            return '%s [%s]' % (self.name, self.span)

class Lexer:
    def __init__(self, filepath):
        self.fd = open(filepath, 'r')
        self.current_line = self.fd.readline()
        self.line = 1
        self.at = 0
        self.peek = None
        self.advance()

    def advance(self):
        # skip whitespaces
        result = RE_WHITESPACE.match(self.current_line, self.at)
        if result is not None:
            self.at = result.end()
        # check if current line is ended
        while len(self.current_line) == self.at or self.current_line[
                self.at] == '\n':
            newline = self.fd.readline()
            # test if eof
            if not newline:
                self.peek = Token(Span(self.line, self.at, self.at), 'eos')
                return
            self.line += 1
            self.at = 0
            self.current_line = newline
            # skip whitespaces after advance next line
            result = RE_WHITESPACE.match(self.current_line, self.at)
            if result is not None:
                self.at = result.end()
        # match symbols
        result = RE_SYMBOL.match(self.current_line, self.at)
        if result is not None:
            span = Span(self.line, self.at, result.end())
            self.at = result.end()
            self.peek = Token(span, result.group())
            return
        # match identifier
        result = RE_IDENT.match(self.current_line, self.at)
        if result is not None:
            span = Span(self.line, self.at, result.end())
            self.at = result.end()
            ident = result.group()
            if ident in KEYWORDS:
                self.peek = Token(span, ident)
            else:
                self.peek = Token(span, 'identifier', ident)
            return
        # match number
        result = RE_NUMBER.match(self.current_line, self.at)
        if result is not None:
            span = Span(self.line, self.at, result.end())
            self.at = result.end()
            self.peek = Token(span, 'number', int(result.group()))
            return
        # illegal token
        raise Exception('unrecognized character at line %d column %d' %
                        (self.line, self.at))

    def peep(self, token):
        return self.peek.name == token

    def next(self):
        result = self.peek
        self.advance()
        return result

    def match(self, possible_name):
        if self.peek.name == possible_name:
            return self.next()

    def expect(self, expected_name):
        if self.peek.name == expected_name:
            return self.next()
        raise Exception('expect %s instead of %s at line %d column %d' %
                        (token, self.peek, self.line, self.at))
