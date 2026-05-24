# MAPS Architecture

MAPS has four layers.

## 1. Compiler

Location:

```text
src/maps-compiler
```

The compiler is written in Python. TensorC is parsed into an AST, checked, and
lowered to C code that calls the Mini OS user runtime.

Important files:

```text
tensorc/lexer.py
tensorc/parser.py
tensorc/ast.py
tensorc/sema.py
tensorc/codegen_c.py
tensorc/driver.py
```

The compiler does not generate raw MMIO or simulator-specific code. It targets:

```c
user_npu_gemm(...)
user_write_cstr(...)
```

## 2. Mini OS

Location:

```text
src/maps-os
```

The Mini OS is written in C and assembly. It provides startup, trap handling,
syscalls, console output, NPU submission, and a user runtime.

Boot path:

```text
runtime/crt0.S
  -> set sp
  -> set mtvec
  -> clear .bss
  -> kmain()
```

Syscall path:

```text
user runtime
  -> ecall
  -> CPU jumps to mtvec
  -> arch/riscv/trap.S
  -> kernel/syscall.c
  -> driver or kernel service
  -> mret
```

## 3. Simulator

Location:

```text
src/maps-sim
```

The simulator is written in C++. It models:

- RV32I CPU plus minimal CSR/trap support
- DRAM
- Bus
- Console MMIO
- NPU MMIO device
- NPU controller/DMA/buffers/array
- ELF loader
- Machine wrapper

Main runtime path:

```text
Machine
  -> Cpu::step()
  -> Bus
  -> DRAM / Console / NPU
```

## 4. ABI

Location:

```text
src/maps-abi
```

This documents the stable contract between compiler, OS, and simulator:

- memory map
- syscall ABI
- NPU descriptor ABI
- program layout
- compiler target ABI

All CPU-visible addresses are byte addresses.

## MMIO Map

```text
0x10000000  NPU
0x10000100  Console
```

The NPU descriptor is a 10-word `uint32_t` structure in DRAM. Matrix data is
row-major `int32_t`.

