# Syscall ABI

User code enters the Mini OS with `ecall`.

Register convention:

| Register | Meaning |
| --- | --- |
| `a7` | syscall number |
| `a0` | argument 0 |
| `a1` | argument 1 |
| `a2` | argument 2 |
| `a0` after return | return value |

The CPU saves the next PC to `mepc`, writes the cause to `mcause`, jumps to
`mtvec`, and returns with `mret`.

## Mini OS Syscalls

| Number | Name | Arguments | Return |
| ---: | --- | --- | --- |
| `0` | `exit` | `a0 = code` | code |
| `1` | `write` | `a0 = ptr`, `a1 = len` | bytes written |
| `2` | `npu_submit` | `a0 = npu_desc_t*` | `0` on success |

User code should normally call the user runtime APIs instead of emitting raw
syscalls directly:

- `user_exit`
- `user_write`
- `user_write_cstr`
- `user_npu_gemm`

