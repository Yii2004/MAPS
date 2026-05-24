# MAPS ABI

This directory documents the software/hardware ABI shared by:

- `maps-sim`: RV32I simulator, MMIO bus, NPU device
- `maps-os`: Mini OS, syscall table, user runtime
- `maps-compiler`: future TensorC frontend and code generator

The goal is to keep compiler output independent from simulator internals. A
compiled user program should target the OS user runtime and this ABI, not private
C++ implementation details.

Documents:

- [memory-map.md](memory-map.md)
- [program-layout.md](program-layout.md)
- [syscall.md](syscall.md)
- [npu.md](npu.md)
- [compiler-target.md](compiler-target.md)

