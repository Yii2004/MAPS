# Program Layout

MAPS executable programs are ELF32 little-endian RISC-V executables.

Current target:

- ISA: `rv32i_zicsr`
- ABI: `ilp32`
- Entry symbol: `_start`
- Calling convention: standard RISC-V integer calling convention

The Mini OS demo currently uses:

| Section | Address |
| --- | ---: |
| `.text` | starts at `0x00001000` |
| `.maps_os_data` | `0x00002000` |
| `.bss` | linker-defined after data |
| stack top | end of linker DRAM region |

Startup code must:

1. Set `sp` to `_stack_top`.
2. Set `mtvec` to the OS trap vector.
3. Clear `[__bss_start, __bss_end)`.
4. Call `kmain`.
5. Exit through the machine exit ABI.

