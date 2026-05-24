# Demo Walkthrough

This document follows the TensorC GEMM demo through the full MAPS stack.

## 1. TensorC Source

File:

```text
src/maps-compiler/examples/gemm.tc
```

It declares two initialized matrices, one output matrix, does some scalar
control-flow work, calls `gemm(A, B, C)`, checks `C[0][0]`, and prints a message.

## 2. Python Compiler

Command:

```powershell
$env:PYTHONPATH="src/maps-compiler"
python -m tensorc.driver src/maps-compiler/examples/gemm.tc -o out.c
```

The generated C calls the Mini OS user runtime:

```c
user_npu_gemm(A, B, C, 2u, 2u, 2u);
user_write_cstr("tensorc npu ok\n");
```

## 3. Mini OS Build

`maps-os` CMake automatically runs the TensorC compiler and links the generated C:

```powershell
cmake --build src/maps-os/build
```

Generated files:

```text
src/maps-os/build/generated/tensorc_gemm.c
src/maps-os/build/maps_os_tensorc_example.elf
```

## 4. Simulator Run

The CTest test:

```text
src/maps-sim/tests/test_tensorc_os_example.cpp
```

loads `maps_os_tensorc_example.elf` with the ELF loader and runs it in `Machine`.

Execution path:

```text
TensorC-generated user_main
  -> user_npu_gemm
  -> OS syscall
  -> trap.S
  -> kernel syscall dispatcher
  -> NPU driver
  -> NPU MMIO
  -> Bus
  -> NPU device/controller/array
```

## 5. Expected Result

The NPU computes:

```text
[1 2]   [5 6]   [19 22]
[3 4] x [7 8] = [43 50]
```

The simulator also verifies console output:

```text
maps-os boot
tensorc npu ok
maps-os done
```

