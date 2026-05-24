import argparse
import sys

from .codegen_c import CCodegen
from .lexer import Lexer
from .parser import Parser
from .sema import SemanticAnalyzer
from .tokens import CompileError


def compile_source(source):
    tokens = Lexer(source).tokenize()
    program = Parser(tokens).parse()
    symbols = SemanticAnalyzer().analyze(program)
    return CCodegen().generate(program, symbols)


def main(argv=None):
    parser = argparse.ArgumentParser(description="TensorC compiler")
    parser.add_argument("input", help="TensorC source file")
    parser.add_argument("-o", "--output", help="output C file")
    args = parser.parse_args(argv)

    try:
        with open(args.input, "r", encoding="utf-8") as f:
            source = f.read()
        output = compile_source(source)
        if args.output:
            with open(args.output, "w", encoding="utf-8", newline="\n") as f:
                f.write(output)
        else:
            sys.stdout.write(output)
        return 0
    except CompileError as exc:
        sys.stderr.write("tensorc: error: {}\n".format(exc))
        return 1


if __name__ == "__main__":
    raise SystemExit(main())

