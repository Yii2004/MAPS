# Compiler Target ABI

The first TensorC compiler backend should target MAPS user runtime calls rather
than raw MMIO or raw syscall instructions.

Recommended lowering for matrix multiply:

```c
user_npu_gemm(a, b, c, m, n, k);
```

Recommended lowering for output/debug:

```c
user_write(ptr, len);
user_write_cstr("text");
```

Program requirements:

- Generate RV32I-compatible C or assembly.
- Link against `maps-os` user runtime.
- Use `int32_t` tensors initially.
- Use row-major layout.
- Pass all addresses as byte addresses.

The compiler should not assume simulator internals such as `Memory` word indices,
NPU buffer sizes, or C++ class layouts.

