# MAPS Simulator

`maps-sim` is the C++ hardware simulator layer of MAPS. It models a small
RV32I-based machine with DRAM, MMIO devices, and an NPU with a systolic-array
execution path.

The simulator is intentionally compact. It exists to make the compiler -> OS ->
hardware path executable and understandable, not to model every detail of a
modern CPU or accelerator.

## Components

```text
include/
  common/   shared types, config, memory, bus
  cpu/      RV32I CPU interface
  npu/      NPU array, PE, DMA, buffers, controller, MMIO device
  runtime/  bare-metal C runtime headers used by examples
  system/   ELF loader and Machine wrapper

src/
  common/   DRAM and Bus implementation
  cpu/      RV32I interpreter plus minimal CSR/trap support
  npu/      NPU simulator implementation
  runtime/  bare-metal runtime helpers
  system/   loader and whole-machine wiring

tests/      focused unit and end-to-end tests
examples/   bare-metal RISC-V example program
```

## Simulated Machine

```text
Cpu
  -> Bus
     -> DRAM
     -> Console MMIO
     -> NPU MMIO
```

CPU-visible addresses are byte addresses. The memory map is documented in
`src/maps-abi/memory-map.md`.

## CPU

The CPU is a small RV32I interpreter. It supports enough integer instructions
for the generated RISC-V programs used by MAPS, plus a minimal `zicsr` subset:

- `mtvec`
- `mepc`
- `mcause`
- `mstatus`
- `ecall`
- `mret`

This is sufficient for the Mini OS syscall path:

```text
user code -> ecall -> mtvec trap vector -> OS syscall dispatcher -> mret
```

## NPU

The NPU path is:

```text
NpuDevice MMIO
  -> Controller
  -> DMA
  -> Buffer
  -> Array
  -> PE::tick()
```

The array supports the three dataflow modes used by the project:

- WS: weight stationary
- OS: output stationary
- IS: input stationary

Matrix data is row-major `int32_t`. NPU descriptors use byte addresses.

## ELF Loader and Machine

The `system` module provides:

- `ElfLoader`: loads ELF32 little-endian RISC-V executables into DRAM
- `BinaryLoader`: loads raw bytes/words
- `Machine`: owns DRAM, Bus, CPU, and NPU and runs programs end to end

## Build and Test

```powershell
cmake -S src/maps-sim -B src/maps-sim/build
cmake --build src/maps-sim/build
ctest --test-dir src/maps-sim/build --output-on-failure
```

Some end-to-end tests are enabled only when the corresponding RISC-V ELF files
have already been built by `maps-os`.

