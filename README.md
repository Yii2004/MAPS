# MAPS: Mini AI PC Simulator

MAPS is a small educational system project that connects compiler, OS, and
computer architecture concepts in one runnable stack.

The project is intentionally compact. It is not trying to become a full OS, a
production compiler, or a cycle-accurate commercial NPU simulator. Its goal is
to make the full path understandable:

```text
TensorC source
  -> Python compiler
  -> generated C
  -> Mini OS user runtime
  -> OS syscall/trap
  -> MMIO NPU driver
  -> C++ CPU/Bus/NPU simulator
  -> DRAM result
```

## Repository Layout

```text
src/
  maps-abi/       ABI documents shared by compiler, OS, and simulator
  maps-compiler/  Python TensorC compiler
  maps-os/        Mini OS, user runtime, MMIO drivers, trap/syscall path
  maps-sim/       C++ RV32I CPU, Bus, DRAM, NPU, ELF loader, Machine
```

## Current Capabilities

- C++ simulator:
  - RV32I CPU subset plus minimal `zicsr` trap support
  - DRAM and MMIO Bus
  - Console MMIO
  - NPU MMIO device
  - NPU controller, DMA, buffers, systolic array PE dataflow
  - ELF32 RISC-V loader

- Mini OS:
  - `crt0` startup
  - `.bss` clear
  - `mtvec` trap entry
  - syscall table
  - console driver
  - NPU driver
  - user runtime

- TensorC compiler:
  - Python lexer/parser/AST/semantic checker/C codegen
  - `int32` scalar variables
  - arithmetic and comparison expressions
  - `if` / `while` / `for`
  - functions returning `int32`
  - 2D `int32` tensors
  - tensor shape inference from initializers
  - tensor indexing, e.g. `C[0][0]`
  - `gemm(A, B, C)` lowered to `user_npu_gemm`
  - `print("...")` lowered to `user_write_cstr`

## Toolchain

MAPS needs:

- Python 3.8+
- CMake 3.16+
- a native C/C++ compiler for `maps-sim`
- a CMake build tool such as Make or Ninja
- a RISC-V bare-metal toolchain providing `riscv64-unknown-elf-gcc`

On Windows, MSYS2 UCRT64 with MinGW tools is one convenient option. On Linux or
macOS, GCC/Clang plus Make/Ninja is also fine. The RISC-V cross compiler is used
only for building ELF programs that run inside the simulator.

## Build

Build the Mini OS and TensorC-generated OS example:

```powershell
cmake -S src/maps-os -B src/maps-os/build
cmake --build src/maps-os/build
```

Build and test the simulator:

```powershell
cmake -S src/maps-sim -B src/maps-sim/build
cmake --build src/maps-sim/build
ctest --test-dir src/maps-sim/build --output-on-failure
```

If CMake cannot pick a suitable generator automatically, pass one explicitly,
for example `-G "MinGW Makefiles"` on Windows with MinGW, or `-G Ninja` when
using Ninja.

Run Python compiler tests:

```powershell
$env:PYTHONPATH="src/maps-compiler"
python -m unittest discover -s src/maps-compiler/tests
```

## Main Demo

The main TensorC demo is:

```text
src/maps-compiler/examples/gemm.tc
```

`maps-os` CMake compiles it automatically into:

```text
src/maps-os/build/generated/tensorc_gemm.c
src/maps-os/build/maps_os_tensorc_example.elf
```

The simulator test `test_tensorc_os_example` loads that ELF and checks that:

- the OS boots
- the TensorC program prints through OS syscall + console MMIO
- the NPU computes the matrix multiply correctly

Expected output collected by the simulator:

```text
maps-os boot
tensorc npu ok
maps-os done
```

## Documentation

- [Architecture](docs/architecture.md)
- [Demo Walkthrough](docs/demo.md)
- [ABI Documents](src/maps-abi/README.md)

## Open Source Note

This project is released under the MIT License. See [LICENSE](LICENSE).
