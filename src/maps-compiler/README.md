# MAPS Compiler

`maps-compiler` is the Python frontend for TensorC, a small C-like tensor
language that targets MAPS Mini OS.

The first backend emits C code that calls the Mini OS user runtime:

- `user_npu_gemm`
- `user_write_cstr`

This keeps the compiler independent from simulator internals and follows the
ABI documented in `src/maps-abi`.

## Example

```tensorc
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
    print("tensorc npu ok\n");
}

return 0;
```

## Supported TensorC Subset

- `int32` scalar variables
- arithmetic: `+`, `-`, `*`, `/`, `%`
- comparisons: `==`, `!=`, `<`, `<=`, `>`, `>=`
- `if` / `else`
- `while`
- `for`
- functions with `int32` parameters and `int32` return type
- 2D `tensor<int32, rows, cols>` declarations
- shape inference from tensor initializers: `tensor<int32> A = [[...]]`
- tensor indexing: `C[0][0]`
- `gemm(A, B, C)`
- `print("text")`
- `return expr`

Compile to C:

```powershell
python -m tensorc.driver examples/gemm.tc -o build/gemm.c
```

Run tests:

```powershell
python -m unittest discover -s tests
```
