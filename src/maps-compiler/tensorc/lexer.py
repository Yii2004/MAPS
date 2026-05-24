from .tokens import CompileError, Token


KEYWORDS = {
    "tensor": "TENSOR",
    "int32": "INT32",
    "gemm": "GEMM",
    "print": "PRINT",
    "return": "RETURN",
    "if": "IF",
    "else": "ELSE",
    "while": "WHILE",
    "for": "FOR",
    "fn": "FN",
}

SYMBOLS = {
    "<": "LT",
    ">": "GT",
    ",": "COMMA",
    ";": "SEMI",
    "=": "EQUAL",
    "[": "LBRACKET",
    "]": "RBRACKET",
    "(": "LPAREN",
    ")": "RPAREN",
    "{": "LBRACE",
    "}": "RBRACE",
    "+": "PLUS",
    "-": "MINUS",
    "*": "STAR",
    "/": "SLASH",
    "%": "PERCENT",
    "!": "BANG",
}

TWO_CHAR = {
    "->": "ARROW",
    "==": "EQEQ",
    "!=": "NEQ",
    "<=": "LTE",
    ">=": "GTE",
}


class Lexer:
    def __init__(self, source):
        self.source = source
        self.index = 0
        self.line = 1
        self.column = 1

    def tokenize(self):
        tokens = []
        while not self._eof():
            ch = self._peek()
            if ch in " \t\r\n":
                self._consume_ws()
                continue
            if ch == "/" and self._peek(1) == "/":
                self._consume_line_comment()
                continue
            if ch.isalpha() or ch == "_":
                tokens.append(self._identifier())
                continue
            if ch.isdigit():
                tokens.append(self._number())
                continue
            if ch == '"':
                tokens.append(self._string())
                continue
            two = ch + self._peek(1)
            if two in TWO_CHAR:
                tokens.append(self._two_char())
                continue
            if ch in SYMBOLS:
                tokens.append(self._symbol())
                continue
            raise CompileError("unexpected character {!r} at {}:{}".format(ch, self.line, self.column))
        tokens.append(Token("EOF", "", self.line, self.column))
        return tokens

    def _eof(self):
        return self.index >= len(self.source)

    def _peek(self, offset=0):
        pos = self.index + offset
        return "\0" if pos >= len(self.source) else self.source[pos]

    def _advance(self):
        ch = self.source[self.index]
        self.index += 1
        if ch == "\n":
            self.line += 1
            self.column = 1
        else:
            self.column += 1
        return ch

    def _consume_ws(self):
        while not self._eof() and self._peek() in " \t\r\n":
            self._advance()

    def _consume_line_comment(self):
        while not self._eof() and self._peek() != "\n":
            self._advance()

    def _identifier(self):
        line, column = self.line, self.column
        text = []
        while not self._eof() and (self._peek().isalnum() or self._peek() == "_"):
            text.append(self._advance())
        value = "".join(text)
        return Token(KEYWORDS.get(value, "IDENT"), value, line, column)

    def _number(self):
        line, column = self.line, self.column
        text = []
        while not self._eof() and self._peek().isdigit():
            text.append(self._advance())
        return Token("NUMBER", "".join(text), line, column)

    def _string(self):
        line, column = self.line, self.column
        self._advance()
        value = []
        while not self._eof() and self._peek() != '"':
            ch = self._advance()
            if ch == "\\":
                esc = self._advance()
                if esc == "n":
                    value.append("\n")
                elif esc == "t":
                    value.append("\t")
                elif esc == "\\":
                    value.append("\\")
                elif esc == '"':
                    value.append('"')
                else:
                    raise CompileError("invalid escape \\{} at {}:{}".format(esc, line, column))
            else:
                value.append(ch)
        if self._eof():
            raise CompileError("unterminated string at {}:{}".format(line, column))
        self._advance()
        return Token("STRING", "".join(value), line, column)

    def _symbol(self):
        line, column = self.line, self.column
        ch = self._advance()
        return Token(SYMBOLS[ch], ch, line, column)

    def _two_char(self):
        line, column = self.line, self.column
        text = self._advance() + self._advance()
        return Token(TWO_CHAR[text], text, line, column)
