from dataclasses import dataclass
from typing import List, Optional


@dataclass
class Program:
    declarations: List[object]


@dataclass
class Block:
    statements: List[object]


@dataclass
class Param:
    name: str
    type_name: str


@dataclass
class FunctionDecl:
    name: str
    params: List[Param]
    return_type: str
    body: Block


@dataclass
class TensorDecl:
    name: str
    rows: Optional[int]
    cols: Optional[int]
    init: Optional[List[List[int]]]


@dataclass
class VarDecl:
    name: str
    init: object


@dataclass
class AssignStmt:
    target: object
    value: object


@dataclass
class GemmStmt:
    a: str
    b: str
    c: str
    rows: int = 0
    cols: int = 0
    inner: int = 0


@dataclass
class PrintStmt:
    text: str


@dataclass
class IfStmt:
    condition: object
    then_body: Block
    else_body: Optional[Block]


@dataclass
class WhileStmt:
    condition: object
    body: Block


@dataclass
class ForStmt:
    init: Optional[object]
    condition: Optional[object]
    step: Optional[object]
    body: Block


@dataclass
class ReturnStmt:
    value: object


@dataclass
class ExprStmt:
    expr: object


@dataclass
class IntLiteral:
    value: int


@dataclass
class VarRef:
    name: str


@dataclass
class TensorIndex:
    name: str
    row: object
    col: object
    cols: int = 0


@dataclass
class UnaryOp:
    op: str
    operand: object


@dataclass
class BinaryOp:
    op: str
    left: object
    right: object


@dataclass
class CallExpr:
    name: str
    args: List[object]
