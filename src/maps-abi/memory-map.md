# Memory Map

All CPU-visible addresses are byte addresses.

| Region | Base | Size | Owner |
| --- | ---: | ---: | --- |
| DRAM | `0x00000000` | 4 MiB | Program, OS, data |
| NPU MMIO | `0x10000000` | `0x100` | NPU device |
| Console MMIO | `0x10000100` | `0x100` | Text output |

## Console MMIO

| Offset | Name | Access | Meaning |
| ---: | --- | --- | --- |
| `0x00` | `DATA` | write | Low byte is appended to console output |
| `0x04` | `STATUS` | read | `1` means ready |

## NPU MMIO

| Offset | Name |
| ---: | --- |
| `0x00` | `DESC_ADDR` |
| `0x04` | `CMD` |
| `0x08` | `STATUS` |
| `0x0c` | `ERROR` |
| `0x10` | `CYCLE_COUNT_LO` |
| `0x14` | `CYCLE_COUNT_HI` |
| `0x18` | `DMA_READ_COUNT_LO` |
| `0x1c` | `DMA_READ_COUNT_HI` |
| `0x20` | `DMA_WRITE_COUNT_LO` |
| `0x24` | `DMA_WRITE_COUNT_HI` |
| `0x28` | `MAC_COUNT_LO` |
| `0x2c` | `MAC_COUNT_HI` |

`DESC_ADDR` is a byte address pointing to a 10-word NPU descriptor in DRAM.

