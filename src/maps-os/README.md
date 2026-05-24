# MAPS Mini OS

MAPS Mini OS is the first software layer that runs on the MAPS RV32I simulator.
The current version is intentionally small and single-address-space:

```text
crt0.S -> kmain() -> user_main()
```

The OS provides:

- a tiny boot/runtime path that clears `.bss`
- console output through a simple MMIO console
- an NPU driver using MMIO
- a minimal trap path and OS syscall table
- a demo user program that launches GEMM on the simulated NPU

This is not a fully privileged OS yet. Kernel and user code still share one
address space, but user code now enters the kernel through `ecall`, `mtvec`, and
`mret`. This gives the project a real syscall boundary before adding page tables,
processes, and user/kernel memory protection.
