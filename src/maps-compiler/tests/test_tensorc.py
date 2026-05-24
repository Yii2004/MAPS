import unittest

from tensorc.driver import compile_source
from tensorc.lexer import Lexer
from tensorc.parser import Parser
from tensorc.sema import SemanticAnalyzer
from tensorc.tokens import CompileError


SOURCE = """
fn scale(int32 x) -> int32 {
    return x * 1;
}

tensor<int32> A = [[1, 2], [3, 4]];
tensor<int32> B = [[5, 6], [7, 8]];
tensor<int32, 2, 2> C;

int32 loops = 0;
for (int32 i = 0; i < 2; i = i + 1) {
    loops = loops + scale(1);
}

gemm(A, B, C);
if (C[0][0] == 19) {
    print("ok\\n");
} else {
    print("bad\\n");
}
while (loops < 2) {
    loops = loops + 1;
}
return 0;
"""


class TensorCTests(unittest.TestCase):
    def test_lexer(self):
        tokens = Lexer(SOURCE).tokenize()
        kinds = [tok.kind for tok in tokens[:10]]
        self.assertEqual(kinds, ["FN", "IDENT", "LPAREN", "INT32", "IDENT", "RPAREN", "ARROW", "INT32", "LBRACE", "RETURN"])

    def test_parser_and_sema(self):
        program = Parser(Lexer(SOURCE).tokenize()).parse()
        symbols = SemanticAnalyzer().analyze(program)
        self.assertEqual(symbols["A"].rows, 2)
        self.assertEqual(symbols["B"].cols, 2)
        self.assertEqual(symbols["C"].rows, 2)
        self.assertEqual(len(program.declarations), 10)

    def test_codegen_c(self):
        c = compile_source(SOURCE)
        self.assertIn("i32 A[4] = {1, 2, 3, 4};", c)
        self.assertIn("i32 scale(i32 x)", c)
        self.assertIn("for (i32 i = 0; (i < 2); i = (i + 1))", c)
        self.assertIn("user_npu_gemm(A, B, C, 2u, 2u, 2u)", c)
        self.assertIn("if ((C[(0) * 2 + (0)] == 19))", c)

    def test_gemm_shape_error(self):
        bad = """
        tensor<int32, 2, 3> A;
        tensor<int32, 2, 2> B;
        tensor<int32, 2, 2> C;
        gemm(A, B, C);
        """
        with self.assertRaises(CompileError):
            compile_source(bad)

    def test_shape_inference_requires_initializer(self):
        bad = "tensor<int32> A;"
        with self.assertRaises(CompileError):
            compile_source(bad)


if __name__ == "__main__":
    unittest.main()
