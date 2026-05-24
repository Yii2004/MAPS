# NPU ABI

The NPU accepts a descriptor in DRAM. All addresses are byte addresses.

## Descriptor Layout

`npu_desc_t` is 10 little-endian `uint32_t` words:

| Word | Field |
| ---: | --- |
| 0 | `matrix_m` |
| 1 | `matrix_n` |
| 2 | `matrix_k` |
| 3 | `matrix_a_base` |
| 4 | `matrix_b_base` |
| 5 | `matrix_c_base` |
| 6 | `dataflow` |
| 7 | `tile_m` |
| 8 | `tile_n` |
| 9 | `tile_k` |

Matrices are row-major:

- A shape: `M x K`
- B shape: `K x N`
- C shape: `M x N`

Current scalar type is signed `int32_t`. Overflow, quantization, saturation, and
strided tensors are not part of the ABI yet.

## Dataflow Values

| Value | Dataflow |
| ---: | --- |
| `0` | WS |
| `1` | OS |
| `2` | IS |

The current OS user runtime uses OS dataflow and 16x16x16 tiles by default.

