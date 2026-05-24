from dataclasses import dataclass

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
    PrintStmt,
    ReturnStmt,
    TensorDecl,
    TensorIndex,
    UnaryOp,
    VarDecl,
    VarRef,
    WhileStmt,
)
from .tokens import CompileError


@dataclass(frozen=True)
class TensorType:
    rows: int
    cols: int


@dataclass(frozen=True)
class ScalarType:
    name: str = "int32"


class SemanticAnalyzer:
    def analyze(self, program):
        symbols = {}
        functions = {}
        for decl in program.declarations:
            if isinstance(decl, FunctionDecl):
                if decl.name in functions:
                    raise CompileError("duplicate function '{}'".format(decl.name))
                functions[decl.name] = decl

        for decl in program.declarations:
            if isinstance(decl, FunctionDecl):
                local = dict(symbols)
                for param in decl.params:
                    if param.name in local:
                        raise CompileError("duplicate parameter '{}'".format(param.name))
                    local[param.name] = ScalarType()
                self._block(decl.body, local, functions)
            else:
                self._stmt(decl, symbols, functions)
        return symbols

    def _block(self, block, symbols, functions):
        local = dict(symbols)
        for stmt in block.statements:
            self._stmt(stmt, local, functions)

    def _stmt(self, stmt, symbols, functions):
        if isinstance(stmt, TensorDecl):
            self._tensor_decl(stmt, symbols)
        elif isinstance(stmt, VarDecl):
            if stmt.name in symbols:
                raise CompileError("duplicate variable '{}'".format(stmt.name))
            self._require_int(stmt.init, symbols, functions)
            symbols[stmt.name] = ScalarType()
        elif isinstance(stmt, AssignStmt):
            self._assign(stmt, symbols, functions)
        elif isinstance(stmt, GemmStmt):
            self._gemm(stmt, symbols)
        elif isinstance(stmt, (PrintStmt,)):
            return
        elif isinstance(stmt, IfStmt):
            self._require_int(stmt.condition, symbols, functions)
            self._block(stmt.then_body, symbols, functions)
            if stmt.else_body is not None:
                self._block(stmt.else_body, symbols, functions)
        elif isinstance(stmt, WhileStmt):
            self._require_int(stmt.condition, symbols, functions)
            self._block(stmt.body, symbols, functions)
        elif isinstance(stmt, ForStmt):
            local = dict(symbols)
            if stmt.init is not None:
                self._stmt(stmt.init, local, functions)
            if stmt.condition is not None:
                self._require_int(stmt.condition, local, functions)
            if stmt.step is not None:
                self._stmt(stmt.step, local, functions)
            self._block(stmt.body, local, functions)
        elif isinstance(stmt, ReturnStmt):
            self._require_int(stmt.value, symbols, functions)
        elif isinstance(stmt, ExprStmt):
            self._expr_type(stmt.expr, symbols, functions)
        else:
            raise CompileError("unsupported statement {}".format(type(stmt).__name__))

    def _tensor_decl(self, stmt, symbols):
        if stmt.name in symbols:
            raise CompileError("duplicate tensor '{}'".format(stmt.name))
        rows, cols = stmt.rows, stmt.cols
        if stmt.init is not None:
            inferred_rows = len(stmt.init)
            inferred_cols = len(stmt.init[0]) if stmt.init else 0
            for row in stmt.init:
                if len(row) != inferred_cols:
                    raise CompileError("tensor '{}' initializer is ragged".format(stmt.name))
            if rows is None and cols is None:
                rows, cols = inferred_rows, inferred_cols
            elif rows != inferred_rows or cols != inferred_cols:
                raise CompileError("tensor '{}' initializer shape mismatch".format(stmt.name))
        if rows is None or cols is None:
            raise CompileError("tensor '{}' requires shape or initializer".format(stmt.name))
        if rows <= 0 or cols <= 0:
            raise CompileError("tensor '{}' has invalid shape".format(stmt.name))
        stmt.rows = rows
        stmt.cols = cols
        symbols[stmt.name] = TensorType(rows, cols)

    def _assign(self, stmt, symbols, functions):
        target_type = self._expr_type(stmt.target, symbols, functions)
        value_type = self._expr_type(stmt.value, symbols, functions)
        if type(target_type) is not type(value_type):
            raise CompileError("assignment type mismatch")
        if isinstance(target_type, TensorType):
            raise CompileError("cannot assign whole tensor")

    def _gemm(self, stmt, symbols):
        for name in (stmt.a, stmt.b, stmt.c):
            if name not in symbols:
                raise CompileError("undefined tensor '{}'".format(name))
            if not isinstance(symbols[name], TensorType):
                raise CompileError("'{}' is not a tensor".format(name))
        a = symbols[stmt.a]
        b = symbols[stmt.b]
        c = symbols[stmt.c]
        if a.cols != b.rows:
            raise CompileError("gemm shape mismatch: {}.cols != {}.rows".format(stmt.a, stmt.b))
        if c.rows != a.rows or c.cols != b.cols:
            raise CompileError("gemm output '{}' must be {}x{}".format(stmt.c, a.rows, b.cols))
        stmt.rows = a.rows
        stmt.cols = b.cols
        stmt.inner = a.cols

    def _require_int(self, expr, symbols, functions):
        typ = self._expr_type(expr, symbols, functions)
        if not isinstance(typ, ScalarType):
            raise CompileError("expected int32 expression")
        return typ

    def _expr_type(self, expr, symbols, functions):
        if isinstance(expr, IntLiteral):
            return ScalarType()
        if isinstance(expr, VarRef):
            if expr.name not in symbols:
                raise CompileError("undefined symbol '{}'".format(expr.name))
            return symbols[expr.name]
        if isinstance(expr, TensorIndex):
            if expr.name not in symbols or not isinstance(symbols[expr.name], TensorType):
                raise CompileError("'{}' is not a tensor".format(expr.name))
            self._require_int(expr.row, symbols, functions)
            self._require_int(expr.col, symbols, functions)
            expr.cols = symbols[expr.name].cols
            return ScalarType()
        if isinstance(expr, UnaryOp):
            self._require_int(expr.operand, symbols, functions)
            return ScalarType()
        if isinstance(expr, BinaryOp):
            self._require_int(expr.left, symbols, functions)
            self._require_int(expr.right, symbols, functions)
            return ScalarType()
        if isinstance(expr, CallExpr):
            if expr.name not in functions:
                raise CompileError("undefined function '{}'".format(expr.name))
            fn = functions[expr.name]
            if len(expr.args) != len(fn.params):
                raise CompileError("function '{}' expects {} args".format(expr.name, len(fn.params)))
            for arg in expr.args:
                self._require_int(arg, symbols, functions)
            return ScalarType()
        raise CompileError("unsupported expression {}".format(type(expr).__name__))
