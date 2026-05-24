from .ast import (
    AssignStmt,
    BinaryOp,
    Block,
    CallExpr,
    ExprStmt,
    ForStmt,
    FunctionDecl,
    GemmStmt,
    IfStmt,
    IntLiteral,
    Param,
    PrintStmt,
    Program,
    ReturnStmt,
    TensorDecl,
    TensorIndex,
    UnaryOp,
    VarDecl,
    VarRef,
    WhileStmt,
)
from .tokens import CompileError


class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def parse(self):
        declarations = []
        while not self._check("EOF"):
            if self._check("FN"):
                declarations.append(self._function_decl())
            else:
                declarations.append(self._statement())
        return Program(declarations)

    def _function_decl(self):
        self._expect("FN")
        name = self._expect("IDENT").value
        self._expect("LPAREN")
        params = []
        if not self._check("RPAREN"):
            params.append(self._param())
            while self._match("COMMA"):
                params.append(self._param())
        self._expect("RPAREN")
        self._expect("ARROW")
        self._expect("INT32")
        return FunctionDecl(name, params, "int32", self._block())

    def _param(self):
        self._expect("INT32")
        return Param(self._expect("IDENT").value, "int32")

    def _block(self):
        self._expect("LBRACE")
        statements = []
        while not self._check("RBRACE"):
            statements.append(self._statement())
        self._expect("RBRACE")
        return Block(statements)

    def _statement(self):
        if self._check("TENSOR"):
            return self._tensor_decl(expect_semi=True)
        if self._check("INT32"):
            stmt = self._var_decl()
            self._expect("SEMI")
            return stmt
        if self._check("GEMM"):
            return self._gemm()
        if self._check("PRINT"):
            return self._print()
        if self._check("IF"):
            return self._if()
        if self._check("WHILE"):
            return self._while()
        if self._check("FOR"):
            return self._for()
        if self._check("RETURN"):
            return self._return()
        expr = self._expression()
        if self._match("EQUAL"):
            value = self._expression()
            self._expect("SEMI")
            return AssignStmt(expr, value)
        self._expect("SEMI")
        return ExprStmt(expr)

    def _tensor_decl(self, expect_semi):
        self._expect("TENSOR")
        self._expect("LT")
        self._expect("INT32")
        rows = None
        cols = None
        if self._match("COMMA"):
            rows = self._number()
            self._expect("COMMA")
            cols = self._number()
        self._expect("GT")
        name = self._expect("IDENT").value
        init = None
        if self._match("EQUAL"):
            init = self._matrix_literal()
        if expect_semi:
            self._expect("SEMI")
        return TensorDecl(name, rows, cols, init)

    def _var_decl(self):
        self._expect("INT32")
        name = self._expect("IDENT").value
        init = IntLiteral(0)
        if self._match("EQUAL"):
            init = self._expression()
        return VarDecl(name, init)

    def _matrix_literal(self):
        rows = []
        self._expect("LBRACKET")
        rows.append(self._row_literal())
        while self._match("COMMA"):
            rows.append(self._row_literal())
        self._expect("RBRACKET")
        return rows

    def _row_literal(self):
        values = []
        self._expect("LBRACKET")
        values.append(self._number())
        while self._match("COMMA"):
            values.append(self._number())
        self._expect("RBRACKET")
        return values

    def _gemm(self):
        self._expect("GEMM")
        self._expect("LPAREN")
        a = self._expect("IDENT").value
        self._expect("COMMA")
        b = self._expect("IDENT").value
        self._expect("COMMA")
        c = self._expect("IDENT").value
        self._expect("RPAREN")
        self._expect("SEMI")
        return GemmStmt(a, b, c)

    def _print(self):
        self._expect("PRINT")
        self._expect("LPAREN")
        text = self._expect("STRING").value
        self._expect("RPAREN")
        self._expect("SEMI")
        return PrintStmt(text)

    def _if(self):
        self._expect("IF")
        self._expect("LPAREN")
        condition = self._expression()
        self._expect("RPAREN")
        then_body = self._block()
        else_body = None
        if self._match("ELSE"):
            else_body = self._block()
        return IfStmt(condition, then_body, else_body)

    def _while(self):
        self._expect("WHILE")
        self._expect("LPAREN")
        condition = self._expression()
        self._expect("RPAREN")
        return WhileStmt(condition, self._block())

    def _for(self):
        self._expect("FOR")
        self._expect("LPAREN")
        init = None
        if self._check("INT32"):
            init = self._var_decl()
        elif not self._check("SEMI"):
            init = self._for_assignment_or_expr()
        self._expect("SEMI")
        condition = None if self._check("SEMI") else self._expression()
        self._expect("SEMI")
        step = None if self._check("RPAREN") else self._for_assignment_or_expr()
        self._expect("RPAREN")
        return ForStmt(init, condition, step, self._block())

    def _for_assignment_or_expr(self):
        expr = self._expression()
        if self._match("EQUAL"):
            return AssignStmt(expr, self._expression())
        return ExprStmt(expr)

    def _return(self):
        self._expect("RETURN")
        value = self._expression()
        self._expect("SEMI")
        return ReturnStmt(value)

    def _expression(self):
        return self._equality()

    def _equality(self):
        expr = self._comparison()
        while self._match("EQEQ") or self._match("NEQ"):
            op = self.tokens[self.pos - 1].value
            expr = BinaryOp(op, expr, self._comparison())
        return expr

    def _comparison(self):
        expr = self._term()
        while self._match("LT") or self._match("GT") or self._match("LTE") or self._match("GTE"):
            op = self.tokens[self.pos - 1].value
            expr = BinaryOp(op, expr, self._term())
        return expr

    def _term(self):
        expr = self._factor()
        while self._match("PLUS") or self._match("MINUS"):
            op = self.tokens[self.pos - 1].value
            expr = BinaryOp(op, expr, self._factor())
        return expr

    def _factor(self):
        expr = self._unary()
        while self._match("STAR") or self._match("SLASH") or self._match("PERCENT"):
            op = self.tokens[self.pos - 1].value
            expr = BinaryOp(op, expr, self._unary())
        return expr

    def _unary(self):
        if self._match("MINUS") or self._match("BANG"):
            op = self.tokens[self.pos - 1].value
            return UnaryOp(op, self._unary())
        return self._primary()

    def _primary(self):
        if self._match("NUMBER"):
            return IntLiteral(int(self.tokens[self.pos - 1].value))
        if self._match("LPAREN"):
            expr = self._expression()
            self._expect("RPAREN")
            return expr
        if self._match("IDENT"):
            name = self.tokens[self.pos - 1].value
            if self._match("LPAREN"):
                args = []
                if not self._check("RPAREN"):
                    args.append(self._expression())
                    while self._match("COMMA"):
                        args.append(self._expression())
                self._expect("RPAREN")
                return CallExpr(name, args)
            if self._match("LBRACKET"):
                row = self._expression()
                self._expect("RBRACKET")
                self._expect("LBRACKET")
                col = self._expression()
                self._expect("RBRACKET")
                return TensorIndex(name, row, col)
            return VarRef(name)
        tok = self._peek()
        raise CompileError("expected expression at {}:{}".format(tok.line, tok.column))

    def _number(self):
        sign = 1
        if self._match("MINUS"):
            sign = -1
        return sign * int(self._expect("NUMBER").value)

    def _peek(self):
        return self.tokens[self.pos]

    def _check(self, kind):
        return self._peek().kind == kind

    def _match(self, kind):
        if self._check(kind):
            self.pos += 1
            return True
        return False

    def _expect(self, kind):
        tok = self._peek()
        if tok.kind != kind:
            raise CompileError("expected {}, got {} at {}:{}".format(kind, tok.kind, tok.line, tok.column))
        self.pos += 1
        return tok
