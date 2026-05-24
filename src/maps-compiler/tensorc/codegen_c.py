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


class CCodegen:
    def generate(self, program, symbols):
        lines = ['#include "maps_os/user_runtime.h"', ""]

        for decl in program.declarations:
            if isinstance(decl, TensorDecl):
                lines.extend(self._tensor_decl(decl, global_scope=True, indent=0))
                lines.append("")

        for decl in program.declarations:
            if isinstance(decl, FunctionDecl):
                lines.extend(self._function(decl))
                lines.append("")

        main_body = [d for d in program.declarations if not isinstance(d, (TensorDecl, FunctionDecl))]
        lines.append("int user_main(void) {")
        returned = self._emit_block_items(lines, main_body, 1)
        if not returned:
            lines.append("    return 0;")
        lines.append("}")
        lines.append("")
        return "\n".join(lines)

    def _function(self, fn):
        params = ", ".join("i32 {}".format(p.name) for p in fn.params)
        lines = ["i32 {}({}) {{".format(fn.name, params)]
        returned = self._emit_block_items(lines, fn.body.statements, 1)
        if not returned:
            lines.append("    return 0;")
        lines.append("}")
        return lines

    def _emit_block(self, lines, block, indent):
        return self._emit_block_items(lines, block.statements, indent)

    def _emit_block_items(self, lines, statements, indent):
        returned = False
        for stmt in statements:
            self._stmt(lines, stmt, indent)
            returned = returned or isinstance(stmt, ReturnStmt)
        return returned

    def _stmt(self, lines, stmt, indent):
        pad = "    " * indent
        if isinstance(stmt, TensorDecl):
            lines.extend(self._tensor_decl(stmt, global_scope=False, indent=indent))
        elif isinstance(stmt, VarDecl):
            lines.append("{}i32 {} = {};".format(pad, stmt.name, self._expr(stmt.init)))
        elif isinstance(stmt, AssignStmt):
            lines.append("{}{} = {};".format(pad, self._expr(stmt.target), self._expr(stmt.value)))
        elif isinstance(stmt, GemmStmt):
            lines.append("{}(void)user_npu_gemm({}, {}, {}, {}u, {}u, {}u);".format(
                pad, stmt.a, stmt.b, stmt.c, stmt.rows, stmt.cols, stmt.inner))
        elif isinstance(stmt, PrintStmt):
            lines.append('{}(void)user_write_cstr("{}");'.format(pad, self._escape_c_string(stmt.text)))
        elif isinstance(stmt, IfStmt):
            lines.append("{}if ({}) {{".format(pad, self._expr(stmt.condition)))
            self._emit_block(lines, stmt.then_body, indent + 1)
            if stmt.else_body is None:
                lines.append("{}}}".format(pad))
            else:
                lines.append("{}}} else {{".format(pad))
                self._emit_block(lines, stmt.else_body, indent + 1)
                lines.append("{}}}".format(pad))
        elif isinstance(stmt, WhileStmt):
            lines.append("{}while ({}) {{".format(pad, self._expr(stmt.condition)))
            self._emit_block(lines, stmt.body, indent + 1)
            lines.append("{}}}".format(pad))
        elif isinstance(stmt, ForStmt):
            init = self._for_part(stmt.init)
            cond = "" if stmt.condition is None else self._expr(stmt.condition)
            step = self._for_part(stmt.step)
            lines.append("{}for ({}; {}; {}) {{".format(pad, init, cond, step))
            self._emit_block(lines, stmt.body, indent + 1)
            lines.append("{}}}".format(pad))
        elif isinstance(stmt, ReturnStmt):
            lines.append("{}return {};".format(pad, self._expr(stmt.value)))
        elif isinstance(stmt, ExprStmt):
            lines.append("{}{};".format(pad, self._expr(stmt.expr)))
        else:
            raise AssertionError("unsupported statement {}".format(type(stmt).__name__))

    def _tensor_decl(self, stmt, global_scope, indent):
        pad = "    " * indent
        count = stmt.rows * stmt.cols
        values = self._flatten_init(stmt)
        prefix = '__attribute__((section(".maps_os_data")))\n' if global_scope else ""
        line = "{}i32 {}[{}] = {{{}}};".format(pad, stmt.name, count, ", ".join(str(v) for v in values))
        if prefix:
            return [prefix.strip(), line]
        return [line]

    def _flatten_init(self, stmt):
        if stmt.init is None:
            return [0 for _ in range(stmt.rows * stmt.cols)]
        values = []
        for row in stmt.init:
            values.extend(row)
        return values

    def _for_part(self, stmt):
        if stmt is None:
            return ""
        if isinstance(stmt, VarDecl):
            return "i32 {} = {}".format(stmt.name, self._expr(stmt.init))
        if isinstance(stmt, AssignStmt):
            return "{} = {}".format(self._expr(stmt.target), self._expr(stmt.value))
        if isinstance(stmt, ExprStmt):
            return self._expr(stmt.expr)
        raise AssertionError("unsupported for part")

    def _expr(self, expr):
        if isinstance(expr, IntLiteral):
            return str(expr.value)
        if isinstance(expr, VarRef):
            return expr.name
        if isinstance(expr, TensorIndex):
            return "{}[({}) * {} + ({})]".format(expr.name, self._expr(expr.row), expr.cols, self._expr(expr.col))
        if isinstance(expr, UnaryOp):
            return "({}{})".format(expr.op, self._expr(expr.operand))
        if isinstance(expr, BinaryOp):
            return "({} {} {})".format(self._expr(expr.left), expr.op, self._expr(expr.right))
        if isinstance(expr, CallExpr):
            return "{}({})".format(expr.name, ", ".join(self._expr(a) for a in expr.args))
        raise AssertionError("unsupported expression {}".format(type(expr).__name__))

    def _escape_c_string(self, text):
        out = []
        for ch in text:
            if ch == "\\":
                out.append("\\\\")
            elif ch == '"':
                out.append('\\"')
            elif ch == "\n":
                out.append("\\n")
            elif ch == "\t":
                out.append("\\t")
            else:
                out.append(ch)
        return "".join(out)
